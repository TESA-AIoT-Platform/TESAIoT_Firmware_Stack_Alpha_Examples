#include "ui_run_examples.h"
#include "lv_port_disp.h"
#include "lvgl.h"

void run_ui_layout_examples(void) {
  lv_obj_t *scr = lv_obj_create(NULL);
  lv_obj_set_size(scr, ACTUAL_DISP_HOR_RES, ACTUAL_DISP_VER_RES);
  lv_scr_load(scr);
  //
  //

  // ui_layout_example01_flex_basics(scr);
  // ui_layout_example02_spacer_grow(scr);
  // ui_layout_example03_alignment(scr);
  // ui_layout_example04_wrap_chips(scr);
  // ui_layout_example05_reverse(scr);
  // ui_layout_example06_scroll(scr);
  // ui_layout_example07_grid_form(scr);
  // ui_layout_example08_grid_dashboard(scr);
  // ui_layout_example09_stack_overlay(scr);
  // ui_layout_example10_cards_pills(scr);
  ui_layout_example11_symbols(scr);
}