#include "ui_layout_example08_grid_dashboard.h"
#include "ui_layout.h"
#include "ui_layout_examples_common.h"

/* A tile/card with a title and big value */
static lv_obj_t *make_tile(lv_obj_t *parent, const char *title,
                           const char *value) {
  lv_obj_t *tile = ui_card_container(parent, 12, 12);
  ui_width_pct(tile, 100);
  lv_obj_set_height(tile, 90);

  /* Use flex column inside tile */
  lv_obj_set_layout(tile, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(tile, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(tile, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_gap(tile, 6, 0);

  lv_obj_t *t = lv_label_create(tile);
  lv_label_set_text(t, title ? title : "");
  lv_obj_set_style_text_opa(t, LV_OPA_70, 0);

  lv_obj_t *v = lv_label_create(tile);
  lv_label_set_text(v, value ? value : "");

  /* Make value look more “primary” */
  lv_obj_set_style_text_font(v, LV_FONT_DEFAULT, 0); /* keep default font */
  lv_obj_set_style_text_opa(v, LV_OPA_COVER, 0);

  return tile;
}

void ui_layout_example08_grid_dashboard(lv_obj_t *parent) {
  ui_ex_common_t ex =
      ui_ex_create(parent, "Example 08 - Grid dashboard",
                   "Grid tiles with spans (one wide tile + regular tiles)");

  ui_ex_section_title(ex.body, "Dashboard:");

  /* 2-column dashboard grid:
   * - 2 cols of equal width (FR units)
   * - 3 rows (content)
   */
  static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                       LV_GRID_TEMPLATE_LAST};

  static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                       LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

  lv_obj_t *grid = ui_clean_grid_container(ex.body, col_dsc, row_dsc, 10, 10);
  ui_width_fill(grid);
  ui_ex_debug_border(grid);

  /* Row 0: wide tile spanning both columns */
  lv_obj_t *t0 = make_tile(grid, "SYSTEM", "OK");
  ui_grid_place(t0, 0, 2, 0, 1, LV_GRID_ALIGN_STRETCH, LV_GRID_ALIGN_CENTER);
  lv_obj_set_height(t0, 100);

  /* Row 1: two tiles */
  lv_obj_t *t1 = make_tile(grid, "TEMP", "27°C");
  ui_grid_place(t1, 0, 1, 1, 1, LV_GRID_ALIGN_STRETCH, LV_GRID_ALIGN_CENTER);

  lv_obj_t *t2 = make_tile(grid, "HUM", "45%");
  ui_grid_place(t2, 1, 1, 1, 1, LV_GRID_ALIGN_STRETCH, LV_GRID_ALIGN_CENTER);

  /* Row 2: two tiles */
  lv_obj_t *t3 = make_tile(grid, "POWER", "12.3V");
  ui_grid_place(t3, 0, 1, 2, 1, LV_GRID_ALIGN_STRETCH, LV_GRID_ALIGN_CENTER);

  lv_obj_t *t4 = make_tile(grid, "NET", "CONNECTED");
  ui_grid_place(t4, 1, 1, 2, 1, LV_GRID_ALIGN_STRETCH, LV_GRID_ALIGN_CENTER);

  /* Note */
  lv_obj_t *note = lv_label_create(ex.body);
  lv_label_set_text(note, "Pass:\n"
                          "- Top tile spans both columns\n"
                          "- Remaining tiles align in a clean 2-column grid\n"
                          "- Gaps and padding are consistent");
  lv_obj_set_style_text_opa(note, LV_OPA_80, 0);
}
