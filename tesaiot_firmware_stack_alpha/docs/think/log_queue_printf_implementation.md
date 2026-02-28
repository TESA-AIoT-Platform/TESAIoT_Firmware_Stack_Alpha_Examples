# Log Queue Notes (Legacy Design Reference)

This document captures a legacy design where CM33 log queue messages were forwarded to CM55 via `IPC_CMD_LOG`. It is kept as historical reference only.

Current implementation uses CM33 as UART owner; CM55 print output uses `IPC_CMD_PRINT` to CM33; CM33 queued logs are drained and printed locally on CM33 UART.

---

## Summary: How It Worked on `ipc-pipe-KIT_PSE84_AI` (Historical)

> The remaining sections in this file describe a historical branch design and are not the current implementation in this repository.

### Architecture Overview

Logging from CM33 to the CM55 console is split into two layers:

1. **log_queue** (frontend): A FreeRTOS queue and printf-style API on CM33. Any task can call `log_queue_printf(format, ...)` without blocking; messages are queued.
2. **log_ipc_transport** (backend): A dedicated worker task on CM33 that reads from the log queue and sends each message to CM55 over the existing IPC pipe using a dedicated command `IPC_CMD_LOG`.

CM55 receives all IPC messages in the same pipe callback; when `cmd == IPC_CMD_LOG` it prints the payload to its UART (no extra "cm55_ipc_task:" prefix), so CM33 logs appear on the CM55 serial output.

### Data Flow

```
  CM33 tasks                    CM33                          CM55
  -----------                   ----                          ----
  log_queue_printf("...")  -->  [FreeRTOS queue]  -->  log_ipc_dispatch_worker
                                      |                        |
                                      v                        v
                                ipc_msg_t                 Cy_IPC_Pipe_SendMessage
                                (cmd=IPC_CMD_LOG,             (to CM55)
                                 data=message)
                                                                 |
                                                                 v
                                                          cm55_msg_callback
                                                                 |
                                                                 v
                                                          cm55_ipc_task:
                                                          if (cmd==IPC_CMD_LOG)
                                                            printf("%s", data);
                                                          else
                                                            printf("cm55_ipc_task: %s", data);
```

### Components on the Branch

| Component | Location | Role |
|----------|----------|------|
| **log_queue** | `proj_cm33_ns/log_queue.c`, `log_queue.h` | FreeRTOS queue (length 16, message size 128 bytes). `log_queue_init()` creates the queue; `log_queue_printf(format, ...)` formats with vsnprintf and sends with `xQueueSend(..., 0)` (non-blocking). |
| **log_ipc_transport** | `proj_cm33_ns/log_ipc_transport.c`, `log_ipc_transport.h` | `log_ipc_transport_init()` calls `log_queue_init()` and creates the "Log IPC Worker" task. The worker loop: `xQueueReceive(log_queue, &msg, portMAX_DELAY)` → fill `ipc_msg_t` (client_id, release_mask, cmd=IPC_CMD_LOG, data=msg.message) → `Cy_IPC_Pipe_SendMessage(CM55_IPC_PIPE_EP_ADDR, CM33_IPC_PIPE_EP_ADDR, ...)` with retry on `CY_IPC_PIPE_ERROR_SEND_BUSY` (1 ms delay). |
| **ipc_communication.h** (shared) | `shared/include/ipc_communication.h` | Defines `IPC_CMD_LOG (0x90)`, `IPC_DATA_MAX_LEN (128)`, and `ipc_msg_t` with `client_id`, `reserved`, `release_mask`, `cmd`, `value`, `data[]`. |
| **cm33_ipc_pipe.c** | `proj_cm33_ns/cm33_ipc_pipe.c` | IPC task uses `log_queue_printf("CM33: Counter=0x%08x\n", ...)` and `log_queue_printf("ERROR ...")` instead of `printf`. |
| **cm55_ipc_pipe.c** | `proj_cm55/src/cm55_ipc_pipe.c` | Callback stores `last_msg_cmd`; `cm55_ipc_task` checks `if (last_msg_cmd == IPC_CMD_LOG)` then `printf("%s", msg_str)` else `printf("cm55_ipc_task: %s\n\r", msg_str)`. |
| **proj_cm33_ns/main.c** | `proj_cm33_ns/main.c` | Calls `log_ipc_transport_init()` before `__enable_irq()` and `cm33_ipc_pipe_start()`. Banner and startup text use `log_queue_printf()`. |

### Branch-Specific Details

- **ipc_msg_t** on the branch: `client_id`, `reserved`, `release_mask` (uint16_t), `cmd` (uint32_t), `value`, `data[IPC_DATA_MAX_LEN]` with `IPC_DATA_MAX_LEN = 128`.
- **This project** currently: `client_id`, `intr_mask` (uint16_t), `cmd` (uint8_t), `value`, `data[64]`. No `IPC_CMD_LOG`, no `reserved`/`release_mask` (uses `intr_mask`).

---

## Implementation Plan (This Project: KIT_PSE84_EVAL_EPC2)

### Goal

Replicate the same behavior: CM33 tasks call a thread-safe, non-blocking log API; a dedicated worker sends log messages to CM55 over the IPC pipe; CM55 prints them on its UART when `cmd == IPC_CMD_LOG`.

### Steps

1. **Shared IPC definitions** (`shared/include/ipc_communication.h`)
   - Add `#define IPC_CMD_LOG (0x90)`.
   - Decide payload size for log lines:
     - **Option A**: Keep `IPC_DATA_MAX_LEN` at 64; log messages truncated to 63 chars + null in transport.
     - **Option B**: Increase `IPC_DATA_MAX_LEN` to 128 for log compatibility with the branch (requires consistent `ipc_msg_t` layout on both cores and in shared code).
   - No change to existing `ipc_msg_t` layout (keep `intr_mask`, `cmd` as uint8_t); only add the new command and optionally extend data length.

2. **log_queue** (CM33)
   - Add `proj_cm33_ns/log_queue.h`: Declare `QueueHandle_t log_queue`, `LOG_QUEUE_LENGTH` (e.g. 16), `LOG_MESSAGE_SIZE` (e.g. 128 or 64 to match IPC_DATA_MAX_LEN), `log_msg_t { char message[LOG_MESSAGE_SIZE] }`, `bool log_queue_init(void)`, `void log_queue_printf(const char *format, ...)`.
   - Add `proj_cm33_ns/log_queue.c`: Define `log_queue`, implement `log_queue_init()` (xQueueCreate), `log_queue_printf()` (va_list, vsnprintf into `log_msg_t`, xQueueSend(..., 0)). Add to CM33 build (Makefile/sources).

3. **log_ipc_transport** (CM33)
   - Add `proj_cm33_ns/log_ipc_transport.h`: Declare `bool log_ipc_transport_init(void)`.
   - Add `proj_cm33_ns/log_ipc_transport.c`: Include `log_queue.h`, `ipc_communication.h`. Worker task: loop with `xQueueReceive(log_queue, &msg, portMAX_DELAY)`; fill `ipc_msg_t` (client_id=CM55_IPC_PIPE_CLIENT_ID, intr_mask=CY_IPC_CYPIPE_INTR_MASK_EP1, cmd=IPC_CMD_LOG, value=0, strncpy data from msg.message, null-terminate); `Cy_IPC_Pipe_SendMessage(CM55_IPC_PIPE_EP_ADDR, CM33_IPC_PIPE_EP_ADDR, ...)` with retry on `CY_IPC_PIPE_ERROR_SEND_BUSY` (vTaskDelay 1 ms). `log_ipc_transport_init()`: call `log_queue_init()`, then xTaskCreate(worker, "Log IPC Worker", stack, priority). Use same `ipc_msg_t` layout as existing pipe (intr_mask; if IPC_DATA_MAX_LEN is 64, truncate message to 63 chars). Add to CM33 build.

4. **CM33 main** (`proj_cm33_ns/main.c`)
   - Call `log_ipc_transport_init()` before `__enable_irq()` and before `cm33_ipc_pipe_start()`. If it returns false, call `handle_error()`.
   - Optionally use `log_queue_printf()` for the startup banner instead of `printf` so that banner also goes to CM55.

5. **CM33 IPC task** (`proj_cm33_ns/cm33_ipc_pipe.c`)
   - Replace `printf("IPC Task Counter: ...")` and error `printf`s with `log_queue_printf(...)` so counter and errors are sent to CM55 via the log transport. Keep LED and pipe send logic unchanged.

6. **CM55 IPC task** (`proj_cm55/src/cm55_ipc_pipe.c`)
   - In the pipe callback, store the received `cmd` (e.g. in a `last_msg_cmd` or existing equivalent).
   - In the task that processes received messages: if `cmd == IPC_CMD_LOG`, print only the payload, e.g. `printf("%s", msg_str)`; otherwise keep current behavior (e.g. `printf("cm55_ipc_task: %s\n\r", msg_str)`).

7. **Build and testing**
   - Ensure CM33 and CM55 both use the same `ipc_communication.h` (IPC_CMD_LOG, IPC_DATA_MAX_LEN, ipc_msg_t). Build both projects; run and verify CM33 log_queue_printf output appears on the CM55 UART.

### Optional / Future

- Increase `IPC_DATA_MAX_LEN` to 128 and set `LOG_MESSAGE_SIZE` to 128 for full-line logs if desired.
- Add a simple ring buffer or drop policy if the log queue is full (currently branch uses non-blocking send; full queue drops the message).

---

*Reference: [ipc-pipe-KIT_PSE84_AI](https://github.com/drsanti/psoc-e84-lvgl-ipc/tree/ipc-pipe-KIT_PSE84_AI) branch.*
