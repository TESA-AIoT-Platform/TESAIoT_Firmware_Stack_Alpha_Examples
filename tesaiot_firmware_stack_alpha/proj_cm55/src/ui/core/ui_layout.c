/**
 * @file ui_layout.c
 * @brief Implementation of semantic abstractions for the LVGL 9.x layout
 * engine.
 *
 * @author Asst.Prof.Dr.Santi Nuratch, TESA
 * @date   2026-01-23
 *
 * This library centralizes common layout patterns to reduce boilerplate
 * and ensure visual consistency across the application.
 *
 * Supported Patterns:
 * - Atomic Spacing: X/Y/XY variants for Padding and Margins.
 * - Atomic Sizing: Fixed, Percentage, and Content/Fill helpers.
 * - Sizing & Pos: Fill parent, Width/Height fill, Wrap/Width/Height content,
 * Centering.
 * - Flex Layouts: Pre-configured Row/Column & semantic presets (Center,
 * Between, etc.).
 * - Grid Layouts: Simplified grid dsc management and cell spanning.
 * - Atomic Styling: Radius, Background (color/opa), and Border helpers.
 * - Visibility: Show, Hide, Toggle, and Layer Opacity.
 * - Polish: 'Clean' layout-only containers and component styling (Pills/Cards).
 */
#include "ui_layout.h"

/* =========================================================================
 * Core “polish” helpers
 * ========================================================================= */

void ui_container_clear_style(lv_obj_t *obj) {
  lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(obj, 0, 0);
  lv_obj_set_style_outline_width(obj, 0, 0);
  lv_obj_set_style_shadow_width(obj, 0, 0);
  lv_obj_set_style_pad_all(obj, 0, 0);
}

void ui_disable_scroll(lv_obj_t *obj) {
  lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

void ui_enable_scroll(lv_obj_t *obj, lv_dir_t scroll_dir) {
  lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scroll_dir(obj, scroll_dir);
  lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF); /* polished default */
}

void ui_set_no_click(lv_obj_t *obj) {
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
}

void ui_set_clickable(lv_obj_t *obj) {
  lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
}

void ui_apply_clean(lv_obj_t *obj) {
  ui_container_clear_style(obj);
  ui_disable_scroll(obj);
  ui_set_no_click(obj);
}

/* =========================================================================
 * Spacing helpers
 * ========================================================================= */

void ui_pad_all(lv_obj_t *obj, int32_t pad) {
  lv_obj_set_style_pad_all(obj, pad, 0);
}

void ui_pad_xy(lv_obj_t *obj, int32_t pad_x, int32_t pad_y) {
  lv_obj_set_style_pad_left(obj, pad_x, 0);
  lv_obj_set_style_pad_right(obj, pad_x, 0);
  lv_obj_set_style_pad_top(obj, pad_y, 0);
  lv_obj_set_style_pad_bottom(obj, pad_y, 0);
}

void ui_pad_x(lv_obj_t *obj, int32_t pad_x) {
  lv_obj_set_style_pad_left(obj, pad_x, 0);
  lv_obj_set_style_pad_right(obj, pad_x, 0);
}

void ui_pad_y(lv_obj_t *obj, int32_t pad_y) {
  lv_obj_set_style_pad_top(obj, pad_y, 0);
  lv_obj_set_style_pad_bottom(obj, pad_y, 0);
}

void ui_gap(lv_obj_t *obj, int32_t gap) {
  lv_obj_set_style_pad_gap(obj, gap, 0);
}

void ui_margin_all(lv_obj_t *obj, int32_t margin) {
  lv_obj_set_style_margin_all(obj, margin, 0);
}

void ui_margin_xy(lv_obj_t *obj, int32_t margin_x, int32_t margin_y) {
  lv_obj_set_style_margin_left(obj, margin_x, 0);
  lv_obj_set_style_margin_right(obj, margin_x, 0);
  lv_obj_set_style_margin_top(obj, margin_y, 0);
  lv_obj_set_style_margin_bottom(obj, margin_y, 0);
}

void ui_margin_x(lv_obj_t *obj, int32_t margin_x) {
  lv_obj_set_style_margin_left(obj, margin_x, 0);
  lv_obj_set_style_margin_right(obj, margin_x, 0);
}

void ui_margin_y(lv_obj_t *obj, int32_t margin_y) {
  lv_obj_set_style_margin_top(obj, margin_y, 0);
  lv_obj_set_style_margin_bottom(obj, margin_y, 0);
}

/* =========================================================================
 * Size & Position
 * ========================================================================= */

void ui_fill_parent(lv_obj_t *obj) {
  lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
  lv_obj_set_pos(obj, 0, 0);
}

void ui_wrap_content(lv_obj_t *obj) {
  lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
}

void ui_width_content(lv_obj_t *obj) { lv_obj_set_width(obj, LV_SIZE_CONTENT); }

void ui_height_content(lv_obj_t *obj) {
  lv_obj_set_height(obj, LV_SIZE_CONTENT);
}

void ui_center_in_parent(lv_obj_t *obj) { lv_obj_center(obj); }

void ui_width(lv_obj_t *obj, int32_t w) { lv_obj_set_width(obj, w); }

void ui_width_pct(lv_obj_t *obj, int32_t pct) {
  lv_obj_set_width(obj, LV_PCT(pct));
}

void ui_height(lv_obj_t *obj, int32_t h) { lv_obj_set_height(obj, h); }

void ui_height_pct(lv_obj_t *obj, int32_t pct) {
  lv_obj_set_height(obj, LV_PCT(pct));
}

void ui_size(lv_obj_t *obj, int32_t w, int32_t h) {
  lv_obj_set_size(obj, w, h);
}

void ui_size_pct(lv_obj_t *obj, int32_t w_pct, int32_t h_pct) {
  lv_obj_set_size(obj, LV_PCT(w_pct), LV_PCT(h_pct));
}

void ui_width_fill(lv_obj_t *obj) {
  lv_obj_set_width(obj, LV_PCT(100));
  lv_obj_set_height(obj, LV_SIZE_CONTENT);
}

void ui_height_fill(lv_obj_t *obj) {
  lv_obj_set_width(obj, LV_SIZE_CONTENT);
  lv_obj_set_height(obj, LV_PCT(100));
}

/* =========================================================================
 * Flex core
 * ========================================================================= */

lv_obj_t *ui_flex_container(lv_obj_t *parent, lv_flex_flow_t flow,
                            lv_flex_align_t main_place,
                            lv_flex_align_t cross_place,
                            lv_flex_align_t track_place, int32_t gap,
                            int32_t pad_all) {
  lv_obj_t *cont = lv_obj_create(parent);

  lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(cont, flow);
  lv_obj_set_flex_align(cont, main_place, cross_place, track_place);

  ui_pad_all(cont, pad_all);
  ui_gap(cont, gap);

  return cont;
}

lv_obj_t *ui_clean_flex_container(lv_obj_t *parent, lv_flex_flow_t flow,
                                  lv_flex_align_t main_place,
                                  lv_flex_align_t cross_place,
                                  lv_flex_align_t track_place, int32_t gap,
                                  int32_t pad_all) {
  lv_obj_t *cont = ui_flex_container(parent, flow, main_place, cross_place,
                                     track_place, gap, pad_all);

  /* Make it layout-only; then restore requested spacing */
  ui_apply_clean(cont);
  ui_pad_all(cont, pad_all);
  ui_gap(cont, gap);

  return cont;
}

/* =========================================================================
 * Flex setters
 * ========================================================================= */

void ui_flex_flow(lv_obj_t *obj, lv_flex_flow_t flow) {
  lv_obj_set_layout(obj, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(obj, flow);
}

void ui_flex_align(lv_obj_t *obj, lv_flex_align_t main_place,
                   lv_flex_align_t cross_place, lv_flex_align_t track_place) {
  lv_obj_set_layout(obj, LV_LAYOUT_FLEX);
  lv_obj_set_flex_align(obj, main_place, cross_place, track_place);
}

void ui_flex_grow(lv_obj_t *obj, uint8_t grow) {
  lv_obj_set_flex_grow(obj, grow);
}

void ui_flex_center(lv_obj_t *obj) {
  ui_flex_align(obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                LV_FLEX_ALIGN_CENTER);
}

void ui_flex_between(lv_obj_t *obj) {
  ui_flex_align(obj, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER,
                LV_FLEX_ALIGN_CENTER);
}

void ui_flex_around(lv_obj_t *obj) {
  ui_flex_align(obj, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER,
                LV_FLEX_ALIGN_CENTER);
}

void ui_flex_start(lv_obj_t *obj) {
  ui_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                LV_FLEX_ALIGN_CENTER);
}

void ui_flex_end(lv_obj_t *obj) {
  ui_flex_align(obj, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER,
                LV_FLEX_ALIGN_CENTER);
}

lv_obj_t *ui_spacer(lv_obj_t *parent, uint8_t grow) {
  lv_obj_t *s = lv_obj_create(parent);

  lv_obj_set_size(s, 1, 1);
  lv_obj_set_flex_grow(s, grow ? grow : 1);

  ui_apply_clean(s);

  return s;
}

/* =========================================================================
 * Flex convenience wrappers (theme-visible)
 * ========================================================================= */

lv_obj_t *ui_row(lv_obj_t *parent, int32_t gap, int32_t pad_all) {
  return ui_flex_container(parent, LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, gap,
                           pad_all);
}

lv_obj_t *ui_col(lv_obj_t *parent, int32_t gap, int32_t pad_all) {
  return ui_flex_container(parent, LV_FLEX_FLOW_COLUMN, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, gap,
                           pad_all);
}

lv_obj_t *ui_row_center(lv_obj_t *parent, int32_t gap, int32_t pad_all) {
  return ui_flex_container(parent, LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_CENTER,
                           LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, gap,
                           pad_all);
}

lv_obj_t *ui_col_center(lv_obj_t *parent, int32_t gap, int32_t pad_all) {
  return ui_flex_container(parent, LV_FLEX_FLOW_COLUMN, LV_FLEX_ALIGN_CENTER,
                           LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, gap,
                           pad_all);
}

lv_obj_t *ui_row_wrap(lv_obj_t *parent, int32_t gap, int32_t pad_all) {
  return ui_flex_container(parent, LV_FLEX_FLOW_ROW_WRAP, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, gap,
                           pad_all);
}

lv_obj_t *ui_col_wrap(lv_obj_t *parent, int32_t gap, int32_t pad_all) {
  return ui_flex_container(parent, LV_FLEX_FLOW_COLUMN_WRAP,
                           LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START, gap, pad_all);
}

lv_obj_t *ui_row_reverse(lv_obj_t *parent, int32_t gap, int32_t pad_all) {
  return ui_flex_container(parent, LV_FLEX_FLOW_ROW_REVERSE,
                           LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                           LV_FLEX_ALIGN_START, gap, pad_all);
}

lv_obj_t *ui_col_reverse(lv_obj_t *parent, int32_t gap, int32_t pad_all) {
  return ui_flex_container(parent, LV_FLEX_FLOW_COLUMN_REVERSE,
                           LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START, gap, pad_all);
}

lv_obj_t *ui_row_wrap_reverse(lv_obj_t *parent, int32_t gap, int32_t pad_all) {
  return ui_flex_container(parent, LV_FLEX_FLOW_ROW_WRAP_REVERSE,
                           LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                           LV_FLEX_ALIGN_START, gap, pad_all);
}

lv_obj_t *ui_col_wrap_reverse(lv_obj_t *parent, int32_t gap, int32_t pad_all) {
  return ui_flex_container(parent, LV_FLEX_FLOW_COLUMN_WRAP_REVERSE,
                           LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START, gap, pad_all);
}

/* =========================================================================
 * Flex convenience wrappers (clean/layout-only)
 * ========================================================================= */

lv_obj_t *ui_clean_row(lv_obj_t *parent, int32_t gap, int32_t pad_all) {
  return ui_clean_flex_container(parent, LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_START,
                                 LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, gap,
                                 pad_all);
}

lv_obj_t *ui_clean_col(lv_obj_t *parent, int32_t gap, int32_t pad_all) {
  return ui_clean_flex_container(parent, LV_FLEX_FLOW_COLUMN,
                                 LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                                 LV_FLEX_ALIGN_START, gap, pad_all);
}

lv_obj_t *ui_clean_row_center(lv_obj_t *parent, int32_t gap, int32_t pad_all) {
  return ui_clean_flex_container(parent, LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_CENTER,
                                 LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                                 gap, pad_all);
}

lv_obj_t *ui_clean_col_center(lv_obj_t *parent, int32_t gap, int32_t pad_all) {
  return ui_clean_flex_container(parent, LV_FLEX_FLOW_COLUMN,
                                 LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                                 LV_FLEX_ALIGN_CENTER, gap, pad_all);
}

lv_obj_t *ui_clean_row_wrap(lv_obj_t *parent, int32_t gap, int32_t pad_all) {
  return ui_clean_flex_container(parent, LV_FLEX_FLOW_ROW_WRAP,
                                 LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                                 LV_FLEX_ALIGN_START, gap, pad_all);
}

lv_obj_t *ui_clean_col_wrap(lv_obj_t *parent, int32_t gap, int32_t pad_all) {
  return ui_clean_flex_container(parent, LV_FLEX_FLOW_COLUMN_WRAP,
                                 LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                                 LV_FLEX_ALIGN_START, gap, pad_all);
}

lv_obj_t *ui_clean_row_reverse(lv_obj_t *parent, int32_t gap, int32_t pad_all) {
  return ui_clean_flex_container(parent, LV_FLEX_FLOW_ROW_REVERSE,
                                 LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                                 LV_FLEX_ALIGN_START, gap, pad_all);
}

lv_obj_t *ui_clean_col_reverse(lv_obj_t *parent, int32_t gap, int32_t pad_all) {
  return ui_clean_flex_container(parent, LV_FLEX_FLOW_COLUMN_REVERSE,
                                 LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                                 LV_FLEX_ALIGN_START, gap, pad_all);
}

lv_obj_t *ui_clean_row_wrap_reverse(lv_obj_t *parent, int32_t gap,
                                    int32_t pad_all) {
  return ui_clean_flex_container(parent, LV_FLEX_FLOW_ROW_WRAP_REVERSE,
                                 LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                                 LV_FLEX_ALIGN_START, gap, pad_all);
}

lv_obj_t *ui_clean_col_wrap_reverse(lv_obj_t *parent, int32_t gap,
                                    int32_t pad_all) {
  return ui_clean_flex_container(parent, LV_FLEX_FLOW_COLUMN_WRAP_REVERSE,
                                 LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                                 LV_FLEX_ALIGN_START, gap, pad_all);
}

/* =========================================================================
 * Grid
 * ========================================================================= */

lv_obj_t *ui_grid_container(lv_obj_t *parent, const lv_coord_t *col_dsc,
                            const lv_coord_t *row_dsc, int32_t gap,
                            int32_t pad_all) {
  lv_obj_t *cont = lv_obj_create(parent);

  lv_obj_set_layout(cont, LV_LAYOUT_GRID);
  lv_obj_set_grid_dsc_array(cont, col_dsc, row_dsc);

  ui_pad_all(cont, pad_all);
  ui_gap(cont, gap);

  return cont;
}

lv_obj_t *ui_clean_grid_container(lv_obj_t *parent, const lv_coord_t *col_dsc,
                                  const lv_coord_t *row_dsc, int32_t gap,
                                  int32_t pad_all) {
  lv_obj_t *cont = ui_grid_container(parent, col_dsc, row_dsc, gap, pad_all);

  ui_apply_clean(cont);
  ui_pad_all(cont, pad_all);
  ui_gap(cont, gap);

  return cont;
}

void ui_grid_place(lv_obj_t *obj, uint8_t col, uint8_t col_span, uint8_t row,
                   uint8_t row_span, lv_grid_align_t x_align,
                   lv_grid_align_t y_align) {
  lv_obj_set_grid_cell(obj, x_align, col, col_span, y_align, row, row_span);
}

/* =========================================================================
 * Stack / Overlay
 * ========================================================================= */

lv_obj_t *ui_stack_container(lv_obj_t *parent, int32_t pad_all) {
  lv_obj_t *cont = lv_obj_create(parent);

  lv_obj_set_layout(cont, LV_LAYOUT_NONE);
  ui_pad_all(cont, pad_all);

  return cont;
}

lv_obj_t *ui_clean_stack_container(lv_obj_t *parent, int32_t pad_all) {
  lv_obj_t *cont = ui_stack_container(parent, pad_all);

  ui_apply_clean(cont);
  ui_pad_all(cont, pad_all);

  return cont;
}

/* =========================================================================
 * Clean containers
 * ========================================================================= */

lv_obj_t *ui_clean_container(lv_obj_t *parent) {
  lv_obj_t *cont = lv_obj_create(parent);
  ui_apply_clean(cont);
  return cont;
}

lv_obj_t *ui_clean_scroll_container(lv_obj_t *parent, lv_dir_t scroll_dir) {
  lv_obj_t *cont = lv_obj_create(parent);

  ui_apply_clean(cont);
  ui_enable_scroll(cont, scroll_dir);

  /* Scroll containers MUST be clickable to receive touch events for scrolling
   */
  ui_set_clickable(cont);

  return cont;
}

/* =========================================================================
 * Card / Pill (optional visuals)
 * ========================================================================= */

lv_obj_t *ui_card_container(lv_obj_t *parent, int32_t pad_all, int32_t radius) {
  lv_obj_t *card = lv_obj_create(parent);

  /* Let theme provide colors; set shape + spacing */
  lv_obj_set_style_radius(card, radius, 0);
  ui_pad_all(card, pad_all);

  /* Cards usually shouldn't scroll themselves */
  ui_disable_scroll(card);

  return card;
}

void ui_make_pill(lv_obj_t *obj, int32_t pad_x, int32_t pad_y) {
  /* A very large radius yields a pill/circle depending on size */
  lv_obj_set_style_radius(obj, 999, 0);
  ui_pad_xy(obj, pad_x, pad_y);
}

/* =========================================================================
 * Atomic Styling
 * ========================================================================= */

void ui_radius(lv_obj_t *obj, int32_t radius) {
  lv_obj_set_style_radius(obj, radius, 0);
}

void ui_bg_color(lv_obj_t *obj, lv_color_t color) {
  lv_obj_set_style_bg_color(obj, color, 0);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
}

void ui_bg_opa(lv_obj_t *obj, lv_opa_t opa) {
  lv_obj_set_style_bg_opa(obj, opa, 0);
}

void ui_border(lv_obj_t *obj, int32_t width, lv_color_t color, lv_opa_t opa) {
  lv_obj_set_style_border_width(obj, width, 0);
  lv_obj_set_style_border_color(obj, color, 0);
  lv_obj_set_style_border_opa(obj, opa, 0);
}

/* =========================================================================
 * Object State & Visibility
 * ========================================================================= */

void ui_show(lv_obj_t *obj) { lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN); }

void ui_hide(lv_obj_t *obj) { lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN); }

void ui_toggle(lv_obj_t *obj) {
  if (lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)) {
    ui_show(obj);
  } else {
    ui_hide(obj);
  }
}

void ui_opacity(lv_obj_t *obj, lv_opa_t opa) {
  lv_obj_set_style_opa(obj, opa, 0);
}
