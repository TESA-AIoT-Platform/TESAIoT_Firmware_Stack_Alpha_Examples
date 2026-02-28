#include "ui_layout_example11_symbols.h"
#include "ui_layout.h"
#include "ui_layout_examples_common.h"

typedef struct {
  const char *symbol;
  const char *name;
} ui_symbol_item_t;

static const ui_symbol_item_t media_symbols[] = {
    {LV_SYMBOL_AUDIO, "AUDIO"},        {LV_SYMBOL_VIDEO, "VIDEO"},
    {LV_SYMBOL_LIST, "LIST"},          {LV_SYMBOL_MUTE, "MUTE"},
    {LV_SYMBOL_VOLUME_MID, "VOL_MID"}, {LV_SYMBOL_VOLUME_MAX, "VOL_MAX"},
    {LV_SYMBOL_IMAGE, "IMAGE"},        {LV_SYMBOL_PREV, "PREV"},
    {LV_SYMBOL_PLAY, "PLAY"},          {LV_SYMBOL_PAUSE, "PAUSE"},
    {LV_SYMBOL_STOP, "STOP"},          {LV_SYMBOL_NEXT, "NEXT"},
    {LV_SYMBOL_EJECT, "EJECT"},        {LV_SYMBOL_SHUFFLE, "SHUFFLE"},
    {LV_SYMBOL_LOOP, "LOOP"},          {NULL, NULL}};

static const ui_symbol_item_t common_symbols[] = {
    {LV_SYMBOL_BULLET, "BULLET"},
    {LV_SYMBOL_OK, "OK"},
    {LV_SYMBOL_CLOSE, "CLOSE"},
    {LV_SYMBOL_POWER, "POWER"},
    {LV_SYMBOL_SETTINGS, "SETTINGS"},
    {LV_SYMBOL_HOME, "HOME"},
    {LV_SYMBOL_DOWNLOAD, "DOWNLOAD"},
    {LV_SYMBOL_DRIVE, "DRIVE"},
    {LV_SYMBOL_REFRESH, "REFRESH"},
    {LV_SYMBOL_TINT, "TINT"},
    {LV_SYMBOL_PLUS, "PLUS"},
    {LV_SYMBOL_MINUS, "MINUS"},
    {LV_SYMBOL_EYE_OPEN, "EYE_OPEN"},
    {LV_SYMBOL_EYE_CLOSE, "EYE_CLOSE"},
    {LV_SYMBOL_WARNING, "WARNING"},
    {LV_SYMBOL_DIRECTORY, "DIR"},
    {LV_SYMBOL_UPLOAD, "UPLOAD"},
    {LV_SYMBOL_CALL, "CALL"},
    {LV_SYMBOL_CUT, "CUT"},
    {LV_SYMBOL_COPY, "COPY"},
    {LV_SYMBOL_SAVE, "SAVE"},
    {LV_SYMBOL_BARS, "BARS"},
    {LV_SYMBOL_ENVELOPE, "ENVELOPE"},
    {LV_SYMBOL_CHARGE, "CHARGE"},
    {LV_SYMBOL_PASTE, "PASTE"},
    {LV_SYMBOL_BELL, "BELL"},
    {LV_SYMBOL_KEYBOARD, "KBD"},
    {LV_SYMBOL_GPS, "GPS"},
    {LV_SYMBOL_FILE, "FILE"},
    {LV_SYMBOL_TRASH, "TRASH"},
    {LV_SYMBOL_EDIT, "EDIT"},
    {LV_SYMBOL_BACKSPACE, "BACKSP"},
    {LV_SYMBOL_SD_CARD, "SD_CARD"},
    {LV_SYMBOL_NEW_LINE, "NEW_LINE"},
    {NULL, NULL}};

static const ui_symbol_item_t hardware_symbols[] = {
    {LV_SYMBOL_WIFI, "WIFI"},
    {LV_SYMBOL_BATTERY_FULL, "BAT_FULL"},
    {LV_SYMBOL_BATTERY_3, "BAT_3"},
    {LV_SYMBOL_BATTERY_2, "BAT_2"},
    {LV_SYMBOL_BATTERY_1, "BAT_1"},
    {LV_SYMBOL_BATTERY_EMPTY, "BAT_EMPTY"},
    {LV_SYMBOL_USB, "USB"},
    {LV_SYMBOL_BLUETOOTH, "BLUETOOTH"},
    {NULL, NULL}};

static const ui_symbol_item_t arrow_symbols[] = {{LV_SYMBOL_LEFT, "LEFT"},
                                                 {LV_SYMBOL_RIGHT, "RIGHT"},
                                                 {LV_SYMBOL_UP, "UP"},
                                                 {LV_SYMBOL_DOWN, "DOWN"},
                                                 {NULL, NULL}};

static void add_symbols_to_container(lv_obj_t *parent,
                                     const ui_symbol_item_t *items) {
  /* Use a wrapping row for the symbols */
  lv_obj_t *grid = ui_row_wrap(parent, 10, 10);
  ui_width_fill(grid);
  ui_height_content(grid);

  for (int i = 0; items[i].symbol != NULL; i++) {
    /* Create a small card for each symbol */
    lv_obj_t *card = ui_card_container(grid, 8, 8);
    ui_size(card, 120, 74);
    ui_set_no_click(card);

    /* Center content in card */
    ui_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    ui_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                  LV_FLEX_ALIGN_CENTER);
    ui_gap(card, 4);

    /* Symbol label */
    lv_obj_t *s_lbl = lv_label_create(card);
    lv_label_set_text(s_lbl, items[i].symbol);
    lv_obj_set_style_text_font(s_lbl, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(s_lbl, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_text_align(s_lbl, LV_TEXT_ALIGN_CENTER, 0);
    ui_height_content(s_lbl);

    /* Name label */
    lv_obj_t *n_lbl = lv_label_create(card);
    lv_label_set_text(n_lbl, items[i].name);
    lv_obj_set_style_text_font(n_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_opa(n_lbl, LV_OPA_60, 0);
    lv_obj_set_style_text_align(n_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(n_lbl, LV_LABEL_LONG_SCROLL_CIRCULAR);
    ui_width_fill(n_lbl);
    ui_height_content(n_lbl);
  }
}

void ui_layout_example11_symbols(lv_obj_t *parent) {
  ui_ex_common_t ex = ui_ex_create(parent, "Example 11 - Symbols Browser",
                                   "Categorized built-in FontAwesome symbols");

  /* Tab View for categories */
  lv_obj_t *tv = lv_tabview_create(ex.body);
  ui_fill_parent(tv);
  lv_tabview_set_tab_bar_position(tv, LV_DIR_TOP);
  /* Dark theme for tab bar */
  lv_obj_t *tab_bar = lv_tabview_get_tab_bar(tv);
  lv_obj_set_style_bg_color(tab_bar, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
  lv_obj_set_style_text_color(tab_bar, lv_color_white(), 0);
  lv_obj_set_style_bg_opa(tab_bar, LV_OPA_COVER, 0);

  /* Tab 1: Media */
  lv_obj_t *t1 = lv_tabview_add_tab(tv, "Media");
  ui_enable_scroll(t1, LV_DIR_VER);
  add_symbols_to_container(t1, media_symbols);

  /* Tab 2: Common */
  lv_obj_t *t2 = lv_tabview_add_tab(tv, "Common");
  ui_enable_scroll(t2, LV_DIR_VER);
  add_symbols_to_container(t2, common_symbols);

  /* Tab 3: Hardware */
  lv_obj_t *t3 = lv_tabview_add_tab(tv, "Hardware");
  ui_enable_scroll(t3, LV_DIR_VER);
  add_symbols_to_container(t3, hardware_symbols);

  /* Tab 4: Arrows */
  lv_obj_t *t4 = lv_tabview_add_tab(tv, "Arrows");
  ui_enable_scroll(t4, LV_DIR_VER);
  add_symbols_to_container(t4, arrow_symbols);

  /* Final note at the bottom of the body (if needed) -
   * actually body is filled by tabview */
}
