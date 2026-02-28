#include "ui_layout_example07_grid_form.h"
#include "ui_layout.h"
#include "ui_layout_examples_common.h"

/* A simple “field” mock: a card-like container with placeholder text */
static lv_obj_t *make_field(lv_obj_t *parent, const char *placeholder) {
  lv_obj_t *field = ui_card_container(parent, 10, 10);
  ui_height(field, 44);
  ui_width_pct(field, 100);

  /* left-aligned placeholder label */
  lv_obj_t *lbl = lv_label_create(field);
  lv_label_set_text(lbl, placeholder ? placeholder : "");
  lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 0, 0);
  lv_obj_set_style_text_opa(lbl, LV_OPA_70, 0);

  return field;
}

void ui_layout_example07_grid_form(lv_obj_t *parent) {
  ui_ex_common_t ex = ui_ex_create(parent, "Example 07 - Grid form",
                                   "2-column grid: label + field, and a submit "
                                   "button spanning both columns");

  ui_ex_section_title(ex.body, "Form:");

  /* Grid descriptors:
   * - 2 columns: label (content), field (flexible)
   * - rows: content-based (auto)
   */
  static const lv_coord_t col_dsc[] = {LV_GRID_CONTENT, /* label column */
                                       LV_GRID_FR(1),   /* field column */
                                       LV_GRID_TEMPLATE_LAST};

  static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                       LV_GRID_CONTENT, LV_GRID_CONTENT,
                                       LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

  lv_obj_t *grid = ui_clean_grid_container(ex.body, col_dsc, row_dsc, 10, 10);
  ui_width_fill(grid);
  ui_ex_debug_border(grid);

  /* Row 0 */
  lv_obj_t *l0 = lv_label_create(grid);
  lv_label_set_text(l0, "Name");
  ui_grid_place(l0, 0, 1, 0, 1, LV_GRID_ALIGN_START, LV_GRID_ALIGN_CENTER);

  lv_obj_t *f0 = make_field(grid, "John Doe");
  ui_grid_place(f0, 1, 1, 0, 1, LV_GRID_ALIGN_STRETCH, LV_GRID_ALIGN_CENTER);

  /* Row 1 */
  lv_obj_t *l1 = lv_label_create(grid);
  lv_label_set_text(l1, "Email");
  ui_grid_place(l1, 0, 1, 1, 1, LV_GRID_ALIGN_START, LV_GRID_ALIGN_CENTER);

  lv_obj_t *f1 = make_field(grid, "name@example.com");
  ui_grid_place(f1, 1, 1, 1, 1, LV_GRID_ALIGN_STRETCH, LV_GRID_ALIGN_CENTER);

  /* Row 2 */
  lv_obj_t *l2 = lv_label_create(grid);
  lv_label_set_text(l2, "Phone");
  ui_grid_place(l2, 0, 1, 2, 1, LV_GRID_ALIGN_START, LV_GRID_ALIGN_CENTER);

  lv_obj_t *f2 = make_field(grid, "+1 234 567 890");
  ui_grid_place(f2, 1, 1, 2, 1, LV_GRID_ALIGN_STRETCH, LV_GRID_ALIGN_CENTER);

  /* Row 3 */
  lv_obj_t *l3 = lv_label_create(grid);
  lv_label_set_text(l3, "Country");
  ui_grid_place(l3, 0, 1, 3, 1, LV_GRID_ALIGN_START, LV_GRID_ALIGN_CENTER);

  lv_obj_t *f3 = make_field(grid, "Select...");
  ui_grid_place(f3, 1, 1, 3, 1, LV_GRID_ALIGN_STRETCH, LV_GRID_ALIGN_CENTER);

  /* Row 4: submit spanning both columns */
  lv_obj_t *submit = lv_btn_create(grid);
  ui_height(submit, 44);
  ui_grid_place(submit, 0, 2, 4, 1, LV_GRID_ALIGN_STRETCH,
                LV_GRID_ALIGN_CENTER);

  lv_obj_t *submit_lbl = lv_label_create(submit);
  lv_label_set_text(submit_lbl, "Submit");
  lv_obj_center(submit_lbl);

  /* Note */
  lv_obj_t *note = lv_label_create(ex.body);
  lv_label_set_text(note, "Pass:\n"
                          "- Labels align in left column\n"
                          "- Fields stretch to fill right column\n"
                          "- Submit button spans both columns");
  lv_obj_set_style_text_opa(note, LV_OPA_80, 0);
}
