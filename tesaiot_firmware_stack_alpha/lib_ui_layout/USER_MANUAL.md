# lib_ui_layout – User Manual

**Author:** Asst. Prof. Santi Nuratch, Ph.D  
**Organization:** Thailand Embedded Systems Association (TESA)

---

## 1. Overview

lib_ui_layout is a static library of LVGL 9.x layout helpers for the PSoC Edge CM55 (display) core. It provides semantic abstractions for flex, grid, stack, and “clean” containers; spacing (pad, gap, margin); size/position helpers; and optional card/pill styling. Using it reduces boilerplate and keeps UI code consistent. The library is built separately; the application links `libui_layout.a` and includes `ui_layout.h` and `ui_style.h`.

---

## 2. Features

- **Flex containers** – Row/column, center, wrap, reverse, wrap-reverse; theme-visible or “clean” (layout-only).
- **Grid** – Grid container and `ui_grid_place` for child placement.
- **Stack / overlay** – Stack container, fill parent, center in parent.
- **Clean containers** – No background, border, scroll, or click; optional clean scroll container.
- **Spacing** – Pad (all, xy, x, y), gap, margin (all, xy, x, y).
- **Size & position** – Fill parent, wrap content, width/height (fixed or percent), center in parent.
- **Flex setters** – Flow, align, grow; presets (center, between, around, start, end); spacer.
- **Card / pill** – Card container, pill styling.
- **Atomic styling** – Radius, bg color/opa, border; show/hide, opacity.
- **Style constants** – `ui_style.h` defines spacing, radius, and size scales (e.g. `UI_SPACE_M`, `UI_RADIUS_CARD`).

---

## 3. Dependencies

- **LVGL 9.x** – Flex and grid APIs, `lv_obj_t`, `lv_flex_flow_t`, `lv_grid_align_t`, etc.
- **Library build only** – FreeRTOS, CMSIS, and ModusToolbox `mtb_shared` paths (see lib_ui_layout Makefile) for compiling `ui_layout.c`. The application must already have LVGL and its dependencies.

---

## 4. Building the Library

Paths in this section are relative to the **project root** (parent of `lib_ui_layout`).

### 4.1 Prerequisites

- ARM GCC toolchain (e.g. ModusToolbox `mtb-gcc-arm-eabi`).
- LVGL 9.2 and dependencies in `mtb_shared` (or adjust `MTB_SHARED` in `lib_ui_layout/Makefile`).

### 4.2 Build

From project root:

```bash
cd lib_ui_layout
make
```

Output:

- `lib_ui_layout/lib/libui_layout.a` – static library.
- `lib_ui_layout/include/ui_layout.h`, `lib_ui_layout/include/ui_style.h` – public headers (copied from `lib_ui_layout/` by the Makefile).

### 4.3 Override GCC path

```bash
GCC_PATH=/path/to/gcc/bin make
```

Windows (example):

```bash
GCC_PATH=C:/Users/YourUser/Infineon/Tools/mtb-gcc-arm-eabi/14.2.1/gcc/bin make
```

### 4.4 Clean

```bash
cd lib_ui_layout
make clean
```

---

## 5. Integration (Makefile)

The CM55 app can use the pre-built library or the in-tree source. Paths below are relative to the **CM55 project directory** (e.g. `proj_cm55/`).

### 5.1 Using the library in proj_cm55 (default)

proj_cm55 is set up to use the library when `USE_UI_LAYOUT_LIB=1` (default):

- **PREBUILD** – Build the library before each app build: `make -C ../lib_ui_layout`.
- **LDLIBS** – Link the static library: `../lib_ui_layout/lib/libui_layout.a`.
- **INCLUDES** – Add `../lib_ui_layout/include` so the compiler finds `ui_layout.h` and `ui_style.h`.
- **CY_IGNORE** – Exclude in-tree `src/ui/core/ui_layout.c` to avoid duplicate symbols.

Relevant Makefile snippet:

```makefile
USE_UI_LAYOUT_LIB ?= 1

ifeq ($(USE_UI_LAYOUT_LIB),1)
CY_IGNORE += src/ui/core/ui_layout.c
LDLIBS += ../lib_ui_layout/lib/libui_layout.a
endif

ifeq ($(USE_UI_LAYOUT_LIB),1)
PREBUILD = make -C ../lib_ui_layout
endif
```

`INCLUDES += ../lib_ui_layout/include` is set unconditionally so headers are always available.

| USE_UI_LAYOUT_LIB | Behavior |
|-------------------|----------|
| `1` (default)     | Link `libui_layout.a`, run PREBUILD, exclude `src/ui/core/ui_layout.c`. |
| `0`               | Compile `src/ui/core/ui_layout.c`; no library, no PREBUILD. |

Build with source instead of library:

```bash
cd proj_cm55
USE_UI_LAYOUT_LIB=0 make build
```

### 5.2 Using the library in another project

If your CM55 project is a sibling of `lib_ui_layout` (same workspace):

```makefile
INCLUDES += ../lib_ui_layout/include
LDLIBS += ../lib_ui_layout/lib/libui_layout.a
PREBUILD += make -C ../lib_ui_layout
```

If the project is elsewhere, use an absolute path or variable:

```makefile
LIB_UI_LAYOUT = /path/to/lib_ui_layout
INCLUDES += $(LIB_UI_LAYOUT)/include
LDLIBS += $(LIB_UI_LAYOUT)/lib/libui_layout.a
PREBUILD += make -C $(LIB_UI_LAYOUT)
```

If your project has its own `ui_layout.c`, exclude it to avoid duplicate symbols:

```makefile
CY_IGNORE += path/to/ui_layout.c
```

You can also copy `lib/libui_layout.a` and `include/*.h` into the project and reference those paths; then build the library separately when it changes.

---

## 6. API Reference

Include the header:

```c
#include "ui_layout.h"
```

Optional style constants:

```c
#include "ui_style.h"
```

### 6.1 Core polish

| Function | Description |
|----------|-------------|
| `ui_container_clear_style(obj)` | Clear bg, border, outline, shadow, pad. |
| `ui_disable_scroll(obj)` | Disable scrolling. |
| `ui_enable_scroll(obj, scroll_dir)` | Enable scrolling in direction. |
| `ui_set_no_click(obj)` | Make non-clickable. |
| `ui_set_clickable(obj)` | Make clickable. |
| `ui_apply_clean(obj)` | Clear visuals + no click + no scroll. |
| `ui_layout_only(obj)` | Alias for `ui_apply_clean(obj)`. |

### 6.2 Spacing

| Function | Description |
|----------|-------------|
| `ui_pad_all(obj, pad)` | Padding on all sides. |
| `ui_pad_xy(obj, pad_x, pad_y)` | Horizontal and vertical padding. |
| `ui_pad_x(obj, pad_x)` / `ui_pad_y(obj, pad_y)` | Single-axis padding. |
| `ui_gap(obj, gap)` | Gap between flex/grid children. |
| `ui_margin_all(obj, margin)` | Margin on all sides. |
| `ui_margin_xy(obj, margin_x, margin_y)` | Horizontal and vertical margin. |
| `ui_margin_x(obj, margin_x)` / `ui_margin_y(obj, margin_y)` | Single-axis margin. |

### 6.3 Size & position

| Function | Description |
|----------|-------------|
| `ui_fill_parent(obj)` | Size to fill parent. |
| `ui_wrap_content(obj)` | Size to wrap content. |
| `ui_width_content(obj)` / `ui_height_content(obj)` | Width/height wrap content. |
| `ui_center_in_parent(obj)` | Center in parent. |
| `ui_width(obj, w)` / `ui_height(obj, h)` | Fixed width/height. |
| `ui_width_pct(obj, pct)` / `ui_height_pct(obj, pct)` | Width/height in percent. |
| `ui_size(obj, w, h)` / `ui_size_pct(obj, w_pct, h_pct)` | Size fixed or percent. |
| `ui_width_fill(obj)` / `ui_height_fill(obj)` | Fill parent width/height. |

### 6.4 Flex containers (creators)

| Function | Description |
|----------|-------------|
| `ui_flex_container(parent, flow, main_place, cross_place, track_place, gap, pad_all)` | Generic flex container. |
| `ui_clean_flex_container(...)` | Same, with clean style. |
| `ui_row(parent, gap, pad_all)` / `ui_col(parent, gap, pad_all)` | Row/column. |
| `ui_row_center` / `ui_col_center` | Centered. |
| `ui_row_wrap` / `ui_col_wrap` | Wrap. |
| `ui_row_reverse` / `ui_col_reverse` | Reverse. |
| `ui_row_wrap_reverse` / `ui_col_wrap_reverse` | Wrap reverse. |
| `ui_clean_row` / `ui_clean_col` | Clean row/column. |
| `ui_clean_row_center` / `ui_clean_col_center` | Clean centered. |
| `ui_clean_row_wrap` / `ui_clean_col_wrap` | Clean wrap. |
| `ui_clean_row_reverse` / `ui_clean_col_reverse` | Clean reverse. |
| `ui_clean_row_wrap_reverse` / `ui_clean_col_wrap_reverse` | Clean wrap reverse. |
| `ui_spacer(parent, grow)` | Invisible flex item that grows. |

### 6.5 Flex setters

| Function | Description |
|----------|-------------|
| `ui_flex_flow(obj, flow)` | Set flex flow. |
| `ui_flex_align(obj, main_place, cross_place, track_place)` | Set alignment. |
| `ui_flex_grow(obj, grow)` | Set flex grow. |
| `ui_flex_center(obj)` / `ui_flex_between(obj)` / `ui_flex_around(obj)` | Presets. |
| `ui_flex_start(obj)` / `ui_flex_end(obj)` | Start/end align. |

### 6.6 Grid

| Function | Description |
|----------|-------------|
| `ui_grid_container(parent, col_dsc, row_dsc, gap, pad_all)` | Create grid. |
| `ui_clean_grid_container(...)` | Clean grid. |
| `ui_grid_place(obj, col, col_span, row, row_span, x_align, y_align)` | Place child in grid. |

### 6.7 Stack / overlay

| Function | Description |
|----------|-------------|
| `ui_stack_container(parent, pad_all)` | Stack children. |
| `ui_clean_stack_container(parent, pad_all)` | Clean stack. |

### 6.8 Clean containers

| Function | Description |
|----------|-------------|
| `ui_clean_container(parent)` | Clean container. |
| `ui_clean_scroll_container(parent, scroll_dir)` | Clean scroll container. |

### 6.9 Card / pill

| Function | Description |
|----------|-------------|
| `ui_card_container(parent, pad_all, radius)` | Card-style container. |
| `ui_make_pill(obj, pad_x, pad_y)` | Pill/chip style (radius + padding). |

### 6.10 Atomic styling

| Function | Description |
|----------|-------------|
| `ui_radius(obj, radius)` | Set corner radius. |
| `ui_bg_color(obj, color)` / `ui_bg_opa(obj, opa)` | Background. |
| `ui_border(obj, width, color, opa)` | Border. |

### 6.11 Visibility

| Function | Description |
|----------|-------------|
| `ui_show(obj)` / `ui_hide(obj)` / `ui_toggle(obj)` | Show, hide, toggle. |
| `ui_opacity(obj, opa)` | Set opacity. |

---

## 7. Types and Constants (ui_style.h)

### 7.1 Spacing

| Name | Value | Use |
|------|-------|-----|
| UI_SPACE_XXS, UI_SPACE_XS | 2, 4 | Extra small. |
| UI_SPACE_S, UI_SPACE_M, UI_SPACE_L, UI_SPACE_XL | 6, 8, 12, 16 | Normal scale. |
| UI_PAD_SCREEN | UI_SPACE_L | Screen padding. |
| UI_GAP_SMALL, UI_GAP_NORMAL, UI_GAP_LARGE | S, M, L | Gaps. |

### 7.2 Radius

| Name | Value |
|------|-------|
| UI_RADIUS_NONE, UI_RADIUS_S, UI_RADIUS_M, UI_RADIUS_L, UI_RADIUS_XL | 0, 6, 10, 12, 16 |
| UI_RADIUS_CARD, UI_RADIUS_PILL | L, 999 |

### 7.3 Sizes

| Name | Description |
|------|-------------|
| UI_BTN_HEIGHT, UI_BTN_MIN_WIDTH | Button size. |
| UI_ROW_HEIGHT | Row height. |
| UI_CARD_MIN_HEIGHT | Card min height. |
| UI_PILL_PAD_H, UI_PILL_PAD_V | Pill padding. |

### 7.4 Opacity / z-order

| Name | Description |
|------|-------------|
| UI_OPA_DISABLED, UI_OPA_MUTED, UI_OPA_OVERLAY | Opacity. |
| UI_Z_NORMAL, UI_Z_OVERLAY, UI_Z_MODAL | Z-order. |

### 7.5 Macros

| Name | Description |
|------|-------------|
| UI_CLAMP(x, lo, hi) | Clamp value. |
| UI_MIN(a, b)` / `UI_MAX(a, b) | Min/max. |

---

## 8. Usage Examples

**Simple column with row and button:**

```c
#include "ui_layout.h"
#include "lvgl.h"

void my_screen(lv_obj_t *parent) {
  lv_obj_t *root = ui_clean_col(parent, 8, 12);
  ui_fill_parent(root);

  lv_obj_t *header = ui_row(root, 8, 8);
  lv_label_set_text(lv_label_create(header), "Title");

  lv_obj_t *body = ui_clean_col(root, 8, 12);
  ui_flex_grow(body, 1);

  lv_obj_t *btn = lv_btn_create(body);
  lv_label_set_text(lv_label_create(btn), "OK");
}
```

**Using style constants:**

```c
#include "ui_layout.h"
#include "ui_style.h"

lv_obj_t *card = ui_card_container(parent, UI_SPACE_M, UI_RADIUS_CARD);
ui_pad_all(card, UI_PAD_SCREEN);
```

---

## 9. Limits and Notes

- **LVGL 9.x** – The API uses LVGL 9 flex/grid and types; it is not compatible with LVGL 8.
- **Target** – The library is built for CM55 (cortex-m55, softfp). Use the same ABI and toolchain when linking. See [TOOLCHAIN_NOTE.md](TOOLCHAIN_NOTE.md) for GCC vs LLVM_ARM.
- **cycfg_system.h stub** – Building the library requires a minimal `config/cycfg_system.h` stub so that `FreeRTOSConfig.h` (pulled in via LVGL/FreeRTOS) can be included. The stub contains no real definitions; the application uses the BSP-generated `cycfg_system.h`. The lib Makefile adds `-Iconfig` to use `lib_ui_layout/config/cycfg_system.h`.
- **Duplicate symbols** – Either link the library and exclude in-tree `ui_layout.c`, or compile `ui_layout.c` and do not link the library.

---

## 10. Source Layout

| File | Description |
|------|-------------|
| `lib_ui_layout/ui_layout.c` | Implementation. |
| `lib_ui_layout/ui_layout.h` | Public API (copied to `include/` by make). |
| `lib_ui_layout/ui_style.h` | Style constants (copied to `include/` by make). |
| `lib_ui_layout/Makefile` | Builds library and copies headers. |
| `lib_ui_layout/config/cycfg_system.h` | Minimal stub for library build. |

All source and build outputs live under `lib_ui_layout/`.
