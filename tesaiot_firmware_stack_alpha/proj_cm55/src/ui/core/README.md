# UI Core — Tutorial

This directory provides the **layout and style core** for the CM55 LVGL UI: semantic helpers over LVGL 9.x so you can build screens with less boilerplate and consistent spacing and structure. All layout logic is in [ui_layout.c](ui_layout.c); [ui_style.h](ui_style.h) defines spacing, radius, and size constants.

**Includes**

- `#include "ui_layout.h"` — layout containers, spacing, flex, grid, cards, visibility.
- `#include "ui_style.h"` — use `UI_SPACE_*`, `UI_RADIUS_*`, `UI_GAP_*`, etc. instead of raw numbers.

---

## 1. Getting started

Create a container (row or column), set size and padding, then add children. Prefer **clean** containers when you only need layout (no background/border):

```c
#include "ui_layout.h"
#include "ui_style.h"

void my_screen(lv_obj_t *parent) {
  lv_obj_t *row = ui_clean_row(parent, UI_GAP_NORMAL, UI_SPACE_M);
  ui_width_pct(row, 100);
  ui_wrap_content(row);

  lv_obj_t *btn = lv_btn_create(row);
  ui_height(btn, UI_BTN_HEIGHT);
  ui_width(btn, 80);
  lv_obj_t *lbl = lv_label_create(btn);
  lv_label_set_text(lbl, "OK");
  lv_obj_center(lbl);
}
```

Use **theme-visible** containers (`ui_row`, `ui_col`, …) when the container should show background/border from the theme. Use **clean** variants (`ui_clean_row`, `ui_clean_col`, …) for layout-only grouping.

---

## 2. Usage examples

### Example 1 — Row and column (flex)

Horizontal row of buttons, then a vertical column. Clean containers keep the focus on content:

```c
lv_obj_t *row = ui_clean_row(parent, 8, 8);
ui_width_pct(row, 100);
ui_wrap_content(row);
ui_border(row, 1, lv_palette_main(LV_PALETTE_GREY), LV_OPA_50);
ui_radius(row, UI_RADIUS_S);

lv_obj_t *btn_a = lv_btn_create(row);
ui_size(btn_a, 56, 40);
lv_obj_t *lbl_a = lv_label_create(btn_a);
lv_label_set_text(lbl_a, "A");
lv_obj_center(lbl_a);
lv_obj_t *btn_b = lv_btn_create(row);
ui_size(btn_b, 56, 40);
lv_obj_t *lbl_b = lv_label_create(btn_b);
lv_label_set_text(lbl_b, "B");
lv_obj_center(lbl_b);

lv_obj_t *col = ui_clean_col(parent, 8, 8);
ui_width_pct(col, 100);
ui_wrap_content(col);
lv_obj_t *btn1 = lv_btn_create(col);
ui_size(btn1, LV_PCT(100), 40);
lv_obj_t *lbl1 = lv_label_create(btn1);
lv_label_set_text(lbl1, "1");
lv_obj_center(lbl1);
lv_obj_t *btn2 = lv_btn_create(col);
ui_size(btn2, LV_PCT(100), 40);
lv_obj_t *lbl2 = lv_label_create(btn2);
lv_label_set_text(lbl2, "2");
lv_obj_center(lbl2);
```

### Example 2 — Spacer and flex grow

Push content to one side with a spacer; let one child grow to fill space:

```c
lv_obj_t *toolbar = ui_clean_row(parent, 8, 8);
ui_width_fill(toolbar);
lv_obj_t *lbl = lv_label_create(toolbar);
lv_label_set_text(lbl, "Title");
ui_spacer(toolbar, 1);
lv_obj_t *btn = lv_btn_create(toolbar);
ui_height(btn, 36);
lv_obj_t *row = ui_clean_row(parent, 8, 8);
ui_width_fill(row);
lv_obj_t *left = lv_obj_create(row);
ui_width_content(left);
ui_flex_grow(left, 0);
ui_spacer(row, 1);
lv_obj_t *center = lv_obj_create(row);
ui_flex_grow(center, 1);
ui_spacer(row, 1);
lv_obj_t *right = lv_obj_create(row);
ui_width_content(right);
ui_flex_grow(right, 0);
```

### Example 3 — Grid form

Two-column grid with labels and fields; one submit button spanning both columns:

```c
static const lv_coord_t col_dsc[] = { LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
static const lv_coord_t row_dsc[] = { LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST };

lv_obj_t *grid = ui_clean_grid_container(parent, col_dsc, row_dsc, 10, 10);
ui_width_fill(grid);

lv_obj_t *l0 = lv_label_create(grid);
lv_label_set_text(l0, "Name");
ui_grid_place(l0, 0, 1, 0, 1, LV_GRID_ALIGN_START, LV_GRID_ALIGN_CENTER);

lv_obj_t *f0 = ui_card_container(grid, 10, 10);
ui_height(f0, 44);
ui_grid_place(f0, 1, 1, 0, 1, LV_GRID_ALIGN_STRETCH, LV_GRID_ALIGN_CENTER);

lv_obj_t *submit = lv_btn_create(grid);
ui_height(submit, 44);
ui_grid_place(submit, 0, 2, 1, 1, LV_GRID_ALIGN_STRETCH, LV_GRID_ALIGN_CENTER);
```

### Example 4 — Card and pill

Card container for a block of content; pill style for a chip/tag:

```c
lv_obj_t *card = ui_card_container(parent, UI_SPACE_M, UI_RADIUS_CARD);
ui_height(card, UI_CARD_MIN_HEIGHT);
ui_width_pct(card, 100);

lv_obj_t *title = lv_label_create(card);
lv_label_set_text(title, "Card title");

lv_obj_t *chip_row = ui_clean_row(card, UI_GAP_SMALL, 0);
lv_obj_t *chip = lv_label_create(chip_row);
lv_label_set_text(chip, "Tag");
ui_make_pill(chip, UI_PILL_PAD_H, UI_PILL_PAD_V);
```

### Example 5 — Scrollable area

Vertical scroll container for long content:

```c
lv_obj_t *scroll = ui_clean_scroll_container(parent, LV_DIR_VER);
ui_width_fill(scroll);
ui_height_fill(scroll);

lv_obj_t *col = ui_clean_col(scroll, 8, 8);
ui_width_fill(col);
```

### Example 6 — Visibility and opacity

Show, hide, toggle, or dim:

```c
ui_show(panel);
ui_hide(panel);
ui_toggle(panel);
ui_opacity(label, UI_OPA_MUTED);
```

---

## 3. API reference — ui_layout.h

### Polish and behavior

| API | Description |
|-----|-------------|
| `ui_container_clear_style(obj)` | Clear bg, border, outline, shadow, padding to make a layout-only container. |
| `ui_disable_scroll(obj)` | Turn off scrollbar and scrollable flag. |
| `ui_enable_scroll(obj, scroll_dir)` | Make object scrollable in `scroll_dir` (e.g. `LV_DIR_VER`). |
| `ui_set_no_click(obj)` | Clear clickable flag. |
| `ui_set_clickable(obj)` | Set clickable flag. |
| `ui_apply_clean(obj)` | Clear style + disable scroll + no click. |
| `ui_layout_only(obj)` | Alias for `ui_apply_clean(obj)`. |

### Spacing (layout-agnostic)

| API | Description |
|-----|-------------|
| `ui_pad_all(obj, pad)` | Set padding on all sides. |
| `ui_pad_xy(obj, pad_x, pad_y)` | Set horizontal and vertical padding. |
| `ui_pad_x(obj, pad_x)` | Left/right padding. |
| `ui_pad_y(obj, pad_y)` | Top/bottom padding. |
| `ui_gap(obj, gap)` | Flex/grid gap between children. |
| `ui_margin_all(obj, margin)` | Margin all sides. |
| `ui_margin_xy(obj, margin_x, margin_y)` | Horizontal and vertical margin. |
| `ui_margin_x(obj, margin_x)` | Left/right margin. |
| `ui_margin_y(obj, margin_y)` | Top/bottom margin. |

### Size and position

| API | Description |
|-----|-------------|
| `ui_fill_parent(obj)` | Size 100%×100%, position (0,0). |
| `ui_wrap_content(obj)` | Size to content in both axes. |
| `ui_width_content(obj)` | Width to content. |
| `ui_height_content(obj)` | Height to content. |
| `ui_center_in_parent(obj)` | Center in parent. |
| `ui_width(obj, w)` / `ui_height(obj, h)` | Fixed size. |
| `ui_width_pct(obj, pct)` / `ui_height_pct(obj, pct)` | Size as percentage of parent. |
| `ui_size(obj, w, h)` / `ui_size_pct(obj, w_pct, h_pct)` | Both axes. |
| `ui_width_fill(obj)` | Width 100%, height content. |
| `ui_height_fill(obj)` | Height 100%, width content. |

### Flex — creators (theme-visible)

| API | Description |
|-----|-------------|
| `ui_row(parent, gap, pad_all)` | Row, start alignment. |
| `ui_col(parent, gap, pad_all)` | Column, start alignment. |
| `ui_row_center` / `ui_col_center` | Center on main/cross/track. |
| `ui_row_wrap` / `ui_col_wrap` | Wrap. |
| `ui_row_reverse` / `ui_col_reverse` | Reverse direction. |
| `ui_row_wrap_reverse` / `ui_col_wrap_reverse` | Wrap + reverse. |

### Flex — clean creators (layout-only)

Same names with `ui_clean_` prefix: `ui_clean_row`, `ui_clean_col`, `ui_clean_row_center`, `ui_clean_col_center`, `ui_clean_row_wrap`, `ui_clean_col_wrap`, `ui_clean_row_reverse`, `ui_clean_col_reverse`, `ui_clean_row_wrap_reverse`, `ui_clean_col_wrap_reverse`.

### Flex — low-level and setters

| API | Description |
|-----|-------------|
| `ui_flex_container(parent, flow, main, cross, track, gap, pad_all)` | Full-control flex container. |
| `ui_clean_flex_container(...)` | Same, then `ui_apply_clean` + restore pad/gap. |
| `ui_flex_flow(obj, flow)` | Set layout to flex and flow. |
| `ui_flex_align(obj, main, cross, track)` | Set flex alignment. |
| `ui_flex_grow(obj, grow)` | Set flex grow. |
| `ui_flex_center(obj)` | Center on all axes. |
| `ui_flex_between(obj)` | Space between. |
| `ui_flex_around(obj)` | Space around. |
| `ui_flex_start(obj)` / `ui_flex_end(obj)` | Start/end alignment. |
| `ui_spacer(parent, grow)` | Invisible flex item that grows (default grow 1). |

### Grid

| API | Description |
|-----|-------------|
| `ui_grid_container(parent, col_dsc, row_dsc, gap, pad_all)` | Grid with column/row descriptor arrays (terminate with `LV_GRID_TEMPLATE_LAST`). |
| `ui_clean_grid_container(...)` | Same, then clean + restore pad/gap. |
| `ui_grid_place(obj, col, col_span, row, row_span, x_align, y_align)` | Place child in grid cell. |

### Stack / overlay

| API | Description |
|-----|-------------|
| `ui_stack_container(parent, pad_all)` | Container with `LV_LAYOUT_NONE` for manual stacking. |
| `ui_clean_stack_container(parent, pad_all)` | Same, then clean + restore padding. |

### Clean containers

| API | Description |
|-----|-------------|
| `ui_clean_container(parent)` | Empty container with `ui_apply_clean`. |
| `ui_clean_scroll_container(parent, scroll_dir)` | Clean container, scroll enabled, clickable for touch. |

### Card / pill

| API | Description |
|-----|-------------|
| `ui_card_container(parent, pad_all, radius)` | Card-style container (theme colors, no scroll). |
| `ui_make_pill(obj, pad_x, pad_y)` | Large radius + padding for pill/chip look. |

### Atomic styling

| API | Description |
|-----|-------------|
| `ui_radius(obj, radius)` | Corner radius. |
| `ui_bg_color(obj, color)` | Background color and opa cover. |
| `ui_bg_opa(obj, opa)` | Background opacity. |
| `ui_border(obj, width, color, opa)` | Border width, color, opacity. |

### Visibility

| API | Description |
|-----|-------------|
| `ui_show(obj)` | Clear hidden flag. |
| `ui_hide(obj)` | Set hidden flag. |
| `ui_toggle(obj)` | Toggle hidden. |
| `ui_opacity(obj, opa)` | Set object opacity. |

---

## 4. API reference — ui_style.h

Constants only; include and use instead of magic numbers.

### Spacing scale

| Name | Value | Use |
|------|-------|-----|
| `UI_SPACE_XXS`, `UI_SPACE_XS` | 2, 4 | Extra small. |
| `UI_SPACE_S`, `UI_SPACE_M`, `UI_SPACE_L`, `UI_SPACE_XL` | 6, 8, 12, 16 | Small to extra large. |
| `UI_PAD_SCREEN` | `UI_SPACE_L` | Screen edge padding. |
| `UI_GAP_SMALL`, `UI_GAP_NORMAL`, `UI_GAP_LARGE` | S, M, L | Flex/grid gaps. |

### Radius scale

| Name | Value |
|------|-------|
| `UI_RADIUS_NONE` | 0 |
| `UI_RADIUS_S`, `UI_RADIUS_M`, `UI_RADIUS_L`, `UI_RADIUS_XL` | 6, 10, 12, 16 |
| `UI_RADIUS_CARD` | `UI_RADIUS_L` |
| `UI_RADIUS_PILL` | 999 |

### Standard sizes

| Name | Value |
|------|-------|
| `UI_BTN_HEIGHT`, `UI_BTN_MIN_WIDTH` | 40, 44 |
| `UI_ROW_HEIGHT` | 44 |
| `UI_CARD_MIN_HEIGHT` | 72 |
| `UI_PILL_PAD_H`, `UI_PILL_PAD_V` | 10, 6 |

### Opacity

| Name | Value |
|------|-------|
| `UI_OPA_DISABLED` | `LV_OPA_40` |
| `UI_OPA_MUTED` | `LV_OPA_70` |
| `UI_OPA_OVERLAY` | `LV_OPA_50` |

### Z-order (semantic)

| Name | Value |
|------|-------|
| `UI_Z_NORMAL`, `UI_Z_OVERLAY`, `UI_Z_MODAL` | 0, 10, 20 |

### Macros

| Name | Description |
|------|-------------|
| `UI_CLAMP(x, lo, hi)` | Clamp value. |
| `UI_MIN(a, b)`, `UI_MAX(a, b)` | Min/max (if not already defined). |

---

## 5. Where to see it in action

The [examples](../examples/) directory contains 11 runnable demos (flex, spacer, alignment, wrap, reverse, scroll, grid form, grid dashboard, stack overlay, cards/pills, symbols). Each uses this core; see [../examples/README.md](../examples/README.md) for the catalog and how to run one.
