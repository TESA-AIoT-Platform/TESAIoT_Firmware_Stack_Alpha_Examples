# BSXlite library setup

After downloading and unzipping the Bosch BSXlite library into this folder, do the following.

> **Warning**
> The BSXlite library for the CM33 CPU is available only for the `GCC_ARM` toolchain.
> Fusion functionality is not supported on other toolchains.

## 1. Copy the two required files into `COMPONENT_BSXLITE`

Place them **directly** in this folder (`COMPONENT_BSXLITE\`), next to the `source\` folder.

| Copy from (inside your unzipped package) | Copy to |
|------------------------------------------|--------|
| `bsxlite_interface.h` (in the root of BSXlite_v1.0.2) | `COMPONENT_BSXLITE\bsxlite_interface.h` |
| `lib\GCC_OUT\libalgobsxm33\libalgobsx.a`              | `COMPONENT_BSXLITE\libalgobsx.a`        |

### If your folder is named `BSXlite_v1.0.2` (no `[Generic]`)

- **Header:**  
  `COMPONENT_BSXLITE\BSXlite_v1.0.2\bsxlite_interface.h`  
  → copy to  
  `COMPONENT_BSXLITE\bsxlite_interface.h`

- **Library:**  
  `COMPONENT_BSXLITE\BSXlite_v1.0.2\lib\GCC_OUT\libalgobsxm33\libalgobsx.a`  
  → copy to  
  `COMPONENT_BSXLITE\libalgobsx.a`

If you use **softFP** or **hardFP** variants, use the `.a` from the matching folder (e.g. `libalgobsxm33softFP` or `libalgobsxm33hardFP`) and still copy it as `COMPONENT_BSXLITE\libalgobsx.a`.

## 2. Enable fusion in the application

- In **imu-sensor**: set `CONFIG_APP_MODE = ENABLE_FUSION` in `common.mk`.
- In **psoc-e84-lvgl-ipc**: BSXLITE is already in the build; no extra config needed.

## 3. Build

Use **GCC_ARM** toolchain (required for BSXlite on CM33) and build the CM33 NS project.

## Final layout

```
COMPONENT_BSXLITE\
├── bsxlite_interface.h    ← from Bosch package
├── libalgobsx.a           ← from Bosch package lib\GCC_OUT\libalgobsxm33\
├── source\
│   ├── sensor_hub_fusion.c
│   └── sensor_hub_fusion.h
├── SETUP_BSXLITE.md       ← this file
└── (optional) BSXlite_v1.0.2\   ← your unzipped package can stay here
```
