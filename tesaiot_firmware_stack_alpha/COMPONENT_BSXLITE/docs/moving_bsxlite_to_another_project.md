# Moving COMPONENT_BSXLITE to Another Project

This document describes how to move the BSXLITE sensor fusion component and related code from this application to another ModusToolbox project (for example `psoc-e84-lvgl-ipc` or any PSoC Edge application that uses the BMI270 IMU).

> **Warning**
> The BSXlite library for the CM33 CPU is available only for the `GCC_ARM` toolchain.
> Fusion functionality is not supported on other toolchains.

---

## Overview

**COMPONENT_BSXLITE** provides sensor fusion using the Bosch BSXlite library. It reads accelerometer and gyroscope data from the BMI270, runs the fusion algorithm, and outputs quaternion and Euler orientation. The component is used only in the **CM33 non-secure** project when the application is configured in fusion mode.

---

## What to Move

### 1. Component folder

Copy the entire **COMPONENT_BSXLITE** directory from this application into the **root** of the target application.

| Source | Destination |
|--------|--------------|
| `imu-sensor/COMPONENT_BSXLITE/` | `(target project root)/COMPONENT_BSXLITE/` |

**Contents:**

- `source/sensor_hub_fusion.c` – fusion task and BSXlite integration
- `source/sensor_hub_fusion.h` – public API (task priority, stack size, `create_sensor_hub_fusion_task()`)

### 2. Bosch library files (required)

The component depends on the Bosch BSXlite library. Ensure these files are present in **COMPONENT_BSXLITE** in the target project:

| File | Description |
|------|-------------|
| `bsxlite_interface.h` | Header from the Bosch package |
| `libalgobsx.a` | Prebuilt library from `[Generic]BSXlite_v1.0.2/lib/GCC_OUT/libalgobsxm33/libalgobsx.a` |

If you already have them in this project’s `COMPONENT_BSXLITE`, copy them as well. Otherwise, download the [Bosch BSXlite library](https://www.bosch-sensortec.com/software-tools/double-opt-in-forms/bsxlite-form.html), accept the license, and place the files above in the target’s `COMPONENT_BSXLITE` folder.

---

## Build Integration in the Target Project

### Makefile (CM33 non-secure or equivalent)

In the Makefile of the application that will use fusion (e.g. `proj_cm33_ns/Makefile`), add the component and search path so the build can find **COMPONENT_BSXLITE**:

```make
COMPONENTS+=BSXLITE
SEARCH+=../COMPONENT_BSXLITE
```

Adjust the path in `SEARCH` if your project layout is different (e.g. if the Makefile is not one level below the app root).

### Optional: Use a config mode (like this application)

If the target project has a shared config (e.g. `common.mk`), you can enable fusion only when a specific mode is set:

1. In `common.mk` (or equivalent), set:
   ```make
   CONFIG_APP_MODE = ENABLE_FUSION
   ```
   (Use `ENABLE_ACQUISITION` or leave unset to build without fusion.)

2. In the CM33 NS Makefile, add:
   ```make
   ifeq ($(strip $(CONFIG_APP_MODE)),ENABLE_FUSION)
   COMPONENTS+=BSXLITE
   SEARCH+=../COMPONENT_BSXLITE
   endif
   ```

3. Fusion is supported only with **GCC_ARM** for CM33. Add a check in `common.mk`:
   ```make
   ifeq ($(strip $(CONFIG_APP_MODE)),ENABLE_FUSION)
       ifneq ($(strip $(TOOLCHAIN)),GCC_ARM)
       $(error Fusion mode is supported only by GCC_ARM. Set TOOLCHAIN=GCC_ARM to enable sensor fusion)
       endif
   endif
   ```

---

## Application Code in the Target Project

In the **main** (or startup) file of the application that runs the sensor task (e.g. CM33 NS `main.c`), add the same conditional integration as in this example.

### Include

```c
#ifdef COMPONENT_BSXLITE
    #include "sensor_hub_fusion.h"
#else
    #include "sensor_hub_daq_task.h"   /* or your non-fusion task header */
#endif
```

### Task creation

```c
#ifdef COMPONENT_BSXLITE
    result = create_sensor_hub_fusion_task();
#else
    result = create_sensor_hub_daq_task();   /* or your non-fusion task creation */
#endif
```

If the target application does not have a non-fusion path, you can use only the `#ifdef COMPONENT_BSXLITE` branch and always call `create_sensor_hub_fusion_task()` when the component is in the build.

---

## Dependencies in the Target Project

The fusion code depends on:

| Dependency | Purpose |
|------------|--------|
| **cybsp.h** | BSP (board support, I2C) |
| **FreeRTOS** | `semphr.h`, `task.h` |
| **mtb_bmi270** | BMI270 driver |
| **bsxlite_interface.h** + **libalgobsx.a** | Bosch BSXlite API and library |
| **retarget_io_init.h** | UART/printf retarget (or equivalent) |

Ensure the target application has:

- A BSP that provides the I2C block used for the IMU (same as or compatible with this example).
- FreeRTOS.
- The **mtb_bmi270** (or equivalent) library in the build.
- **TOOLCHAIN=GCC_ARM** when building with fusion (BSXlite for CM33 is provided only for GCC_ARM).

---

## I2C conflict in multi-core projects (CM33 + CM55)

If the target project has **both** CM33 (running the BMI270 fusion task) and **CM55** (e.g. running LVGL display and touch), check for an **I2C conflict**.

### What can go wrong

- The BMI270 on CM33 uses **CYBSP_I2C_CONTROLLER** (typically **SCB0**) and pins such as P8.0 (SCL), P8.1 (SDA).
- The display/touch on CM55 may be configured to use the **same** I2C block. For example, in `proj_cm55/modules/lvgl_display/core/display_i2c_config.h`, when `USE_KIT_PSE84_AI` is not defined, `DISPLAY_I2C_CONTROLLER_HW` is set to `CYBSP_I2C_CONTROLLER_HW` — i.e. **SCB0**.
- **Result:** Both cores use the same physical I2C (SCB0 and the same pins). Only one core can own and drive that peripheral at a time. The other core’s I2C traffic can fail (e.g. **BMI270 init failed** on CM33 if CM55 has already taken over SCB0 for the display).

### How to confirm

- In the target BSP `cycfg_peripherals.h`: `CYBSP_I2C_CONTROLLER_HW` is usually **SCB0**.
- In the CM55 display module: if `DISPLAY_I2C_CONTROLLER_HW` is defined as `CYBSP_I2C_CONTROLLER_HW`, then display and BMI270 share the same I2C.

### What to do

- **Preferred:** Use a **dedicated I2C block** for one of the two:
  - Give the display/touch a different SCB (e.g. via BSP/design so that `DISPLAY_I2C_CONTROLLER_HW` uses another I2C), and keep SCB0 for the BMI270 on CM33, or  
  - Give the BMI270 a different SCB and keep SCB0 for the display on CM55.  
  This requires BSP/design changes (add another I2C in the config and route the correct pins).
- **Alternative:** If you cannot add another I2C, you can **disable** the fusion component in the CM33 Makefile (comment out `COMPONENTS+=BSXLITE` and `SEARCH+=../COMPONENT_BSXLITE`) so the rest of the app (e.g. Wi‑Fi, IPC, display) runs without the BMI270 fusion task.

---

## Component Makefile (if required)

This application does not use a Makefile inside **COMPONENT_BSXLITE**; the build discovers the component via `SEARCH` and `COMPONENTS`. If the target project’s build system does **not** auto-discover sources in a `COMPONENT_*` folder, add a **Makefile** inside `COMPONENT_BSXLITE` in the target project, for example:

```make
SOURCES+=source/sensor_hub_fusion.c
INCLUDES+=.
# If libalgobsx.a is in this folder, add the appropriate library variable
# (e.g. CY_EXTRA_LIBS or LDFLAGS) per your ModusToolbox version.
```

Adjust variable names to match how other local components in the target project add sources and link prebuilt libraries.

---

## Checklist

| Step | Action |
|------|--------|
| 1 | Copy `COMPONENT_BSXLITE/` (with `source/` and optional Makefile) to the target app root. |
| 2 | Ensure `bsxlite_interface.h` and `libalgobsx.a` are in (or referenced from) `COMPONENT_BSXLITE`. |
| 3 | In the target app Makefile: add `COMPONENTS+=BSXLITE` and `SEARCH+=../COMPONENT_BSXLITE` (path as needed). |
| 4 | In main: add conditional `#include` and `create_sensor_hub_fusion_task()` as above. |
| 5 | Confirm BSP, FreeRTOS, mtb_bmi270, and GCC_ARM (for fusion) in the target project. |
| 6 | Add `COMPONENT_BSXLITE/Makefile` only if the target build does not auto-discover the component. |

---

## Reference: Where It Is in This Application

- **Component:** `COMPONENT_BSXLITE/source/sensor_hub_fusion.c`, `sensor_hub_fusion.h`
- **Build:** `common.mk` (`CONFIG_APP_MODE`), `proj_cm33_ns/Makefile` (`COMPONENTS`, `SEARCH`)
- **Usage:** `proj_cm33_ns/main.c` (`#ifdef COMPONENT_BSXLITE` include and task creation)
- **README:** Steps to download Bosch BSXlite and enable fusion are in the main [README.md](../README.md).
