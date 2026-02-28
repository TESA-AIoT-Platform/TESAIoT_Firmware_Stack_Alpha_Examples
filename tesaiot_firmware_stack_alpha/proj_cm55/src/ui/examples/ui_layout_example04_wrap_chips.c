#include "ui_layout_example04_wrap_chips.h"
#include "ui_layout.h"
#include "ui_layout_examples_common.h"

/* Create one chip (pill) with label */
static lv_obj_t *make_chip(lv_obj_t *parent, const char *text) {
  lv_obj_t *chip = lv_obj_create(parent);

  /* Make it pill-like */
  ui_make_pill(chip, 10, 6);

  /* Optional: show chip border so shape is obvious even with theme bg */
  lv_obj_set_style_border_width(chip, 1, 0);
  lv_obj_set_style_border_opa(chip, LV_OPA_40, 0);

  /* Center label */
  lv_obj_t *lbl = lv_label_create(chip);
  lv_label_set_text(lbl, text ? text : "");
  lv_obj_center(lbl);

  /* Make chips not scrollable */
  ui_disable_scroll(chip);

  return chip;
}

void ui_layout_example04_wrap_chips(lv_obj_t *parent) {
  ui_ex_common_t ex =
      ui_ex_create(parent, "Example 04 - Wrap chips",
                   "Row wrap with mixed-length pills (wrap must occur)");

  ui_ex_section_title(ex.body, "ROW WRAP container:");

  /* Wrap container */
  lv_obj_t *wrap = ui_clean_row_wrap(ex.body, 8, 8);
  ui_width_pct(wrap, 100);

  /* Fix height a bit so you can see multiple wrapped lines without scrolling */
  ui_height(wrap, 200);

  /* Show bounds to visualize padding and wrap area */
  ui_ex_debug_border(wrap);

  /* Add mixed-length chips to force wrapping */
  static const char *chips[] = {"LVGL",
                                "Flex",
                                "Wrap",
                                "Chip",
                                "Pill",
                                "Very long chip label",
                                "UI",
                                "Embedded",
                                "9.2",
                                "RowWrap",
                                "Another long label",
                                "Short",
                                "Medium label",
                                "C",
                                "Wrap test"};

  for (size_t i = 0; i < sizeof(chips) / sizeof(chips[0]); i++) {
    make_chip(wrap, chips[i]);
  }

  lv_obj_t *note = lv_label_create(ex.body);

  // Since ex.body is a flex column container, adding top padding to the object
  // effectively pushes it down within the layout flow.
  lv_obj_set_style_pad_top(note, 80, 0);

  lv_label_set_text(note, "Pass:\n"
                          "- Chips flow left-to-right\n"
                          "- When width runs out they wrap to a new line\n"
                          "- Gap between chips and rows stays consistent");
  lv_obj_set_style_text_opa(note, LV_OPA_80, 0);
}
