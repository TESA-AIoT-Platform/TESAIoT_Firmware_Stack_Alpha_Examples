# CM33 System Module – User Manual

**Author:** Asst. Prof. Santi Nuratch, Ph.D  
**Organization:** Thailand Embedded Systems Association (TESA)  
**Version:** 1.0  
**Target:** PSoC Edge E84, CM33 (non-secure)

---

## 1. Overview

The CM33 system module runs on the CM33 (non-secure) core and provides a single-entry initialization for the application: BSP, RTC (CLIB support), retarget-io, LPTimer (tickless idle), and CM55 enable. Call `cm33_system_init()` at the start of `main()` before creating tasks and starting the scheduler.

---

## 2. Features

- **BSP init** – Initializes device and board peripherals via `cybsp_init()`.
- **RTC** – Configures Real-Time Clock and initializes CLIB support library.
- **Retarget-io** – Initializes UART debug output (printf).
- **Tickless idle** – Configures LPTimer for FreeRTOS tickless idle (deep sleep when idle).
- **CM55 enable** – Enables the CM55 core and waits for it to boot before network stack use.
- **Tick hook** – Optional callback invoked on each FreeRTOS tick via `system_register_tick_hook()`.

---

## 3. Dependencies

- **cybsp** – BSP initialization and board config.
- **cy_time** – `mtb_clib_support_init()` for CLIB RTC support.
- **retarget_io_init** – `init_retarget_io()`, `handle_app_error()`.
- **cyabs_rtos** – `cyabs_rtos_set_lptimer()` for tickless idle.
- **FreeRTOS** – Used by cyabs_rtos; `task.h` for tick hook (`xTaskGetTickCountFromISR`).

---

## 4. Integration

### 4.1 Using in Another Project

**Step 1: Copy the module**

Copy the entire `cm33_system` folder to your project's `modules` directory:

```
your_project/proj_cm33_ns/
├── modules/
│   └── cm33_system/          ← Copy this folder
│       ├── cm33_system.c
│       ├── cm33_system.h
│       └── CM33_SYSTEM.md
├── Makefile
└── ...
```

**Step 2: Modify the Makefile**

Add:

```makefile
SOURCES+= modules/cm33_system/cm33_system.c
INCLUDES+= modules/cm33_system
```

**Step 3: Ensure dependencies**

- **retarget_io_init** – Your project must include `retarget_io_init.c` and `retarget_io_init.h` (typically from a retarget-io library or local copy). This provides `init_retarget_io()` and `handle_app_error()`.
- **cybsp** – BSP must define `CYBSP_RTC_config`, `CYBSP_CM33_LPTIMER_0_*`, and CM55 memory symbols (`CYMEM_CM33_0_m55_nvm_START`, `CYBSP_MCUBOOT_HEADER_SIZE`). PSoC Edge E84 BSP includes these.

**Step 4: Add application code**

Call `cm33_system_init()` at the start of `main()` before creating tasks:

```c
#include "cm33_system.h"

int main(void) {
  if (!cm33_system_init()) {
    printf("Error: Failed to initialize CM33 system\n");
    handle_app_error();
  }
  // Create tasks, start scheduler...
}
```

---

### 4.2 Makefile (this project)

```makefile
SOURCES+= modules/cm33_system/cm33_system.c
INCLUDES+= modules/cm33_system
```

---

### 4.3 Init order

1. `cybsp_init()` – Device and board peripherals.
2. `setup_clib_support()` – RTC and CLIB.
3. `init_retarget_io()` – UART debug.
4. `setup_tickless_idle_timer()` – LPTimer for RTOS.
5. `Cy_SysEnableCM55()` – Enable CM55.
6. `Cy_SysLib_Delay(200U)` – Allow CM55 to boot before network stack.
7. `__enable_irq()` – Enable global interrupts.

---

## 5. API Reference

### 5.1 cm33_system_init

| Function | Description |
|----------|-------------|
| `cm33_system_init()` | Performs full CM33 system initialization. Returns `true` on success, `false` if `cybsp_init()` fails. On LPTimer/RTC errors, calls `handle_app_error()` and does not return. |

### 5.2 cm33_system_enable_cm55

| Function | Description |
|----------|-------------|
| `cm33_system_enable_cm55()` | Enables the CM55 core. Call after IPC pipe is started if boot order matters. |

### 5.3 system_register_tick_hook

| Function | Description |
|----------|-------------|
| `system_register_tick_hook(callback, user_data)` | Registers a callback invoked on each FreeRTOS tick. Call after `cm33_system_init()`. Requires `configUSE_TICK_HOOK` set to 1 in FreeRTOSConfig.h. One callback per core; registering again overwrites the previous. Keep the callback short (runs in tick context). |

---

## 6. Limits and Notes

- **Platform:** PSoC Edge E84 with CM55. BSP must provide LPTimer, RTC, and CM55 memory layout.
- **handle_app_error:** Must be defined (e.g. in `retarget_io_init.h`). Called on LPTimer or MCWDT init failure.
- **CM55:** If your project does not use CM55, you may need to modify or exclude the CM55 enable step.
- **Files:** `cm33_system.c` – Implementation; `cm33_system.h` – Public API; `CM33_SYSTEM.md` – This document.
- **Tick hook:** Requires `configUSE_TICK_HOOK` set to 1 in FreeRTOSConfig.h. The callback runs from the tick interrupt/task context; avoid blocking calls.
