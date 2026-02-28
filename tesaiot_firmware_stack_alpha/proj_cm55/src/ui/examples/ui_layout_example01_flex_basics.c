#include "ui_layout_example01_flex_basics.h"
#include "lvgl.h"
#include "ui_layout.h"
#include "ui_layout_examples_common.h"

void ui_layout_example01_flex_basics(lv_obj_t *parent) {
  /* Use common scaffold for consistent look */
  ui_ex_common_t ex = ui_ex_create(parent, "Example 01 - Flex Basics",
                                   "Row vs Column demonstrate basic stacking");

  /* -----------------------------
   * ROW section
   * ----------------------------- */
  ui_ex_section_title(ex.body, "ROW (left -> right)");

  lv_obj_t *row = ui_clean_row(ex.body, 8, 8);
  ui_width_pct(row, 100);
  ui_wrap_content(row);

  /* Visual border to see padding area */
  ui_border(row, 1, lv_palette_main(LV_PALETTE_GREY), LV_OPA_50);
  ui_radius(row, 8);

  ui_ex_button(row, "A", 56, 40);
  ui_ex_button(row, "B", 56, 40);
  ui_ex_button(row, "C", 56, 40);
  ui_ex_button(row, "D", 56, 40);

  /* -----------------------------
   * COLUMN section
   * ----------------------------- */
  ui_ex_section_title(ex.body, "COLUMN (top -> bottom)");

  lv_obj_t *col = ui_clean_col(ex.body, 8, 8);
  ui_width_pct(col, 100);
  ui_wrap_content(col);

  ui_border(col, 1, lv_palette_main(LV_PALETTE_GREY), LV_OPA_50);
  ui_radius(col, 8);

  /* Buttons full width so the stacking is obvious */
  ui_ex_button(col, "1", LV_PCT(100), 40);
  ui_ex_button(col, "2", LV_PCT(100), 40);
  ui_ex_button(col, "3", LV_PCT(100), 40);
  ui_ex_button(col, "4", LV_PCT(100), 40);

  /* Optional: footer note */
  lv_obj_t *note = lv_label_create(ex.body);
  lv_label_set_text(note,
                    "Pass:\n- Row buttons stack horizontally\n- Column buttons "
                    "stack vertically\n- Gaps and padding are consistent");
  ui_opacity(note, LV_OPA_70);
}
