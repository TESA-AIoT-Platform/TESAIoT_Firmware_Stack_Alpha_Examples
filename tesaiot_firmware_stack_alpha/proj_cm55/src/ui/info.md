# LVGL Version Information

## Version
**LVGL Version: 9.2.0**

- Major Version: 9
- Minor Version: 2
- Patch Version: 0

## Details
This project uses LVGL (Light and Versatile Graphics Library) version 9.2.0.

The version information is defined in the dependency file `deps/lvgl.mtb` and the version header at `mtb_shared/lvgl/release-v9.2.0/lv_version.h`.

## Display Screen Size

**Display: Waveshare 7-inch Raspberry Pi DSI LCD C**

- Resolution: 1024 x 600 pixels (width x height)
- Horizontal Resolution: 1024 pixels
- Vertical Resolution: 600 pixels
- Touch Panel: GT911

The display configuration is selected via `CONFIG_DISPLAY = WS7P0DSI_RPI_DISP` in `common.mk`.

The display resolution is defined in `src/lvgl_src/lv_port_disp.h` with the following values:
- `ACTUAL_DISP_HOR_RES`: 1024 pixels
- `ACTUAL_DISP_VER_RES`: 600 pixels
