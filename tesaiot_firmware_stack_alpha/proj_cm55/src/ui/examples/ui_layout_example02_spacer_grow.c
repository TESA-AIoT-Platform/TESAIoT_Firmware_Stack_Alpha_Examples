#include "ui_layout_example02_spacer_grow.h"
#include "lvgl.h"
#include "ui_layout.h"

/* Helper: button with centered label */
static lv_obj_t *make_btn(lv_obj_t *parent, const char *txt, lv_coord_t w,
                          lv_coord_t h) {
  lv_obj_t *btn = lv_btn_create(parent);
  if (w > 0)
    lv_obj_set_width(btn, w);
  if (h > 0)
    lv_obj_set_height(btn, h);

  lv_obj_t *lbl = lv_label_create(btn);
  lv_label_set_text(lbl, txt);
  lv_obj_center(lbl);

  return btn;
}

static lv_obj_t *make_box(lv_obj_t *parent, const char *txt, lv_coord_t w,
                          lv_coord_t h) {
  lv_obj_t *box = lv_obj_create(parent);
  if (w > 0)
    lv_obj_set_width(box, w);
  if (h > 0)
    lv_obj_set_height(box, h);

  /* light border so it's visible even if theme bg is transparent */
  lv_obj_set_style_border_width(box, 1, 0);
  lv_obj_set_style_border_opa(box, LV_OPA_50, 0);
  lv_obj_set_style_radius(box, 8, 0);

  /* small internal padding */
  lv_obj_set_style_pad_all(box, 8, 0);

  lv_obj_t *lbl = lv_label_create(box);
  lv_label_set_text(lbl, txt);
  lv_obj_center(lbl);

  return box;
}

void ui_layout_example02_spacer_grow(lv_obj_t *parent) {
  /* Root */
  lv_obj_t *root = ui_clean_col(parent, 12, 12);
  lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));

  /* Title */
  lv_obj_t *title = lv_label_create(root);
  lv_label_set_text(title,
                    "Example 02 - Spacer & Grow\nToolbar spacer + flex_grow");

  /* -----------------------------
   * Toolbar with spacer
   * ----------------------------- */
  lv_obj_t *bar = ui_clean_row(root, 8, 8);
  lv_obj_set_width(bar, LV_PCT(100));

  /* show bar bounds */
  lv_obj_set_style_border_width(bar, 1, 0);
  lv_obj_set_style_border_opa(bar, LV_OPA_50, 0);
  lv_obj_set_style_radius(bar, 8, 0);

  make_btn(bar, LV_SYMBOL_LEFT, 44, 40);

  lv_obj_t *bar_title = lv_label_create(bar);
  lv_label_set_text(bar_title, "Toolbar");

  /* Spacer pushes right side to edge */
  ui_spacer(bar, 1);

  make_btn(bar, LV_SYMBOL_SETTINGS, 44, 40);
  make_btn(bar, LV_SYMBOL_LIST, 44, 40);

  /* -----------------------------
   * Grow demo row
   * ----------------------------- */
  lv_obj_t *sec = lv_label_create(root);
  lv_label_set_text(sec, "Grow demo (middle expands):");

  lv_obj_t *grow_row = ui_clean_row(root, 8, 8);
  lv_obj_set_width(grow_row, LV_PCT(100));

  lv_obj_set_style_border_width(grow_row, 1, 0);
  lv_obj_set_style_border_opa(grow_row, LV_OPA_50, 0);
  lv_obj_set_style_radius(grow_row, 8, 0);

  lv_obj_t *left = make_box(grow_row, "Fixed", 80, 50);
  (void)left;

  lv_obj_t *mid = make_box(grow_row, "Grow", 0, 50); /* width auto */
  ui_flex_grow(mid, 1);

  lv_obj_t *right = make_box(grow_row, "Fixed", 80, 50);
  (void)right;

  lv_obj_t *note = lv_label_create(root);
  lv_label_set_text(note, "Pass:\n- Right buttons are pinned to the end\n- "
                          "Middle box expands to fill remaining space");
}
