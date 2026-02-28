# UI Widgets – Tutorial

This directory holds **reusable LVGL widget helpers** (screen, dropdown, keyboard, textarea) and the **widget examples** that run on the display. The examples module implements LVGL-style demos (style, chart, tabview) and custom demos that use these widgets. The display controller calls `run_example()` to show one demo at a time.

## Directory structure

```
src/ui/widgets/
 |
 +--- screen.c
 +--- screen.h
 +--- examples.c
 +--- examples.h
 +--- dropdown.c
 +--- dropdown.h
 +--- keyboard.c
 +--- keyboard.h
 +--- textarea.c
 +--- textarea.h
 +--- my_thai_font.c
 +--- ui_img_debugging_64x64_png.c
 +--- ui_img_healthcare_64x64_png.c
 +--- ai.md
 +--- screen-layout.excalidraw
 +--- README.md
```

**Dependencies:** `examples.c` uses [screen](screen.c), [dropdown](dropdown.c), [keyboard](keyboard.c), [textarea](textarea.c), [ui_layout](../core/ui_layout.h), and TESA datetime/logging. Keyboard depends on textarea. The display controller (`display_controller.c`) includes `examples.h` and calls `run_example()`.

## What each module does

| Module | Purpose |
|--------|---------|
| **screen** | `screen_create_dark()` — full-screen with gradient background; `screen_create_centered_container(parent)` — transparent flex container that centers children. |
| **examples** | `run_example()` — runs one demo (style, chart, or tabview; switch by uncommenting in `examples.c`). Also implements `lv_example_style_1()`, `lv_example_chart_1()`, `lv_example_tabview_2()` and other demos that use dropdown, keyboard, textarea. |
| **dropdown** | `dropdown_create()`, `dropdown_create_simple()` — create dropdown with newline-separated options; helpers to set/get options and selected index. |
| **keyboard** | `keyboard_create()`, `keyboard_create_simple()` — create LVGL keyboard; `keyboard_set_textarea()` / `keyboard_attach_textarea_manual()` to link to a textarea. |
| **textarea** | `textarea_create()` — create textarea with position, size, max length, single/multi-line. |
| **my_thai_font.c** | Custom font asset used by the UI. |
| **ui_img_*.c** | Image assets (e.g. debugging, healthcare icons) used by examples. |

## How to run the widget demo

1. In [examples.c](examples.c), inside `run_example()` or the function it calls (e.g. `example_3()`), uncomment the demo you want: `lv_example_chart_1()`, `lv_example_style_1()`, or `lv_example_tabview_2()`.
2. The display controller calls `run_example()` when the display is ready (see [display_controller.c](../../modules/lvgl_display/controller/display_controller.c)). Only one demo should be active so the screen is not filled by multiple demos at once.

## Usage examples

**Example A — Call the widget demo from the app**

The display controller already calls `run_example()`. From another module you can do the same:

```c
#include "ui/widgets/examples.h"

void show_widget_demo(void) {
  run_example();
}
```

**Example B — Use a dark screen and centered container**

```c
#include "screen.h"

lv_obj_t *scr = screen_create_dark();
lv_obj_t *cont = screen_create_centered_container(scr);
lv_obj_t *label = lv_label_create(cont);
lv_label_set_text(label, "Centered");
lv_obj_center(label);
```

**Example C — Add a dropdown**

```c
#include "dropdown.h"

lv_obj_t *parent = lv_scr_act();
lv_obj_t *dd = dropdown_create_simple(parent, "Option A\nOption B\nOption C", 0);
```

**Example D — Textarea and keyboard together**

```c
#include "textarea.h"
#include "keyboard.h"

lv_obj_t *parent = lv_scr_act();
lv_obj_t *ta = textarea_create(parent, 10, 10, 200, 40, 32, true);
lv_obj_t *kb = keyboard_create_simple(parent);
keyboard_set_textarea(kb, ta);
```

**Example E — Run one specific LVGL example**

To run only the chart (or style, or tabview) demo, call the example function directly. Ensure the active screen is the one you want (e.g. create and load a screen first):

```c
#include "examples.h"

void show_chart_only(void) {
  lv_example_chart_1();
}
```

The implementations of `lv_example_style_1()`, `lv_example_chart_1()`, and `lv_example_tabview_2()` are in [examples.c](examples.c). Uncomment the desired one inside `run_example()` (or the helper it calls) to choose what appears when the display starts.

## Reference

- [lvgl_examples](../../lvgl_examples/README.md) — Reference copy of LVGL examples; adapt code from there into `examples.c`.
- [tabview](../tabview/) — TabView screen and examples; can be used together with widgets on tab content.
