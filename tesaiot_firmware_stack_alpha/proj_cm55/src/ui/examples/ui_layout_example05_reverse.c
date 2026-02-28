#include "ui_layout_example05_reverse.h"
#include "ui_layout.h"
#include "ui_layout_examples_common.h"

/* Create a small labeled box so order is obvious */
static lv_obj_t *make_item(lv_obj_t *parent, const char *text) {
  lv_obj_t *b = ui_ex_box(parent, text, 44, 44);
  return b;
}

/* Chip helper for wrap-reverse */
static lv_obj_t *make_chip(lv_obj_t *parent, const char *text) {
  lv_obj_t *chip = lv_obj_create(parent);
  ui_make_pill(chip, 10, 6);

  lv_obj_set_style_border_width(chip, 1, 0);
  lv_obj_set_style_border_opa(chip, LV_OPA_40, 0);

  lv_obj_t *lbl = lv_label_create(chip);
  lv_label_set_text(lbl, text ? text : "");
  lv_obj_center(lbl);

  ui_disable_scroll(chip);
  ui_set_no_click(chip);

  return chip;
}

void ui_layout_example05_reverse(lv_obj_t *parent) {
  ui_ex_common_t ex =
      ui_ex_create(parent, "Example 05 - Reverse & Wrap-Reverse",
                   "Row/Column reverse ordering + row wrap reverse");

  /* -----------------------------
   * Row reverse
   * ----------------------------- */
  ui_ex_section_title(ex.body, "ROW REVERSE (D C B A visually)");

  lv_obj_t *row_rev = ui_clean_row_reverse(ex.body, 8, 8);
  lv_obj_set_width(row_rev, LV_PCT(100));
  lv_obj_set_height(row_rev, 70);
  ui_ex_debug_border(row_rev);

  make_item(row_rev, "A");
  make_item(row_rev, "B");
  make_item(row_rev, "C");
  make_item(row_rev, "D");

  /* -----------------------------
   * Column reverse
   * ----------------------------- */
  ui_ex_section_title(ex.body, "COLUMN REVERSE (4 3 2 1 visually)");

  lv_obj_t *col_rev = ui_clean_col_reverse(ex.body, 8, 8);
  lv_obj_set_width(col_rev, LV_PCT(100));
  lv_obj_set_height(col_rev, 220);
  ui_ex_debug_border(col_rev);

  /* Fixed-height items so order is obvious */
  lv_obj_t *i1 = ui_ex_box(col_rev, "1", LV_PCT(100), 44);
  lv_obj_t *i2 = ui_ex_box(col_rev, "2", LV_PCT(100), 44);
  lv_obj_t *i3 = ui_ex_box(col_rev, "3", LV_PCT(100), 44);
  lv_obj_t *i4 = ui_ex_box(col_rev, "4", LV_PCT(100), 44);
  (void)i1;
  (void)i2;
  (void)i3;
  (void)i4;

  /* -----------------------------
   * Row wrap reverse
   * ----------------------------- */
  ui_ex_section_title(ex.body,
                      "ROW WRAP REVERSE (order + wrap direction differ)");

  lv_obj_t *wrap_rev = ui_clean_row_wrap_reverse(ex.body, 8, 8);
  lv_obj_set_width(wrap_rev, LV_PCT(100));
  lv_obj_set_height(wrap_rev, 150);
  ui_ex_debug_border(wrap_rev);

  static const char *chips[] = {"A",      "B", "C", "D", "E", "F",
                                "Long-G", "H", "I", "J", "K", "Long-L"};

  for (size_t i = 0; i < sizeof(chips) / sizeof(chips[0]); i++) {
    make_chip(wrap_rev, chips[i]);
  }

  /* Note */
  lv_obj_t *note = lv_label_create(ex.body);
  lv_label_set_text(
      note,
      "Pass:\n"
      "- Row reverse: D appears first (right side)\n"
      "- Column reverse: 4 appears above 1 (reverse order)\n"
      "- Wrap reverse: ordering + wrap direction differ from normal wrap");
  lv_obj_set_style_text_opa(note, LV_OPA_80, 0);
}
