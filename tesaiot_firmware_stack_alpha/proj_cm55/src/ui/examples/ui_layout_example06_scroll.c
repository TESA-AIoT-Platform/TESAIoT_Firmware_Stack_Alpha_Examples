#include "ui_layout_example06_scroll.h"
#include "ui_layout.h"
#include "ui_layout_examples_common.h"

/* Simple “feed item” as a card-like container */
static lv_obj_t *make_feed_item(lv_obj_t *parent, int idx) {
  lv_obj_t *card = ui_card_container(parent, 12, 12);
  ui_width_pct(card, 100);
  ui_height(card, 50);

  /* Make sure card contents are laid out nicely */
  ui_flex_between(card);
  ui_gap(card, 8);

  char left_txt[32];
  lv_snprintf(left_txt, sizeof(left_txt), "Item %02d", idx);

  lv_obj_t *left = lv_label_create(card);
  lv_label_set_text(left, left_txt);

  lv_obj_t *right = lv_label_create(card);
  lv_label_set_text(right, LV_SYMBOL_RIGHT);

  return card;
}

/* Horizontal card for the strip */
static lv_obj_t *make_strip_card(lv_obj_t *parent, int idx) {
  lv_obj_t *card = ui_card_container(parent, 12, 12);
  ui_size(card, 140, 50);

  /* Use a simple centered label */
  lv_obj_t *lbl = lv_label_create(card);
  char txt[32];
  lv_snprintf(txt, sizeof(txt), "Card %d", idx);
  lv_label_set_text(lbl, txt);
  lv_obj_center(lbl);

  return card;
}

void ui_layout_example06_scroll(lv_obj_t *parent) {
  ui_ex_common_t ex =
      ui_ex_create(parent, "Example 06 - Scroll",
                   "Vertical feed + horizontal strip (scroll containers)");

  /* -----------------------------
   * Vertical scroll feed
   * ----------------------------- */
  ui_ex_section_title(ex.body, "Vertical scroll feed:");

  lv_obj_t *feed_sc = ui_clean_scroll_container(ex.body, LV_DIR_VER);
  ui_width_pct(feed_sc, 100);
  ui_height(feed_sc, 240);
  ui_ex_debug_border(feed_sc);

  /* Inside the scroll container: a clean column */
  lv_obj_t *feed_col = ui_clean_col(feed_sc, 8, 8);
  ui_width_fill(feed_col);

  /* Enable scrollbar visibility for this example */
  lv_obj_set_scrollbar_mode(feed_sc, LV_SCROLLBAR_MODE_AUTO);

  for (int i = 1; i <= 20; i++) {
    make_feed_item(feed_col, i);
  }

  /* -----------------------------
   * Horizontal scroll strip
   * ----------------------------- */
  ui_ex_section_title(ex.body, "Horizontal scroll strip:");

  lv_obj_t *strip_sc = ui_clean_scroll_container(ex.body, LV_DIR_HOR);
  ui_width_pct(strip_sc, 100);
  ui_height(strip_sc, 120);
  ui_ex_debug_border(strip_sc);

  /* Inside: a clean row with padding and gap */
  lv_obj_t *strip_row = ui_clean_row(strip_sc, 10, 10);
  ui_height_fill(strip_row);

  /* Enable scrollbar visibility for this example */
  lv_obj_set_scrollbar_mode(strip_sc, LV_SCROLLBAR_MODE_AUTO);

  for (int i = 1; i <= 10; i++) {
    make_strip_card(strip_row, i);
  }

  /* Note */
  lv_obj_t *note = lv_label_create(ex.body);
  lv_label_set_text(note,
                    "Pass:\n"
                    "- Feed scrolls vertically\n"
                    "- Strip scrolls horizontally\n"
                    "- Content is inside scroll containers, not on the body");
  lv_obj_set_style_text_opa(note, LV_OPA_80, 0);
}
