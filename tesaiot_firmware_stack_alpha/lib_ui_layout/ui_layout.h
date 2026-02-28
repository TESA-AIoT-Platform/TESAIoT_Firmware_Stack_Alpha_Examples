/**
 * @file ui_layout.h
 * @brief Semantic abstractions for the LVGL 9.x layout engine.
 *
 * @author Asst.Prof.Dr.Santi Nuratch
 * @date   2026-01-23
 *
 * This library centralizes common layout patterns to reduce boilerplate
 * and ensure visual consistency across the application.
 */

#ifndef UI_LAYOUT_H
#define UI_LAYOUT_H

#include "lvgl.h"

/* =========================================================================
 * Core "polish" helpers
 * ========================================================================= */

void ui_container_clear_style(lv_obj_t *obj);

void ui_disable_scroll(lv_obj_t *obj);
void ui_enable_scroll(lv_obj_t *obj, lv_dir_t scroll_dir);

void ui_set_no_click(lv_obj_t *obj);
void ui_set_clickable(lv_obj_t *obj);

/* Clean = clear visuals + no click + no scroll */
void ui_apply_clean(lv_obj_t *obj);

/* Alias: semantic name used in UI code */
static inline void ui_layout_only(lv_obj_t *obj) { ui_apply_clean(obj); }

/* =========================================================================
 * Spacing helpers (layout-agnostic)
 * ========================================================================= */

void ui_pad_all(lv_obj_t *obj, int32_t pad);
void ui_pad_xy(lv_obj_t *obj, int32_t pad_x, int32_t pad_y);
void ui_pad_x(lv_obj_t *obj, int32_t pad_x);
void ui_pad_y(lv_obj_t *obj, int32_t pad_y);
void ui_gap(lv_obj_t *obj, int32_t gap);

void ui_margin_all(lv_obj_t *obj, int32_t margin);
void ui_margin_xy(lv_obj_t *obj, int32_t margin_x, int32_t margin_y);
void ui_margin_x(lv_obj_t *obj, int32_t margin_x);
void ui_margin_y(lv_obj_t *obj, int32_t margin_y);

/* =========================================================================
 * Size & Position
 * ========================================================================= */

void ui_fill_parent(lv_obj_t *obj);
void ui_wrap_content(lv_obj_t *obj);
void ui_width_content(lv_obj_t *obj);
void ui_height_content(lv_obj_t *obj);
void ui_center_in_parent(lv_obj_t *obj);

void ui_width(lv_obj_t *obj, int32_t w);
void ui_width_pct(lv_obj_t *obj, int32_t pct);
void ui_height(lv_obj_t *obj, int32_t h);
void ui_height_pct(lv_obj_t *obj, int32_t pct);
void ui_size(lv_obj_t *obj, int32_t w, int32_t h);
void ui_size_pct(lv_obj_t *obj, int32_t w_pct, int32_t h_pct);

void ui_width_fill(lv_obj_t *obj);
void ui_height_fill(lv_obj_t *obj);

/* =========================================================================
 * Flex
 * ========================================================================= */

/* Core creators */
lv_obj_t *ui_flex_container(lv_obj_t *parent, lv_flex_flow_t flow,
                            lv_flex_align_t main_place,
                            lv_flex_align_t cross_place,
                            lv_flex_align_t track_place, int32_t gap,
                            int32_t pad_all);

lv_obj_t *ui_clean_flex_container(lv_obj_t *parent, lv_flex_flow_t flow,
                                  lv_flex_align_t main_place,
                                  lv_flex_align_t cross_place,
                                  lv_flex_align_t track_place, int32_t gap,
                                  int32_t pad_all);

/* Convenience wrappers (theme-visible) */
lv_obj_t *ui_row(lv_obj_t *parent, int32_t gap, int32_t pad_all);
lv_obj_t *ui_col(lv_obj_t *parent, int32_t gap, int32_t pad_all);
lv_obj_t *ui_row_center(lv_obj_t *parent, int32_t gap, int32_t pad_all);
lv_obj_t *ui_col_center(lv_obj_t *parent, int32_t gap, int32_t pad_all);

lv_obj_t *ui_row_wrap(lv_obj_t *parent, int32_t gap, int32_t pad_all);
lv_obj_t *ui_col_wrap(lv_obj_t *parent, int32_t gap, int32_t pad_all);

lv_obj_t *ui_row_reverse(lv_obj_t *parent, int32_t gap, int32_t pad_all);
lv_obj_t *ui_col_reverse(lv_obj_t *parent, int32_t gap, int32_t pad_all);

lv_obj_t *ui_row_wrap_reverse(lv_obj_t *parent, int32_t gap, int32_t pad_all);
lv_obj_t *ui_col_wrap_reverse(lv_obj_t *parent, int32_t gap, int32_t pad_all);

/* Convenience wrappers (clean/layout-only) */
lv_obj_t *ui_clean_row(lv_obj_t *parent, int32_t gap, int32_t pad_all);
lv_obj_t *ui_clean_col(lv_obj_t *parent, int32_t gap, int32_t pad_all);
lv_obj_t *ui_clean_row_center(lv_obj_t *parent, int32_t gap, int32_t pad_all);
lv_obj_t *ui_clean_col_center(lv_obj_t *parent, int32_t gap, int32_t pad_all);

lv_obj_t *ui_clean_row_wrap(lv_obj_t *parent, int32_t gap, int32_t pad_all);
lv_obj_t *ui_clean_col_wrap(lv_obj_t *parent, int32_t gap, int32_t pad_all);

lv_obj_t *ui_clean_row_reverse(lv_obj_t *parent, int32_t gap, int32_t pad_all);
lv_obj_t *ui_clean_col_reverse(lv_obj_t *parent, int32_t gap, int32_t pad_all);

lv_obj_t *ui_clean_row_wrap_reverse(lv_obj_t *parent, int32_t gap,
                                    int32_t pad_all);
lv_obj_t *ui_clean_col_wrap_reverse(lv_obj_t *parent, int32_t gap,
                                    int32_t pad_all);

/* Flex setters (useful for existing objects) */
void ui_flex_flow(lv_obj_t *obj, lv_flex_flow_t flow);
void ui_flex_align(lv_obj_t *obj, lv_flex_align_t main_place,
                   lv_flex_align_t cross_place, lv_flex_align_t track_place);

void ui_flex_grow(lv_obj_t *obj, uint8_t grow);

/* Flex presets */
void ui_flex_center(lv_obj_t *obj);
void ui_flex_between(lv_obj_t *obj);
void ui_flex_around(lv_obj_t *obj);
void ui_flex_start(lv_obj_t *obj);
void ui_flex_end(lv_obj_t *obj);

/* Spacer: invisible flex item that grows */
lv_obj_t *ui_spacer(lv_obj_t *parent, uint8_t grow);

/* =========================================================================
 * Grid
 * ========================================================================= */

lv_obj_t *ui_grid_container(lv_obj_t *parent, const lv_coord_t *col_dsc,
                            const lv_coord_t *row_dsc, int32_t gap,
                            int32_t pad_all);

lv_obj_t *ui_clean_grid_container(lv_obj_t *parent, const lv_coord_t *col_dsc,
                                  const lv_coord_t *row_dsc, int32_t gap,
                                  int32_t pad_all);

void ui_grid_place(lv_obj_t *obj, uint8_t col, uint8_t col_span, uint8_t row,
                   uint8_t row_span, lv_grid_align_t x_align,
                   lv_grid_align_t y_align);

/* =========================================================================
 * Stack / Overlay
 * ========================================================================= */

lv_obj_t *ui_stack_container(lv_obj_t *parent, int32_t pad_all);
lv_obj_t *ui_clean_stack_container(lv_obj_t *parent, int32_t pad_all);

/* =========================================================================
 * Clean containers
 * ========================================================================= */

lv_obj_t *ui_clean_container(lv_obj_t *parent);
lv_obj_t *ui_clean_scroll_container(lv_obj_t *parent, lv_dir_t scroll_dir);

/* =========================================================================
 * Card / Pill (optional visuals)
 * ========================================================================= */

lv_obj_t *ui_card_container(lv_obj_t *parent, int32_t pad_all, int32_t radius);

/* Make obj look like a chip/pill (radius=999 + padding). */
void ui_make_pill(lv_obj_t *obj, int32_t pad_x, int32_t pad_y);

/* =========================================================================
 * Atomic Styling
 * ========================================================================= */

void ui_radius(lv_obj_t *obj, int32_t radius);
void ui_bg_color(lv_obj_t *obj, lv_color_t color);
void ui_bg_opa(lv_obj_t *obj, lv_opa_t opa);
void ui_border(lv_obj_t *obj, int32_t width, lv_color_t color, lv_opa_t opa);

/* =========================================================================
 * Object State & Visibility
 * ========================================================================= */

void ui_show(lv_obj_t *obj);
void ui_hide(lv_obj_t *obj);
void ui_toggle(lv_obj_t *obj);
void ui_opacity(lv_obj_t *obj, lv_opa_t opa);

#endif /* UI_LAYOUT_H */
