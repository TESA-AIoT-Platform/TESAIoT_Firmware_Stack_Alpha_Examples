# Error Handler Module – User Manual

**Author:** Asst. Prof. Santi Nuratch, Ph.D  
**Organization:** Thailand Embedded Systems Association (TESA)

---

## 1. Overview

The error handler module provides a centralized fatal-error path for the PSoC Edge CM33 (non-secure) application. When a fatal condition is detected, a single call disables interrupts, logs an optional message (via the project’s logging path, e.g. IPC or UART), and enters a safe infinite loop while blinking the user LED at a fixed rate for visual feedback.

---

## 2. Features

- **Single entry point** – One function, `handle_error(message)`, for all fatal errors.
- **Hard recovery** – Global interrupts disabled with `__disable_irq()` before any further action.
- **Logging** – Prints a descriptive message via `printf` (redirected to IPC if the ipc_log module is used; otherwise UART if retarget-io is initialized).
- **Visual feedback** – Blinks the user LED at 10 Hz (100 ms half-period) using BSP defines `CYBSP_USER_LED_PORT` and `CYBSP_USER_LED_PIN`.
- **Optional message** – Pass `NULL` to log a generic “Unspecified fatal error” message.

---

## 3. Dependencies

- **PDL / BSP** – `cy_pdl.h`, `cybsp.h` for GPIO and delay (`Cy_GPIO_Inv`, `Cy_SysLib_Delay`).
- **stdio** – `printf` for error text (behavior depends on project: ipc_log redirect or retarget-io UART).
- **C runtime** – No FreeRTOS dependency; safe to call from any context (e.g. before scheduler start).

---

## 4. Integration

### 4.1 Makefile

Paths below are relative to the CM33 project directory (the directory containing the Makefile).

- **INCLUDES** – Add the module directory so the compiler finds `error_handler.h`:
  ```makefile
  INCLUDES += modules/error_handler
  ```
- **SOURCES** – Many build systems (e.g. ModusToolbox) auto-discover `.c` files under the project tree. If yours does not, add the implementation explicitly:
  ```makefile
  SOURCES += modules/error_handler/error_handler.c
  ```

### 4.2 Usage

Include the header and call `handle_error()` when a fatal condition is detected. The function does not return.

```c
#include "error_handler.h"

void some_function(void) {
  if (fatal_condition) {
    handle_error("Detailed error description");
  }
}
```

With no custom message:

```c
handle_error(NULL);
```

---

## 5. API Reference

### 5.1 Lifecycle

| Function | Description |
|----------|-------------|
| `handle_error(message)` | Disables global interrupts, logs the error via `printf` (or a generic message if `message` is NULL), then enters an infinite loop blinking the user LED. Does not return. |

---

## 6. Types

None. The module exposes only `handle_error(const char *message)`.

---

## 7. Limits and Notes

- **Non-returning** – `handle_error()` never returns; execution stays in the blink loop.
- **Interrupts** – Global interrupts are disabled; no further ISRs or tasks run after the call.
- **Logging path** – Output goes through `printf`. If `ipc_log` is integrated and `printf` is redirected, the message is sent to CM55. Otherwise, it uses whatever `printf` target is configured (e.g. retarget-io UART). Ensure the chosen path is initialized if you need the message to appear.
- **Hardware** – Uses the BSP user LED; ensure the board support package defines `CYBSP_USER_LED_PORT` and `CYBSP_USER_LED_PIN`.
