#include "wifi_operator_dashboard.h"

#include "cm55_ipc_app.h"
#include "keyboard.h"
#include "ui_layout.h"
#include "ui_style.h"
#include "ui_theme.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define WIFI_UI_SCAN_LIST_MAX (16U)
#define WIFI_UI_DEBUG_TEXT_MAX (1800U)
#define WIFI_UI_REFRESH_DEFAULT_MS (500U)

typedef struct
{
  ipc_wifi_status_t status;
  wifi_info_t scan_list[WIFI_UI_SCAN_LIST_MAX];
  uint32_t scan_count;
  bool scan_requested;
  char debug_text[WIFI_UI_DEBUG_TEXT_MAX];
  uint32_t debug_sequence;
  bool status_dirty;
  bool scan_dirty;
  bool debug_dirty;
} wifi_ui_store_t;

typedef struct
{
  lv_timer_t *refresh_timer;
  lv_obj_t *header_badge;
  lv_obj_t *status_state_val;
  lv_obj_t *status_reason_val;
  lv_obj_t *status_rssi_val;
  lv_obj_t *status_ssid_val;
  lv_obj_t *status_rssi_chip;
  lv_obj_t *scan_list_cont;
  lv_obj_t *debug_ta;
  lv_obj_t *ssid_ta;
  lv_obj_t *pass_ta;
  lv_obj_t *keyboard;
  lv_obj_t *refresh_dd;
  uint32_t refresh_ms;
  wifi_ui_store_t store;
} wifi_dashboard_ctx_t;

static wifi_dashboard_ctx_t s_ctx;

static const char *wifi_state_to_text(uint8_t state)
{
  if ((uint8_t)IPC_WIFI_LINK_DISCONNECTED == state)
  {
    return "DISCONNECTED";
  }
  if ((uint8_t)IPC_WIFI_LINK_CONNECTING == state)
  {
    return "CONNECTING";
  }
  if ((uint8_t)IPC_WIFI_LINK_CONNECTED == state)
  {
    return "CONNECTED";
  }
  if ((uint8_t)IPC_WIFI_LINK_SCANNING == state)
  {
    return "SCANNING";
  }
  if ((uint8_t)IPC_WIFI_LINK_ERROR == state)
  {
    return "ERROR";
  }
  return "UNKNOWN";
}

static const char *wifi_reason_to_text(uint16_t reason)
{
  if ((uint16_t)IPC_WIFI_REASON_NONE == reason)
  {
    return "NONE";
  }
  if ((uint16_t)IPC_WIFI_REASON_SCAN_BLOCKED_CONNECTED == reason)
  {
    return "SCAN_BLOCKED";
  }
  if ((uint16_t)IPC_WIFI_REASON_SCAN_FAILED == reason)
  {
    return "SCAN_FAILED";
  }
  if ((uint16_t)IPC_WIFI_REASON_CONNECT_FAILED == reason)
  {
    return "CONNECT_FAILED";
  }
  if ((uint16_t)IPC_WIFI_REASON_DISCONNECTED == reason)
  {
    return "DISCONNECTED";
  }
  return "UNKNOWN";
}

static lv_color_t wifi_rssi_color(int32_t rssi)
{
  if (rssi >= -50)
  {
    return UI_COLOR_SUCCESS;
  }
  if (rssi >= -70)
  {
    return UI_COLOR_WARNING;
  }
  return UI_COLOR_DANGER;
}

static lv_obj_t *dashboard_card(lv_obj_t *parent, const char *title, const char *symbol)
{
  lv_obj_t *card = ui_card_container(parent, UI_SPACE_L, UI_RADIUS_CARD);
  lv_obj_t *row = ui_clean_row(card, UI_GAP_NORMAL, 0);
  lv_obj_t *icon = lv_label_create(row);
  lv_obj_t *label = lv_label_create(row);
  ui_width_fill(card);
  ui_height_content(card);
  ui_theme_apply_card(card);
  ui_flex_flow(card, LV_FLEX_FLOW_COLUMN);
  ui_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_label_set_text(icon, symbol);
  lv_obj_set_style_text_color(icon, UI_COLOR_PRIMARY, 0);
  lv_obj_set_style_text_font(icon, UI_FONT_SECTION, 0);
  lv_label_set_text(label, title);
  lv_obj_set_style_text_color(label, UI_COLOR_TEXT, 0);
  lv_obj_set_style_text_font(label, UI_FONT_SECTION, 0);
  return card;
}

static lv_obj_t *dashboard_key_value(lv_obj_t *parent, const char *key, lv_obj_t **value_out)
{
  lv_obj_t *row = ui_clean_row(parent, UI_GAP_NORMAL, 0);
  lv_obj_t *key_lbl = lv_label_create(row);
  lv_obj_t *value_lbl = lv_label_create(row);
  ui_width_fill(row);
  lv_label_set_text(key_lbl, key);
  lv_obj_set_style_text_color(key_lbl, UI_COLOR_TEXT_MUTED, 0);
  lv_obj_set_style_text_font(key_lbl, UI_FONT_BODY, 0);
  ui_flex_grow(value_lbl, 1);
  lv_obj_set_style_text_align(value_lbl, LV_TEXT_ALIGN_RIGHT, 0);
  ui_theme_apply_value(value_lbl);
  lv_label_set_text(value_lbl, "-");
  if (NULL != value_out)
  {
    *value_out = value_lbl;
  }
  return row;
}

static void dashboard_store_init(wifi_dashboard_ctx_t *ctx)
{
  if (NULL == ctx)
  {
    return;
  }
  (void)memset(&ctx->store, 0, sizeof(ctx->store));
  ctx->store.status.state = (uint8_t)IPC_WIFI_LINK_DISCONNECTED;
  ctx->store.status.reason = (uint16_t)IPC_WIFI_REASON_NONE;
  ctx->store.status.rssi = -127;
  ctx->store.status_dirty = true;
  ctx->store.scan_dirty = true;
  ctx->store.debug_dirty = true;
}

static void dashboard_store_poll(wifi_dashboard_ctx_t *ctx)
{
  ipc_wifi_status_t new_status;
  wifi_info_t new_list[WIFI_UI_SCAN_LIST_MAX];
  uint32_t new_count;
  uint32_t debug_seq;

  if (NULL == ctx)
  {
    return;
  }

  if (cm55_get_wifi_status(&new_status))
  {
    if (0 != memcmp(&ctx->store.status, &new_status, sizeof(new_status)))
    {
      (void)memcpy(&ctx->store.status, &new_status, sizeof(new_status));
      ctx->store.status_dirty = true;
    }
  }

  debug_seq = cm55_get_wifi_debug_sequence();
  if (debug_seq != ctx->store.debug_sequence)
  {
    (void)cm55_get_wifi_debug_text(ctx->store.debug_text, sizeof(ctx->store.debug_text));
    ctx->store.debug_sequence = debug_seq;
    ctx->store.debug_dirty = true;
  }

  new_count = 0U;
  if (cm55_get_wifi_list(new_list, WIFI_UI_SCAN_LIST_MAX, &new_count))
  {
    (void)memset(ctx->store.scan_list, 0, sizeof(ctx->store.scan_list));
    if (new_count > WIFI_UI_SCAN_LIST_MAX)
    {
      new_count = WIFI_UI_SCAN_LIST_MAX;
    }
    if (new_count > 0U)
    {
      (void)memcpy(ctx->store.scan_list, new_list, sizeof(wifi_info_t) * new_count);
    }
    ctx->store.scan_count = new_count;
    ctx->store.scan_dirty = true;
  }
}

static void dashboard_show_keyboard(wifi_dashboard_ctx_t *ctx, lv_obj_t *ta)
{
  if ((NULL == ctx) || (NULL == ctx->keyboard) || (NULL == ta))
  {
    return;
  }
  keyboard_set_textarea(ctx->keyboard, ta);
  ui_show(ctx->keyboard);
}

static void dashboard_hide_keyboard(wifi_dashboard_ctx_t *ctx)
{
  if ((NULL == ctx) || (NULL == ctx->keyboard))
  {
    return;
  }
  keyboard_set_textarea(ctx->keyboard, NULL);
  ui_hide(ctx->keyboard);
}

static void dashboard_ta_event_cb(lv_event_t *e)
{
  wifi_dashboard_ctx_t *ctx = (wifi_dashboard_ctx_t *)lv_event_get_user_data(e);
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  if ((NULL == ctx) || (NULL == ta))
  {
    return;
  }
  if (LV_EVENT_FOCUSED == code)
  {
    dashboard_show_keyboard(ctx, ta);
  }
  else if ((LV_EVENT_DEFOCUSED == code) || (LV_EVENT_READY == code) || (LV_EVENT_CANCEL == code))
  {
    dashboard_hide_keyboard(ctx);
  }
}

static void dashboard_keyboard_event_cb(lv_event_t *e)
{
  wifi_dashboard_ctx_t *ctx = (wifi_dashboard_ctx_t *)lv_event_get_user_data(e);
  lv_event_code_t code = lv_event_get_code(e);
  if ((NULL == ctx) || (NULL == ctx->keyboard))
  {
    return;
  }
  if ((LV_EVENT_READY == code) || (LV_EVENT_CANCEL == code))
  {
    dashboard_hide_keyboard(ctx);
  }
}

static void dashboard_connect_cb(lv_event_t *e)
{
  wifi_dashboard_ctx_t *ctx = (wifi_dashboard_ctx_t *)lv_event_get_user_data(e);
  const char *ssid;
  const char *pass;
  if ((NULL == ctx) || (NULL == ctx->ssid_ta) || (NULL == ctx->pass_ta))
  {
    return;
  }
  ssid = lv_textarea_get_text(ctx->ssid_ta);
  pass = lv_textarea_get_text(ctx->pass_ta);
  cm55_trigger_connect(ssid, pass, 0U);
  cm55_trigger_status_request();
}

static void dashboard_disconnect_cb(lv_event_t *e)
{
  (void)e;
  cm55_trigger_disconnect();
  cm55_trigger_status_request();
}

static void dashboard_scan_cb(lv_event_t *e)
{
  wifi_dashboard_ctx_t *ctx = (wifi_dashboard_ctx_t *)lv_event_get_user_data(e);
  if (NULL != ctx)
  {
    ctx->store.scan_requested = true;
    ctx->store.scan_dirty = true;
  }
  cm55_trigger_scan_all();
  cm55_trigger_status_request();
}

static void dashboard_status_cb(lv_event_t *e)
{
  (void)e;
  cm55_trigger_status_request();
}

static lv_obj_t *dashboard_action_button(lv_obj_t *parent, const char *text, lv_event_cb_t cb, void *user_data, bool primary)
{
  lv_obj_t *btn = lv_btn_create(parent);
  lv_obj_t *lbl = lv_label_create(btn);
  lv_obj_set_height(btn, UI_BTN_HEIGHT);
  lv_obj_set_flex_grow(btn, 1);
  if (primary)
  {
    ui_theme_apply_button_primary(btn);
  }
  else
  {
    ui_theme_apply_button_secondary(btn);
  }
  lv_label_set_text(lbl, text);
  lv_obj_center(lbl);
  lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, user_data);
  return btn;
}

static void dashboard_render_status(wifi_dashboard_ctx_t *ctx)
{
  char badge[96];
  if (NULL == ctx)
  {
    return;
  }
  if (NULL != ctx->status_state_val)
  {
    lv_label_set_text(ctx->status_state_val, wifi_state_to_text(ctx->store.status.state));
  }
  if (NULL != ctx->status_reason_val)
  {
    lv_label_set_text(ctx->status_reason_val, wifi_reason_to_text(ctx->store.status.reason));
  }
  if (NULL != ctx->status_rssi_val)
  {
    lv_label_set_text_fmt(ctx->status_rssi_val, "%d dBm", (int)ctx->store.status.rssi);
    lv_obj_set_style_text_color(ctx->status_rssi_val, wifi_rssi_color(ctx->store.status.rssi), 0);
  }
  if (NULL != ctx->status_ssid_val)
  {
    lv_label_set_text(ctx->status_ssid_val, (ctx->store.status.ssid[0] != '\0') ? ctx->store.status.ssid : "-");
  }
  if (NULL != ctx->status_rssi_chip)
  {
    lv_label_set_text_fmt(ctx->status_rssi_chip, "RSSI %d dBm", (int)ctx->store.status.rssi);
    lv_obj_set_style_text_color(ctx->status_rssi_chip, wifi_rssi_color(ctx->store.status.rssi), 0);
  }
  if (NULL != ctx->header_badge)
  {
    (void)snprintf(badge, sizeof(badge), "%s  |  %d dBm", wifi_state_to_text(ctx->store.status.state), (int)ctx->store.status.rssi);
    lv_label_set_text(ctx->header_badge, badge);
    lv_obj_set_style_text_color(ctx->header_badge, wifi_rssi_color(ctx->store.status.rssi), 0);
  }
}

static void dashboard_render_scan(wifi_dashboard_ctx_t *ctx)
{
  uint32_t i;
  if ((NULL == ctx) || (NULL == ctx->scan_list_cont))
  {
    return;
  }
  lv_obj_clean(ctx->scan_list_cont);
  if (0U == ctx->store.scan_count)
  {
    lv_obj_t *empty = lv_label_create(ctx->scan_list_cont);
    if ((uint16_t)IPC_WIFI_REASON_SCAN_BLOCKED_CONNECTED == ctx->store.status.reason)
    {
      lv_label_set_text(empty, "Scan blocked while connected.\nDisconnect then tap Scan.");
    }
    else if ((uint8_t)IPC_WIFI_LINK_SCANNING == ctx->store.status.state)
    {
      lv_label_set_text(empty, "Scanning...");
    }
    else if (ctx->store.scan_requested)
    {
      lv_label_set_text(empty, "No networks found.");
    }
    else
    {
      lv_label_set_text(empty, "Tap Scan to discover networks.");
    }
    lv_obj_set_style_text_color(empty, UI_COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(empty, UI_FONT_BODY, 0);
    return;
  }
  for (i = 0U; i < ctx->store.scan_count; i++)
  {
    lv_obj_t *row = ui_card_container(ctx->scan_list_cont, UI_SPACE_M, UI_RADIUS_M);
    lv_obj_t *line1 = lv_label_create(row);
    lv_obj_t *line2 = lv_label_create(row);
    ui_width_fill(row);
    ui_gap(row, UI_GAP_SMALL);
    ui_flex_flow(row, LV_FLEX_FLOW_COLUMN);
    ui_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    ui_height_content(row);
    ui_theme_apply_card(row);
    lv_obj_set_style_radius(row, UI_RADIUS_M, 0);
    lv_label_set_text_fmt(line1, "%s", (ctx->store.scan_list[i].ssid[0] != '\0') ? ctx->store.scan_list[i].ssid : "(hidden)");
    lv_obj_set_width(line1, LV_PCT(100));
    lv_obj_set_style_text_color(line1, UI_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(line1, UI_FONT_SECTION, 0);
    lv_label_set_text_fmt(line2, "%d dBm  |  Ch %u  |  %s",
                          (int)ctx->store.scan_list[i].rssi,
                          (unsigned int)ctx->store.scan_list[i].channel,
                          ctx->store.scan_list[i].security);
    lv_obj_set_width(line2, LV_PCT(100));
    lv_label_set_long_mode(line2, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(line2, UI_COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(line2, UI_FONT_BODY, 0);
    if ((NULL != ctx->ssid_ta) && (ctx->store.scan_list[i].ssid[0] != '\0'))
    {
      lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_add_event_cb(row, dashboard_status_cb, LV_EVENT_CLICKED, NULL);
    }
  }
}

static void dashboard_render_debug(wifi_dashboard_ctx_t *ctx)
{
  if ((NULL == ctx) || (NULL == ctx->debug_ta))
  {
    return;
  }
  if ('\0' == ctx->store.debug_text[0])
  {
    lv_textarea_set_text(ctx->debug_ta, "[INIT] Waiting for pipe messages...");
  }
  else
  {
    lv_textarea_set_text(ctx->debug_ta, ctx->store.debug_text);
  }
  lv_textarea_set_cursor_pos(ctx->debug_ta, LV_TEXTAREA_CURSOR_LAST);
}

static void dashboard_refresh_cb(lv_timer_t *timer)
{
  wifi_dashboard_ctx_t *ctx = (wifi_dashboard_ctx_t *)lv_timer_get_user_data(timer);
  if (NULL == ctx)
  {
    return;
  }
  dashboard_store_poll(ctx);
  if (ctx->store.status_dirty)
  {
    dashboard_render_status(ctx);
    ctx->store.status_dirty = false;
  }
  if (ctx->store.scan_dirty)
  {
    dashboard_render_scan(ctx);
    ctx->store.scan_dirty = false;
  }
  if (ctx->store.debug_dirty)
  {
    dashboard_render_debug(ctx);
    ctx->store.debug_dirty = false;
  }
}

static void dashboard_refresh_period_cb(lv_event_t *e)
{
  wifi_dashboard_ctx_t *ctx = (wifi_dashboard_ctx_t *)lv_event_get_user_data(e);
  char sel[8];
  uint32_t ms;
  if ((NULL == ctx) || (NULL == ctx->refresh_dd) || (NULL == ctx->refresh_timer))
  {
    return;
  }
  (void)memset(sel, 0, sizeof(sel));
  lv_dropdown_get_selected_str(ctx->refresh_dd, sel, sizeof(sel));
  ms = (uint32_t)strtoul(sel, NULL, 10);
  if (0U == ms)
  {
    ms = WIFI_UI_REFRESH_DEFAULT_MS;
  }
  ctx->refresh_ms = ms;
  lv_timer_set_period(ctx->refresh_timer, (uint32_t)ctx->refresh_ms);
}

static void dashboard_delete_cb(lv_event_t *e)
{
  wifi_dashboard_ctx_t *ctx = (wifi_dashboard_ctx_t *)lv_event_get_user_data(e);
  if (NULL == ctx)
  {
    return;
  }
  if (NULL != ctx->refresh_timer)
  {
    lv_timer_delete(ctx->refresh_timer);
    ctx->refresh_timer = NULL;
  }
}

static void status_screen_build(wifi_dashboard_ctx_t *ctx, lv_obj_t *tab)
{
  lv_obj_t *card;
  lv_obj_t *row;
  if ((NULL == ctx) || (NULL == tab))
  {
    return;
  }
  ui_pad_all(tab, UI_PAD_SCREEN);
  ui_gap(tab, UI_GAP_NORMAL);
  ui_flex_flow(tab, LV_FLEX_FLOW_COLUMN);
  ui_flex_align(tab, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

  card = dashboard_card(tab, "StatusScreen", LV_SYMBOL_WIFI);
  dashboard_key_value(card, "State", &ctx->status_state_val);
  dashboard_key_value(card, "Reason", &ctx->status_reason_val);
  dashboard_key_value(card, "RSSI", &ctx->status_rssi_val);
  dashboard_key_value(card, "SSID", &ctx->status_ssid_val);

  row = ui_clean_row(card, UI_GAP_NORMAL, 0);
  ui_width_fill(row);
  ctx->status_rssi_chip = lv_label_create(row);
  lv_label_set_text(ctx->status_rssi_chip, "RSSI -127 dBm");
  lv_obj_set_style_text_font(ctx->status_rssi_chip, UI_FONT_SECTION, 0);

  row = ui_clean_row(card, UI_GAP_NORMAL, 0);
  ui_width_fill(row);
  dashboard_action_button(row, "Connect", dashboard_connect_cb, ctx, true);
  dashboard_action_button(row, "Disconnect", dashboard_disconnect_cb, ctx, false);
  row = ui_clean_row(card, UI_GAP_NORMAL, 0);
  ui_width_fill(row);
  dashboard_action_button(row, "Scan", dashboard_scan_cb, ctx, false);
  dashboard_action_button(row, "Status", dashboard_status_cb, ctx, false);
}

static void network_screen_build(wifi_dashboard_ctx_t *ctx, lv_obj_t *tab)
{
  lv_obj_t *card;
  lv_obj_t *row;
  lv_obj_t *lbl;
  if ((NULL == ctx) || (NULL == tab))
  {
    return;
  }
  ui_pad_all(tab, UI_PAD_SCREEN);
  ui_gap(tab, UI_GAP_NORMAL);
  ui_flex_flow(tab, LV_FLEX_FLOW_COLUMN);
  ui_flex_align(tab, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

  card = dashboard_card(tab, "NetworkScreen", LV_SYMBOL_EDIT);
  lbl = lv_label_create(card);
  lv_label_set_text(lbl, "SSID");
  lv_obj_set_style_text_color(lbl, UI_COLOR_TEXT_MUTED, 0);
  ctx->ssid_ta = lv_textarea_create(card);
  ui_width_fill(ctx->ssid_ta);
  lv_textarea_set_one_line(ctx->ssid_ta, true);
  lv_textarea_set_text(ctx->ssid_ta, "TERNION");
  ui_theme_apply_textarea(ctx->ssid_ta);
  lv_obj_add_event_cb(ctx->ssid_ta, dashboard_ta_event_cb, LV_EVENT_ALL, ctx);

  lbl = lv_label_create(card);
  lv_label_set_text(lbl, "Password");
  lv_obj_set_style_text_color(lbl, UI_COLOR_TEXT_MUTED, 0);
  ctx->pass_ta = lv_textarea_create(card);
  ui_width_fill(ctx->pass_ta);
  lv_textarea_set_one_line(ctx->pass_ta, true);
  lv_textarea_set_password_mode(ctx->pass_ta, true);
  lv_textarea_set_text(ctx->pass_ta, "111122134");
  ui_theme_apply_textarea(ctx->pass_ta);
  lv_obj_add_event_cb(ctx->pass_ta, dashboard_ta_event_cb, LV_EVENT_ALL, ctx);

  row = ui_clean_row(card, UI_GAP_NORMAL, 0);
  ui_width_fill(row);
  dashboard_action_button(row, "Connect", dashboard_connect_cb, ctx, true);
  dashboard_action_button(row, "Scan", dashboard_scan_cb, ctx, false);

  card = dashboard_card(tab, "Networks", LV_SYMBOL_LIST);
  ctx->scan_list_cont = ui_clean_scroll_container(card, LV_DIR_VER);
  ui_width_fill(ctx->scan_list_cont);
  ui_height(ctx->scan_list_cont, 220);
  ui_gap(ctx->scan_list_cont, UI_GAP_SMALL);
  ui_flex_flow(ctx->scan_list_cont, LV_FLEX_FLOW_COLUMN);
  ui_flex_align(ctx->scan_list_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_scrollbar_mode(ctx->scan_list_cont, LV_SCROLLBAR_MODE_AUTO);
}

static void logs_screen_build(wifi_dashboard_ctx_t *ctx, lv_obj_t *tab)
{
  lv_obj_t *card;
  lv_obj_t *row;
  if ((NULL == ctx) || (NULL == tab))
  {
    return;
  }
  ui_pad_all(tab, UI_PAD_SCREEN);
  ui_gap(tab, UI_GAP_NORMAL);
  ui_flex_flow(tab, LV_FLEX_FLOW_COLUMN);
  ui_flex_align(tab, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

  card = dashboard_card(tab, "LogsScreen", LV_SYMBOL_LIST);
  ctx->debug_ta = lv_textarea_create(card);
  ui_width_fill(ctx->debug_ta);
  ui_height(ctx->debug_ta, 300);
  lv_textarea_set_one_line(ctx->debug_ta, false);
  lv_textarea_set_cursor_click_pos(ctx->debug_ta, false);
  lv_textarea_set_text(ctx->debug_ta, "[INIT] Waiting for pipe messages...");
  ui_theme_apply_log_textarea(ctx->debug_ta);
  lv_obj_set_scrollbar_mode(ctx->debug_ta, LV_SCROLLBAR_MODE_AUTO);

  row = ui_clean_row(card, UI_GAP_NORMAL, 0);
  ui_width_fill(row);
  dashboard_action_button(row, "Status", dashboard_status_cb, ctx, false);
  dashboard_action_button(row, "Scan", dashboard_scan_cb, ctx, false);
}

static void settings_screen_build(wifi_dashboard_ctx_t *ctx, lv_obj_t *tab)
{
  lv_obj_t *card;
  lv_obj_t *lbl;
  if ((NULL == ctx) || (NULL == tab))
  {
    return;
  }
  ui_pad_all(tab, UI_PAD_SCREEN);
  ui_gap(tab, UI_GAP_NORMAL);
  ui_flex_flow(tab, LV_FLEX_FLOW_COLUMN);
  ui_flex_align(tab, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

  card = dashboard_card(tab, "SettingsScreen", LV_SYMBOL_SETTINGS);
  lbl = lv_label_create(card);
  lv_label_set_text(lbl, "Refresh period (ms)");
  lv_obj_set_style_text_color(lbl, UI_COLOR_TEXT_MUTED, 0);
  lv_obj_set_style_text_font(lbl, UI_FONT_BODY, 0);

  ctx->refresh_dd = lv_dropdown_create(card);
  ui_width(ctx->refresh_dd, 160);
  lv_dropdown_set_options(ctx->refresh_dd, "300\n500\n700\n1000");
  lv_dropdown_set_selected(ctx->refresh_dd, 1U);
  lv_obj_set_style_text_font(ctx->refresh_dd, UI_FONT_BODY, 0);
  lv_obj_add_event_cb(ctx->refresh_dd, dashboard_refresh_period_cb, LV_EVENT_VALUE_CHANGED, ctx);
}

void wifi_operator_dashboard_create(lv_obj_t *screen)
{
  lv_display_t *disp;
  lv_coord_t disp_w;
  lv_coord_t disp_h;
  lv_coord_t content_h;
  lv_obj_t *header;
  lv_obj_t *header_title;
  lv_obj_t *content;
  lv_obj_t *tv;
  lv_obj_t *tab_bar;
  lv_obj_t *tab_status;
  lv_obj_t *tab_networks;
  lv_obj_t *tab_logs;
  lv_obj_t *tab_settings;

  if (NULL == screen)
  {
    return;
  }

  (void)memset(&s_ctx, 0, sizeof(s_ctx));
  s_ctx.refresh_ms = WIFI_UI_REFRESH_DEFAULT_MS;
  dashboard_store_init(&s_ctx);

  disp = lv_display_get_default();
  if (NULL != disp)
  {
    disp_w = lv_display_get_horizontal_resolution(disp);
    disp_h = lv_display_get_vertical_resolution(disp);
  }
  else
  {
    disp_w = 800;
    disp_h = 480;
  }
  if (disp_h <= 56)
  {
    content_h = 200;
  }
  else
  {
    content_h = disp_h - 56;
  }

  lv_obj_clean(screen);
  lv_obj_set_size(screen, disp_w, disp_h);
  ui_theme_apply_screen(screen);

  header = ui_clean_row(screen, UI_GAP_NORMAL, UI_SPACE_L);
  ui_width_fill(header);
  ui_height(header, 56);
  lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_bg_color(header, UI_COLOR_BG, 0);
  lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);

  header_title = lv_label_create(header);
  lv_label_set_text(header_title, "WiFi Operator Dashboard");
  ui_theme_apply_header_title(header_title);
  lv_obj_set_style_text_font(header_title, UI_FONT_SECTION, 0);

  s_ctx.header_badge = lv_label_create(header);
  lv_label_set_text(s_ctx.header_badge, "DISCONNECTED  |  -127 dBm");
  lv_obj_set_style_text_font(s_ctx.header_badge, UI_FONT_BODY, 0);
  lv_obj_set_style_text_color(s_ctx.header_badge, UI_COLOR_TEXT_MUTED, 0);

  content = lv_obj_create(screen);
  lv_obj_remove_style_all(content);
  lv_obj_set_size(content, disp_w, content_h);
  lv_obj_align(content, LV_ALIGN_TOP_LEFT, 0, 56);

  tv = lv_tabview_create(content);
  lv_obj_set_size(tv, disp_w, content_h);
  lv_tabview_set_tab_bar_position(tv, LV_DIR_TOP);
  lv_tabview_set_tab_bar_size(tv, 48);
  lv_obj_set_style_bg_color(tv, UI_COLOR_BG, 0);
  lv_obj_set_style_bg_opa(tv, LV_OPA_COVER, 0);

  tab_bar = lv_tabview_get_tab_bar(tv);
  if (NULL != tab_bar)
  {
    lv_obj_set_style_bg_color(tab_bar, UI_COLOR_SURFACE_ALT, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tab_bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_text_color(tab_bar, UI_COLOR_TEXT, LV_PART_ITEMS);
    lv_obj_set_style_text_font(tab_bar, UI_FONT_BODY, LV_PART_ITEMS);
    lv_obj_set_style_bg_color(tab_bar, UI_COLOR_SURFACE_ALT, LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(tab_bar, LV_OPA_COVER, LV_PART_ITEMS);
    lv_obj_set_style_border_width(tab_bar, 0, LV_PART_ITEMS);
    lv_obj_set_style_text_color(tab_bar, UI_COLOR_PRIMARY, LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(tab_bar, UI_COLOR_BG, LV_PART_ITEMS | LV_STATE_CHECKED);
  }

  tab_status = lv_tabview_add_tab(tv, "Status");
  tab_networks = lv_tabview_add_tab(tv, "Network");
  tab_logs = lv_tabview_add_tab(tv, "Logs");
  tab_settings = lv_tabview_add_tab(tv, "Settings");

  status_screen_build(&s_ctx, tab_status);
  network_screen_build(&s_ctx, tab_networks);
  logs_screen_build(&s_ctx, tab_logs);
  settings_screen_build(&s_ctx, tab_settings);

  s_ctx.keyboard = keyboard_create(screen, 0, 0, 0, 0);
  if (NULL != s_ctx.keyboard)
  {
    keyboard_apply_dark_theme(s_ctx.keyboard);
    ui_hide(s_ctx.keyboard);
    lv_obj_add_event_cb(s_ctx.keyboard, dashboard_keyboard_event_cb, LV_EVENT_ALL, &s_ctx);
  }

  s_ctx.refresh_timer = lv_timer_create(dashboard_refresh_cb, s_ctx.refresh_ms, &s_ctx);
  lv_obj_add_event_cb(screen, dashboard_delete_cb, LV_EVENT_DELETE, &s_ctx);

  cm55_trigger_status_request();
  dashboard_refresh_cb(s_ctx.refresh_timer);
}
