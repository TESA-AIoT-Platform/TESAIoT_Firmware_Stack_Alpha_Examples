#ifndef UI_LAYOUT_EXAMPLES_H
#define UI_LAYOUT_EXAMPLES_H

#include "lvgl.h"

/* Optional: include individual headers so you only include this one file */
#include "ui_layout_example01_flex_basics.h"
#include "ui_layout_example02_spacer_grow.h"
#include "ui_layout_example03_alignment.h"
#include "ui_layout_example04_wrap_chips.h"
#include "ui_layout_example05_reverse.h"
#include "ui_layout_example06_scroll.h"
#include "ui_layout_example07_grid_form.h"
#include "ui_layout_example08_grid_dashboard.h"
#include "ui_layout_example09_stack_overlay.h"
#include "ui_layout_example10_cards_pills.h"
#include "ui_layout_example11_symbols.h"

/* If you prefer NOT to include individual headers above,
 * you can comment them out and keep only these declarations. */

void ui_layout_example01_flex_basics(lv_obj_t *parent);
void ui_layout_example02_spacer_grow(lv_obj_t *parent);
void ui_layout_example03_alignment(lv_obj_t *parent);
void ui_layout_example04_wrap_chips(lv_obj_t *parent);
void ui_layout_example05_reverse(lv_obj_t *parent);
void ui_layout_example06_scroll(lv_obj_t *parent);
void ui_layout_example07_grid_form(lv_obj_t *parent);
void ui_layout_example08_grid_dashboard(lv_obj_t *parent);
void ui_layout_example09_stack_overlay(lv_obj_t *parent);
void ui_layout_example10_cards_pills(lv_obj_t *parent);
void ui_layout_example11_symbols(lv_obj_t *parent);

#endif /* UI_LAYOUT_EXAMPLES_H */
