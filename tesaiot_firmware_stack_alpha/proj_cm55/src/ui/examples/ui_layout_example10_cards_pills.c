#include "ui_layout_example10_cards_pills.h"
#include "ui_layout.h"
#include "ui_layout_examples_common.h"
#include <stdlib.h>

/* Context for blinking icons */
typedef struct {
  lv_obj_t *obj;
  lv_color_t color_orig;
} ui_blink_ctx_t;

/* Timer callback to toggle color */
static void ui_blink_timer_cb(lv_timer_t *t) {
  ui_blink_ctx_t *ctx = (ui_blink_ctx_t *)lv_timer_get_user_data(t);
  if (!lv_obj_is_valid(ctx->obj))
    return;

  lv_color_t cur = lv_obj_get_style_text_color(ctx->obj, 0);
  lv_color_t gray = lv_palette_main(LV_PALETTE_GREY);

  /* Simple check: if currently original color, switch to gray; else back */
  if (cur.red == ctx->color_orig.red && cur.green == ctx->color_orig.green &&
      cur.blue == ctx->color_orig.blue) {
    lv_obj_set_style_text_color(ctx->obj, gray, 0);
  } else {
    lv_obj_set_style_text_color(ctx->obj, ctx->color_orig, 0);
  }
}

/* Cleanup context and timer when object is deleted */
static void ui_blink_del_event_cb(lv_event_t *e) {
  lv_timer_t *t = (lv_timer_t *)lv_event_get_user_data(e);
  ui_blink_ctx_t *ctx = (ui_blink_ctx_t *)lv_timer_get_user_data(t);
  lv_timer_delete(t);
  lv_free(ctx);
}

/* Create a simple “card row”: left title/subtitle + right icon */
static lv_obj_t *make_card_row(lv_obj_t *parent, const char *title,
                               const char *subtitle, const char *right_symbol,
                               lv_color_t color) {
  lv_obj_t *card = ui_card_container(parent, 12, 12);
  ui_width_fill(card);

  /* Inside card: row with space-between */
  ui_flex_flow(card, LV_FLEX_FLOW_ROW);
  ui_flex_align(card, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER,
                LV_FLEX_ALIGN_CENTER);
  ui_gap(card, 10);

  /* Left column (title + subtitle) */
  lv_obj_t *left = ui_clean_col(card, 4, 0);
  ui_height_content(left);

  lv_obj_t *t = lv_label_create(left);
  lv_label_set_text(t, title ? title : "");
  ui_height_content(t);

  lv_obj_t *s = lv_label_create(left);
  lv_label_set_text(s, subtitle ? subtitle : "");
  lv_obj_set_style_text_opa(s, LV_OPA_70, 0);
  ui_height_content(s);

  /* Right icon/symbol */
  lv_obj_t *r = lv_label_create(card);
  lv_label_set_text(r, right_symbol ? right_symbol : LV_SYMBOL_RIGHT);
  lv_obj_set_style_text_color(r, color, 0);
  // lv_obj_set_style_transform_scale(r, 512, 0);
  ui_height_content(r);

  /* Setup blinking context */
  ui_blink_ctx_t *ctx = lv_malloc(sizeof(ui_blink_ctx_t));
  ctx->obj = r;
  ctx->color_orig = color;

  /* Every 500ms toggle color */
  lv_timer_t *timer = lv_timer_create(ui_blink_timer_cb, 500, ctx);

  /* Clean up when object r is deleted */
  lv_obj_add_event_cb(r, ui_blink_del_event_cb, LV_EVENT_DELETE, timer);

  /* DEBUG */
  // ui_ex_debug_border(card);
  // ui_ex_debug_border(left);
  // ui_ex_debug_border(t);
  // ui_ex_debug_border(s);
  // ui_ex_debug_border(r);

  return card;
}

/* Create a pill chip */
// static lv_obj_t *make_pill(lv_obj_t *parent, const char *text, int32_t pad_h,
//                            int32_t pad_v, bool selected) {
//   lv_obj_t *pill = lv_obj_create(parent);
//   ui_make_pill(pill, pad_h, pad_v);
//   ui_disable_scroll(pill);
//   ui_set_no_click(pill);

//   /* Visualize pill even if theme bg is subtle */
//   if (selected) {
//     lv_obj_set_style_bg_color(pill, lv_palette_main(LV_PALETTE_BLUE), 0);
//     lv_obj_set_style_bg_opa(pill, LV_OPA_40, 0);
//     lv_obj_set_style_border_color(pill, lv_palette_main(LV_PALETTE_BLUE), 0);
//     lv_obj_set_style_border_width(pill, 1, 0);
//   } else {
//     lv_obj_set_style_border_width(pill, 1, 0);
//     lv_obj_set_style_border_opa(pill, LV_OPA_40, 0);
//   }

//   lv_obj_t *lbl = lv_label_create(pill);
//   lv_label_set_text(lbl, text ? text : "");
//   lv_obj_center(lbl);

//   return pill;
// }

void ui_layout_example10_cards_pills(lv_obj_t *parent) {
  ui_ex_common_t ex =
      ui_ex_create(parent, "Example 10 - Cards & Pills",
                   "Card container usage + pill/chip sizing variants");

  /* -----------------------------
   * Cards section
   * ----------------------------- */
  ui_ex_section_title(ex.body, "Cards:");

  lv_obj_t *cards = ui_clean_col(ex.body, 10, 0);
  ui_width_fill(cards);

  make_card_row(cards, "Wi-Fi", "Connected", LV_SYMBOL_WIFI,
                lv_palette_main(LV_PALETTE_LIME));
  make_card_row(cards, "Battery", "82%", LV_SYMBOL_BATTERY_FULL,
                lv_palette_main(LV_PALETTE_CYAN));
  make_card_row(cards, "Storage", "12.4 / 32GB", LV_SYMBOL_SD_CARD,
                lv_palette_main(LV_PALETTE_DEEP_PURPLE));

  /* -----------------------------
   * Pills section
   * ----------------------------- */
  // ui_ex_section_title(ex.body, "Pills (row wrap):");

  // lv_obj_t *pills = ui_clean_row_wrap(ex.body, 8, 8);
  // lv_obj_set_width(pills, LV_PCT(100));
  // ui_ex_debug_border(pills);

  // make_pill(pills, "Small", 8, 4, false);
  // make_pill(pills, "Medium", 10, 6, true);
  // make_pill(pills, "Large", 14, 8, false);

  // make_pill(pills, "LVGL", 10, 6, false);
  // make_pill(pills, "Chip", 10, 6, false);
  // make_pill(pills, "Longer pill label", 12, 6, false);
  // make_pill(pills, "9.2", 10, 6, false);
  // make_pill(pills, "Wrap", 10, 6, false);

  /* Note */
  lv_obj_t *note = lv_label_create(ex.body);
  lv_label_set_text(note, "Pass:\n"
                          "- Cards have consistent padding/radius\n"
                          "- Pills are fully rounded\n"
                          "- Pills wrap to next line when needed");
  lv_obj_set_style_text_opa(note, LV_OPA_80, 0);
}
