# IPC (Inter-Processor Communication)

This document describes the IPC mechanism implemented between the **Cortex-M33** and **Cortex-M55** cores in the PSOC™ Edge MCU.

## Overview

The IPC (Inter-Processor Communication) in this project uses the Infineon **IPC Pipe** driver to exchange messages between the cores. Each core runs independently, with the CM33 handling lower-level tasks and connectivity, while the CM55 handles the high-performance UI (LVGL) and data display formatting.

### Multi-Core Roles

| Core | Role | IPC Responsibility |
|------|------|-------------------|
| **CM33** | System Control & Connectivity | Handles Wi-Fi hardware interactions, sensor processing, and debug UART output. Sends Wi-Fi/gyro/button events to CM55 and prints forwarded CM55 text (`IPC_CMD_PRINT`). |
| **CM55** | UI & Graphics (LVGL) | Requests Wi-Fi operations and receives status/scan results/events. Does not directly own the debug UART. |

### Block diagram

```mermaid
flowchart TB
    subgraph CM33["CM33 (proj_cm33_ns)"]
        R33[Receiver task]
        S33[Send / WiFi results]
        R33 --> S33
    end
    subgraph Pipe["IPC Pipe (shared memory)"]
        EP1[EP1 CM33]
        EP2[EP2 CM55]
    end
    subgraph CM55["CM55 (proj_cm55)"]
        S55[Sender task]
        R55[Receiver / app task]
        S55 --> R55
    end
    CM33 -->|IPC_CMD_WIFI_SCAN etc.| EP1
    EP1 <--> EP2
    EP2 -->|GYRO, WIFI events, BUTTON| CM55
    CM55 -->|IPC_CMD_WIFI_SCAN etc.| EP2
```

---

## Hardware Configuration

The communication is built on top of the hardware IPC physical channels. The configuration is defined in `shared/include/ipc_communication.h`.

- **IPC Channel 4 (EP1)**: Allocated for CM33 non-secure processor (`CM33_IPC_PIPE_EP_ADDR = 1UL`).
- **IPC Channel 15 (EP2)**: Allocated for CM55 processor (`CM55_IPC_PIPE_EP_ADDR = 2UL`).
- **Client IDs**:
  - CM33 Client ID: `3UL`
  - CM55 Client ID: `5UL`
- **Shared Memory**: A region in SRAM is marked as `CY_SECTION_SHAREDMEM` for the message structures (`ipc_msg_t`).

---

## Data Structures

The IPC messages are exchanged using a standard structure that includes a client ID, interrupt mask, command, and data payload.

```c
#define IPC_DATA_MAX_LEN (128UL)

typedef struct {
  uint16_t client_id; /* Bits 0-7: Client ID */
  uint16_t intr_mask; /* Bits 16-31: Release Mask (MANDATORY for Pipe Driver) */
  uint32_t cmd;
  uint32_t value;
  char data[IPC_DATA_MAX_LEN];
} ipc_msg_t;
```

### IPC Commands

| Command Hex | Macro | Source -> Dest | Payload |
| :--- | :--- | :--- | :--- |
| `0x90` | `IPC_CMD_LOG` | CM33 -> CM55 | Legacy log command (reserved/compatibility). |
| `0x91` | `IPC_CMD_GYRO` | CM33 -> CM55 | `gyro_data_t` |
| `0x93` | `IPC_CMD_BUTTON_EVENT` | CM33 -> CM55 | `button_event_t` |
| `0x94` | `IPC_CMD_CLI_MSG` | CM33 -> CM55 | CLI message payload |
| `0x95` | `IPC_CMD_TOUCH` | CM33 -> CM55 | `ipc_touch_event_t` |
| `0x96` | `IPC_CMD_PRINT` | CM55 -> CM33 | text chunk for CM33 UART print |
| `0x9F` | `IPC_CMD_PING` | CM33 -> CM55 | ping/control message |
| `0xA0` | `IPC_CMD_WIFI_SCAN_REQ` | CM55 -> CM33 | `ipc_wifi_scan_request_t` |
| `0xA1` | `IPC_CMD_WIFI_CONNECT_REQ` | CM55 -> CM33 | `ipc_wifi_connect_request_t` |
| `0xA2` | `IPC_CMD_WIFI_DISCONNECT_REQ` | CM55 -> CM33 | no payload |
| `0xA3` | `IPC_CMD_WIFI_STATUS_REQ` | CM55 -> CM33 | no payload |
| `0xB0` | `IPC_EVT_WIFI_SCAN_RESULT` | CM33 -> CM55 | `wifi_info_t` segment |
| `0xB1` | `IPC_EVT_WIFI_SCAN_COMPLETE` | CM33 -> CM55 | `ipc_wifi_scan_complete_t` |
| `0xB2` | `IPC_EVT_WIFI_STATUS` | CM33 -> CM55 | `ipc_wifi_status_t` |

---

## Initialization Flow

The CM33 core boots first and is responsible for enabling the CM55 core. The IPC pipes must be initialized on both sides, with a small synchronization delay.

```mermaid
sequenceDiagram
    participant CM33s as CM33 (Secure)
    participant CM33ns as CM33 (Non-Secure)
    participant CM55 as CM55

    CM33s->>CM33ns: Boot Non-Secure
    activate CM33ns
    CM33ns->>CM33ns: cm33_ipc_pipe_start()
    Note over CM33ns: Initializes Pipes & 50ms Sync Delay
    CM33ns->>CM55: Cy_SysEnableCM55()
    activate CM55

    CM55->>CM55: cm55_ipc_pipe_start()
    Note over CM55: Initializes Pipes & 50ms Sync Delay

    CM33ns->>CM55: Send IPC Log/Data (Event based)
    CM55->>CM33ns: Send Requests (Periodical/UI Triggered)
```

---

## Communication Logic

```mermaid
flowchart LR
    subgraph CM55["CM55"]
        Q55[Send queue]
        S55[Sender task]
        R55[Receiver task]
        Q55 --> S55
        S55 -->|Cy_IPC_Pipe_SendMessage| IPC
        IPC -->|callback| R55
    end
    subgraph IPC["IPC Pipe"]
        Buf[Shared buffer]
    end
    subgraph CM33["CM33"]
        R33[Receiver task]
        S33[Send WiFi / log / gyro / button]
        R33 --> S33
        S33 --> Buf
    end
```

### CM33 Side (Source: `proj_cm33_ns/cm33_ipc_pipe.c`)
- **`cm33_ipc_receiver_task`**:
  - Monitors received requests from CM55.
  - Handles Wi-Fi request commands (`IPC_CMD_WIFI_SCAN_REQ`, `IPC_CMD_WIFI_CONNECT_REQ`, `IPC_CMD_WIFI_DISCONNECT_REQ`, `IPC_CMD_WIFI_STATUS_REQ`).
  - Handles CM55 print forwarding command (`IPC_CMD_PRINT`) and prints message to CM33 UART.
- **`cm33_ipc_send_wifi_results`**:
  - Iterates through scan results.
  - Packs `total_count` and `current_index` into the `value` field using `IPC_WIFI_SCAN_VALUE_COUNT_SHIFT`.
  - **Implemented a 5ms delay** between segments to prevent static buffer overwrite on the receiver side.

### CM55 Side (Source: `proj_cm55/modules/cm55_ipc_pipe/cm55_ipc_pipe.c`)
- **`cm55_ipc_sender_task`**:
  - Waits for messages in `s_ipc_send_queue`.
  - Atomically sends messages to CM33 using `Cy_IPC_Pipe_SendMessage`.
  - **Implements 100ms throttling** to prevent overwhelming the CM33 and the IPC hardware buffer.
- **CM55 IPC app receiver path**:
  - Parses incoming Wi-Fi scan result/status events, button events, and gyro data.
  - Accumulates Wi-Fi segments and sets ready flags for UI/app consumption.
  - Maintains local cache for status display and command-triggered workflows.

---

## Error Handling & Robustness

1.  **Synchronization Delay**: Both cores implement a 50ms `Cy_SysLib_Delay` during initialization.
2.  **Sender-Consumer Throttling**: The CM33 uses `vTaskDelay(pdMS_TO_TICKS(5))` when sending multi-packet Wi-Fi lists. This ensures the CM55 task has time to copy the data from the shared memory buffer before it is overwritten by the next packet.
3.  **Command Decoupling**: The CM55 receiver task checks specific "Ready" flags rather than just the last command ID, ensuring that transient messages (like Gyro) don't cause the task to skip processing valid Wi-Fi or Event data.
4.  **Shared Memory Security**: Message structures are placed in `CY_SECTION_SHAREDMEM` to ensure visibility across both cores.

---

## Key Files

- `shared/include/ipc_communication.h`: Shared definitions, command codes, and `ipc_msg_t`.
- `proj_cm33_ns/cm33_ipc_pipe.c`: CM33 message management and throttling.
- `proj_cm55/modules/cm55_ipc_pipe/cm55_ipc_pipe.c`: CM55 IPC sender/pipe setup.
- `proj_cm55/modules/cm55_ipc_app/cm55_ipc_app.c`: CM55 app-side receive path and Wi-Fi trigger APIs.

---
*Last updated: 2026-02-23*
