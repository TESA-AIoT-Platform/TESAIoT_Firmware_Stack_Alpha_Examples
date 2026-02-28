#include "ui_layout_examples_common.h"
#include "ui_layout.h"

/* You can tweak these to match your preferred spacing scale */
#define UI_EX_PAD_SCREEN 12
#define UI_EX_GAP_ROOT 12
#define UI_EX_GAP_HEADER 6
#define UI_EX_GAP_BODY 10

void ui_ex_debug_border(lv_obj_t *obj) {
  ui_border(obj, 1, lv_palette_main(LV_PALETTE_YELLOW), LV_OPA_20);
  ui_radius(obj, 4);
}

ui_ex_common_t ui_ex_create(lv_obj_t *parent, const char *title,
                            const char *subtitle) {
  ui_ex_common_t ex;
  ex.root = NULL;
  ex.header = NULL;
  ex.body = NULL;
  ex.title_lbl = NULL;
  ex.subtitle_lbl = NULL;

  /* Root: a clean column with screen padding */
  ex.root = ui_clean_col(parent, UI_EX_GAP_ROOT, UI_EX_PAD_SCREEN);
  ui_fill_parent(ex.root);

  /* Header: clean column (title + subtitle) */
  ex.header = ui_clean_col(ex.root, UI_EX_GAP_HEADER, 0);
  ui_width_fill(ex.header);
  ui_ex_debug_border(ex.header);
  /* padding using new helpers */
  ui_pad_y(ex.header, 8);
  ui_pad_x(ex.header, 8);

  ex.title_lbl = lv_label_create(ex.header);
  lv_label_set_text(ex.title_lbl, title ? title : "");

  if (subtitle && subtitle[0] != '\0') {
    ex.subtitle_lbl = lv_label_create(ex.header);
    lv_label_set_text(ex.subtitle_lbl, subtitle);
    /* Make subtitle a bit “quieter” */
    lv_obj_set_style_text_opa(ex.subtitle_lbl, LV_OPA_70, 0);
  }

  /* Body: clean column, fills remaining space */
  ex.body = ui_clean_col(ex.root, UI_EX_GAP_BODY, 0);
  lv_obj_set_size(ex.body, LV_PCT(100), LV_PCT(100));
  ui_flex_grow(ex.body, 1);

  return ex;
}

lv_obj_t *ui_ex_section_title(lv_obj_t *parent, const char *text) {
  lv_obj_t *lbl = lv_label_create(parent);
  lv_label_set_text(lbl, text ? text : "");
  lv_obj_set_style_text_opa(lbl, LV_OPA_80, 0);
  return lbl;
}

lv_obj_t *ui_ex_button(lv_obj_t *parent, const char *text, lv_coord_t w,
                       lv_coord_t h) {
  lv_obj_t *btn = lv_btn_create(parent);
  if (w > 0)
    lv_obj_set_width(btn, w);
  if (h > 0)
    lv_obj_set_height(btn, h);

  lv_obj_t *lbl = lv_label_create(btn);
  lv_label_set_text(lbl, text ? text : "");
  lv_obj_center(lbl);

  return btn;
}

lv_obj_t *ui_ex_box(lv_obj_t *parent, const char *text, lv_coord_t w,
                    lv_coord_t h) {
  lv_obj_t *box = lv_obj_create(parent);
  if (w > 0)
    lv_obj_set_width(box, w);
  if (h > 0)
    lv_obj_set_height(box, h);

  /* Make it visible */
  ui_ex_debug_border(box);
  ui_pad_all(box, 8);

  lv_obj_t *lbl = lv_label_create(box);
  lv_label_set_text(lbl, text ? text : "");
  lv_obj_center(lbl);

  return box;
}
