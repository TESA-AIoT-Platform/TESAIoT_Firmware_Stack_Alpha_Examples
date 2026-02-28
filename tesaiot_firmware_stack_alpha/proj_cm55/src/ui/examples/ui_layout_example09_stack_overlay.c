#include "ui_layout_example09_stack_overlay.h"
#include "ui_layout.h"
#include "ui_layout_examples_common.h"

/* Badge overlay demo */
static void make_badge_overlay(lv_obj_t *parent) {
  ui_ex_section_title(parent, "Badge overlay:");

  /* Stack container */
  lv_obj_t *stack = ui_clean_stack_container(parent, 0);
  lv_obj_set_width(stack, LV_PCT(100));
  lv_obj_set_height(stack, 120);
  ui_ex_debug_border(stack);

  /* Base card */
  lv_obj_t *card = ui_card_container(stack, 12, 12);
  lv_obj_set_size(card, LV_PCT(100), LV_PCT(100));

  lv_obj_t *lbl = lv_label_create(card);
  lv_label_set_text(lbl, "Messages");
  lv_obj_center(lbl);

  /* Badge */
  lv_obj_t *badge = lv_obj_create(stack);
  ui_make_pill(badge, 8, 4);

  lv_obj_set_style_bg_color(badge, lv_palette_main(LV_PALETTE_RED), 0);
  lv_obj_set_style_bg_opa(badge, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(badge, 0, 0);

  lv_obj_t *btxt = lv_label_create(badge);
  lv_label_set_text(btxt, "3");
  lv_obj_center(btxt);

  /* Position badge top-right */
  lv_obj_align(badge, LV_ALIGN_TOP_RIGHT, -6, 6);
}

/* Loading overlay demo */
static void make_loading_overlay(lv_obj_t *parent) {
  ui_ex_section_title(parent, "Loading overlay:");

  /* Stack container */
  lv_obj_t *stack = ui_clean_stack_container(parent, 0);
  lv_obj_set_width(stack, LV_PCT(100));
  lv_obj_set_height(stack, 160);
  ui_ex_debug_border(stack);

  /* Base content */
  lv_obj_t *card = ui_card_container(stack, 12, 12);
  lv_obj_set_size(card, LV_PCT(100), LV_PCT(100));

  lv_obj_t *txt = lv_label_create(card);
  lv_label_set_text(txt, "Fetching data...");
  lv_obj_center(txt);

  /* Overlay layer */
  lv_obj_t *overlay = lv_obj_create(stack);
  lv_obj_set_size(overlay, LV_PCT(100), LV_PCT(100));

  lv_obj_set_style_bg_color(overlay, lv_palette_main(LV_PALETTE_GREY), 0);
  lv_obj_set_style_bg_opa(overlay, LV_OPA_50, 0);
  lv_obj_set_style_border_width(overlay, 0, 0);

  /* Centered overlay text */
  lv_obj_t *loading = lv_label_create(overlay);
  lv_label_set_text(loading, "Loading...");
  lv_obj_center(loading);

  /* Overlay should not intercept clicks (layout-only demo) */
  ui_set_no_click(overlay);
}

void ui_layout_example09_stack_overlay(lv_obj_t *parent) {
  ui_ex_common_t ex = ui_ex_create(parent, "Example 09 - Stack / Overlay",
                                   "Overlay objects without affecting layout");

  make_badge_overlay(ex.body);
  make_loading_overlay(ex.body);

  lv_obj_t *note = lv_label_create(ex.body);
  lv_label_set_text(note, "Pass:\n"
                          "- Badge floats above card corner\n"
                          "- Loading overlay covers content\n"
                          "- Underlying layout is unchanged");
  lv_obj_set_style_text_opa(note, LV_OPA_80, 0);
}
