/*******************************************************************************
 * File Name        : examples.c
 *
 * Description      : Implementation of widget examples
 *
 *******************************************************************************/

#include "examples.h"
#include "cm55_ipc_app.h"
#include "dropdown.h"
#include "keyboard.h"
#include "lvgl.h"
#include "screen.h"
#include "tesa_datetime.h"
#include "tesa_logging.h"
#include "textarea.h"
#include "ui_layout.h"
#include "wifi_operator_dashboard.h"

extern const lv_image_dsc_t ui_img_debugging_64x64_png;
extern const lv_image_dsc_t ui_img_healthcare_64x64_png;

static void update_label_text(lv_timer_t *timer)
{
  static uint32_t counter = 0;
  lv_obj_t *label = (lv_obj_t *)lv_timer_get_user_data(timer);
  counter++;
  lv_label_set_text_fmt(label, "Counter: %u", (unsigned int)counter);
}

static void update_datetime_label(lv_timer_t *timer)
{
  lv_obj_t *label = (lv_obj_t *)lv_timer_get_user_data(timer);
  char buffer[32];
  if (tesa_get_current_datetime(DATETIME_FORMAT_FULL, buffer, sizeof(buffer)) != NULL)
  {
    lv_label_set_text(label, buffer);
  }
}

void lv_example_style_1(void)
{
  static lv_style_t style;
  lv_style_init(&style);
  lv_style_set_radius(&style, 5);

  /* Set the width and height  */
  lv_style_set_width(&style, 350);
  lv_style_set_height(&style, LV_SIZE_CONTENT);

  /* Set the paddings */
  lv_style_set_pad_ver(&style, 20);
  lv_style_set_pad_left(&style, 50);

  /* Set the x and y positions */
  lv_style_set_x(&style, lv_pct(50));
  lv_style_set_y(&style, 80);

  /* Create base LVGL object with the new style */
  lv_obj_t *obj = lv_obj_create(lv_screen_active());
  lv_obj_add_style(obj, &style, 0);

  /* Create a label and add it to the object */
  lv_obj_t *label = lv_label_create(obj);
  lv_label_set_text(label, "Hello World!");

  /* Create a timer to update the label text every 1 second */
  lv_timer_create(update_label_text, 20, label);
}

static void update_chart(lv_timer_t *timer)
{
  static float smoothed_ser1 = 0.0f;
  static float smoothed_ser2 = 0.0f;
  static bool initialized = false;

  const float alpha = 0.3f;

  lv_obj_t *chart = (lv_obj_t *)lv_timer_get_user_data(timer);
  lv_chart_series_t *ser1 = lv_chart_get_series_next(chart, NULL);
  lv_chart_series_t *ser2 = lv_chart_get_series_next(chart, ser1);

  if (ser1 != NULL)
  {
    int32_t new_value = (int32_t)lv_rand(10, 50);
    if (!initialized)
    {
      smoothed_ser1 = (float)new_value;
    }
    else
    {
      smoothed_ser1 = alpha * (float)new_value + (1.0f - alpha) * smoothed_ser1;
    }
    lv_chart_set_next_value(chart, ser1, (int32_t)smoothed_ser1);
  }

  if (ser2 != NULL)
  {
    int32_t new_value = (int32_t)lv_rand(50, 90);
    if (!initialized)
    {
      smoothed_ser2 = (float)new_value;
    }
    else
    {
      smoothed_ser2 = alpha * (float)new_value + (1.0f - alpha) * smoothed_ser2;
    }
    lv_chart_set_next_value(chart, ser2, (int32_t)smoothed_ser2);
  }

  initialized = true;
  lv_chart_refresh(chart);
}

void lv_example_chart_1(void)
{
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), 0);
  /*Create a chart*/
  lv_obj_t *chart;
  chart = lv_chart_create(lv_screen_active());
  lv_obj_set_size(chart, LV_PCT(50), LV_PCT(50));
  lv_obj_center(chart);
  lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
  lv_chart_set_point_count(chart, 50);
  lv_obj_set_style_bg_color(chart, lv_color_black(), 0);

  /*Add two data series*/
  lv_chart_series_t *ser1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_PRIMARY_Y);
  lv_chart_series_t *ser2 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_SECONDARY_Y);
  int32_t *ser1_y_points = lv_chart_get_y_array(chart, ser1);
  int32_t *ser2_y_points = lv_chart_get_y_array(chart, ser2);

  const float alpha = 0.3f;
  float smoothed_ser1 = 0.0f;
  float smoothed_ser2 = 0.0f;

  uint32_t i;
  for (i = 0; i < 10; i++)
  {
    int32_t new_val1 = (int32_t)lv_rand(10, 50);
    int32_t new_val2 = (int32_t)lv_rand(50, 90);

    if (i == 0)
    {
      smoothed_ser1 = (float)new_val1;
      smoothed_ser2 = (float)new_val2;
    }
    else
    {
      smoothed_ser1 = alpha * (float)new_val1 + (1.0f - alpha) * smoothed_ser1;
      smoothed_ser2 = alpha * (float)new_val2 + (1.0f - alpha) * smoothed_ser2;
    }

    ser1_y_points[i] = (int32_t)smoothed_ser1;
    ser2_y_points[i] = (int32_t)smoothed_ser2;
  }

  lv_obj_set_style_shadow_opa(chart, 255, LV_PART_ITEMS);
  lv_obj_set_style_shadow_offset_x(chart, 0, LV_PART_ITEMS);
  lv_obj_set_style_shadow_width(chart, 20, LV_PART_ITEMS);
  /* Drop shadow color follows each series color automatically,
   * so no manual override is needed. */
  lv_chart_refresh(chart); /*Required after direct set*/

  lv_obj_t *legend_fps = lv_label_create(lv_screen_active());
  lv_label_set_text(legend_fps, "FPS");
  lv_obj_set_style_text_color(legend_fps, lv_palette_main(LV_PALETTE_GREEN), 0);
  lv_obj_align_to(legend_fps, chart, LV_ALIGN_OUT_TOP_LEFT, 0, -10);

  lv_obj_t *legend_cpu = lv_label_create(lv_screen_active());
  lv_label_set_text(legend_cpu, "CPU Usage");
  lv_obj_set_style_text_color(legend_cpu, lv_palette_main(LV_PALETTE_RED), 0);
  lv_obj_align_to(legend_cpu, chart, LV_ALIGN_OUT_TOP_LEFT, 80, -10);

  lv_timer_create(update_chart, 100, chart);
}

void lv_example_tabview_2(void)
{
  /*Create a Tab view object*/
  lv_obj_t *tabview;
  uint32_t tab_count = 0;
  uint32_t i = 0;

  tabview = lv_tabview_create(lv_screen_active());
  lv_tabview_set_tab_bar_position(tabview, LV_DIR_LEFT);
  lv_tabview_set_tab_bar_size(tabview, 80);

  lv_obj_set_style_bg_color(tabview, lv_color_black(), 0);

  lv_obj_t *tab_buttons = lv_tabview_get_tab_bar(tabview);
  lv_obj_set_style_bg_color(tab_buttons, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
  lv_obj_set_style_text_color(tab_buttons, lv_color_white(), 0);

  /*Add 5 tabs (the tabs are page (lv_page) and can be scrolled*/
  lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Tab 1");
  lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Tab 2");
  lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Tab 3");
  lv_obj_t *tab4 = lv_tabview_add_tab(tabview, "Tab 4");
  lv_obj_t *tab5 = lv_tabview_add_tab(tabview, "Tab 5");

  tab_count = lv_tabview_get_tab_count(tabview);
  for (i = 0; i < tab_count; i++)
  {
    lv_obj_t *button = lv_obj_get_child(tab_buttons, i);
    lv_obj_set_style_border_side(button, LV_BORDER_SIDE_RIGHT, LV_PART_MAIN | LV_STATE_CHECKED);
  }
  lv_obj_set_style_bg_color(tab1, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(tab1, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(tab2, lv_palette_darken(LV_PALETTE_AMBER, 3), 0);
  lv_obj_set_style_bg_opa(tab2, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(tab3, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(tab3, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(tab4, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(tab4, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(tab5, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(tab5, LV_OPA_COVER, 0);

  /*Add content to the tabs*/
  lv_obj_t *label = lv_label_create(tab1);
  lv_label_set_text(label, "First tab");
  lv_obj_set_style_text_color(label, lv_color_white(), 0);

  label = lv_label_create(tab2);
  lv_label_set_text(label, "Second tab");
  lv_obj_set_style_text_color(label, lv_color_white(), 0);

  label = lv_label_create(tab3);
  lv_label_set_text(label, "Third tab");
  lv_obj_set_style_text_color(label, lv_color_white(), 0);

  label = lv_label_create(tab4);
  lv_label_set_text(label, "Fourth tab");
  lv_obj_set_style_text_color(label, lv_color_white(), 0);

  label = lv_label_create(tab5);
  lv_label_set_text(label, "Fifth tab");
  lv_obj_set_style_text_color(label, lv_color_white(), 0);

  lv_obj_remove_flag(lv_tabview_get_content(tabview), LV_OBJ_FLAG_SCROLLABLE);
}

static void example_3(void)
{
  /* Make screen black */
  // lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), 0);

  // lv_example_tabview_2();
  // lv_example_style_1();
  lv_example_chart_1();
}

static void example_1(void)
{

  lv_obj_t *scr = lv_obj_create(NULL);
  lv_obj_set_size(scr, ACTUAL_DISP_HOR_RES, ACTUAL_DISP_VER_RES);

  lv_scr_load(scr);
  // lv_obj_center(scr);

  // 1. create a flex container
  lv_obj_t *flex_container = lv_obj_create(scr);
  // 1.1 set the flex flow to column
  lv_obj_set_flex_flow(flex_container, LV_FLEX_FLOW_COLUMN);
  // 1.2 set the flex align to center
  lv_obj_set_flex_align(flex_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  // 1.3 set the width and height to content
  // lv_obj_set_width(flex_container, LV_SIZE_CONTENT);
  // lv_obj_set_height(flex_container, LV_SIZE_CONTENT);

  // 1.4 set the width and height to 100%. PCT stands for `percentage-based
  // size`.
  lv_obj_set_size(flex_container, LV_PCT(100), LV_PCT(100));
  lv_obj_center(flex_container);

  // 1.5 set the background color to black
  lv_obj_set_style_bg_color(flex_container, lv_color_make(0xFF, 0x00, 0x00), 0);
  // 1.6 set the background opacity to 50
  lv_obj_set_style_bg_opa(flex_container, LV_OPA_100, 0);

  // 1.7 set border color to lime
  lv_obj_set_style_border_color(flex_container, lv_color_make(0x00, 0xFF, 0x00), 0);
  // 1.8 set border radius to 20
  lv_obj_set_style_radius(flex_container, 20, 0);

  // 1.8 set border width to 2
  lv_obj_set_style_border_width(flex_container, 2, 0);

  // 1.9 set margin to 20
  lv_obj_set_style_margin_all(flex_container, 100, 0);

  // 2.Create a label and add it to the flex container
  lv_obj_t *label = lv_label_create(flex_container);
  lv_label_set_text(label, "Hello, World!");
  lv_obj_set_width(label, 100);
  lv_obj_set_height(label, 50);
  lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(label, lv_color_white(), 0);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
}

static void example_2(void)
{
  /* Create a dark screen */
  lv_obj_t *scr = screen_create_dark();

  /* Create centered flex container */
  lv_obj_t *container = screen_create_centered_container(scr);

  /* Create dropdown with wireless protocol options (centered in container) */
  lv_obj_t *dd = dropdown_create(container, "Wi-Fi\nBluetooth\nZigbee\nThread\nZ-Wave\nLoRa\nSigfox", 0, 0, 0,
                                 DROPDOWN_DEFAULT_WIDTH, DROPDOWN_DEFAULT_HEIGHT);
  if (dd != NULL)
  {
    /* Apply dark theme styling to dropdown */
    dropdown_apply_dark_theme(dd);
  }

  /* Create textarea above the keyboard */
  lv_obj_t *ta = textarea_create(container, 0, 0, DROPDOWN_DEFAULT_WIDTH, 52, 128, false);
  if (ta != NULL)
  {
    /* Apply dark theme styling to textarea */
    textarea_apply_dark_theme(ta);
  }

  /* Create keyboard below the textarea (1.5x larger) */
  lv_obj_t *kb = keyboard_create(container, 0, 0, (DROPDOWN_DEFAULT_WIDTH * 3) / 2, 347);
  if (kb != NULL)
  {
    /* Apply dark theme styling to keyboard */
    keyboard_apply_dark_theme(kb);
    /* Link textarea to keyboard for automatic input handling */
    if (ta != NULL)
    {
      keyboard_set_textarea(kb, ta);
    }
  }
}

static void example_4(void)
{
  // DO NOTHING
}

static void make_log_row(lv_obj_t *parent, const char *date, const char *time, uint32_t current_rand)
{
  /* Create a container for the custom row */
  lv_obj_t *row = lv_obj_create(parent);
  lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);

  /* Use flexbox for layout (row direction) */
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  /* Remove default styling for clean results */
  lv_obj_set_style_pad_all(row, 2, 0);
  lv_obj_set_style_pad_gap(row, 8, 0);
  lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(row, 0, 0);
  lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);

  /* 1. Date Label - Subtle Grey */
  lv_obj_t *date_label = lv_label_create(row);
  lv_label_set_text(date_label, date);
  lv_obj_set_style_text_color(date_label, lv_palette_main(LV_PALETTE_GREY), 0);
  lv_obj_set_style_text_font(date_label, &lv_font_montserrat_14, 0);

  /* 2. Time Label - White & Prominent */
  lv_obj_t *time_label = lv_label_create(row);
  lv_label_set_text(time_label, time);
  lv_obj_set_style_text_color(time_label, lv_color_white(), 0);
  lv_obj_set_style_text_font(time_label, &lv_font_montserrat_14, 0);

  /* 3. Random Number - Blue Accent with background badge */
  lv_obj_t *rand_label = lv_label_create(row);
  lv_label_set_text_fmt(rand_label, "#%02u", (unsigned int)current_rand);
  lv_obj_set_style_text_color(rand_label, lv_palette_main(LV_PALETTE_BLUE), 0);
  lv_obj_set_style_text_font(rand_label, &lv_font_montserrat_14, 0);
  lv_obj_set_style_bg_color(rand_label, lv_palette_lighten(LV_PALETTE_BLUE, 5), 0);
  lv_obj_set_style_bg_opa(rand_label, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(rand_label, 4, 0);
  lv_obj_set_style_pad_hor(rand_label, 4, 0);
}

static void add_datetime_log_to_card(lv_obj_t *card)
{
  char date_buf[16];
  char time_buf[16];

  if (tesa_get_current_datetime(DATETIME_FORMAT_DATE, date_buf, sizeof(date_buf)) != NULL &&
      tesa_get_current_datetime(DATETIME_FORMAT_TIME, time_buf, sizeof(time_buf)) != NULL)
  {

    uint32_t r = (uint32_t)lv_rand(0, 99);
    make_log_row(card, date_buf, time_buf, r);

    TESA_LOG_INFO("UI", "Added log row: %s %s-#%02u", date_buf, time_buf, r);

    /* If the number of children (rows) is more than 13, the oldest one will be
     * removed.
     */
    if (lv_obj_get_child_count(card) > 15)
    {
      lv_obj_del(lv_obj_get_child(card, 0));
    }
  }
  else
  {
    TESA_LOG_ERROR("UI", "Failed to get current datetime for log row");
  }
}

static void datetime_log_timer_cb(lv_timer_t *timer)
{
  lv_obj_t *card = (lv_obj_t *)lv_timer_get_user_data(timer);
  add_datetime_log_to_card(card);
}

static void example_5(void)
{
  /* 1) Create a main container, flex col, red background */
  lv_obj_t *main_cont = lv_obj_create(lv_screen_active());
  lv_obj_set_size(main_cont, LV_PCT(100), LV_PCT(100));
  lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_bg_color(main_cont, lv_palette_main(LV_PALETTE_RED), 0);
  lv_obj_set_style_border_width(main_cont, 0, 0);
  lv_obj_set_style_radius(main_cont, 0, 0);
  lv_obj_set_style_pad_all(main_cont, 0, 0);

  /* 2) Create a header container inside main container, flex row, blue
   * background */
  lv_obj_t *header = lv_obj_create(main_cont);
  lv_obj_set_size(header, LV_PCT(100), 50);
  lv_obj_set_style_bg_color(header, lv_palette_darken(LV_PALETTE_BLUE, 4), 0);
  lv_obj_set_style_border_width(header, 0, 0);
  lv_obj_set_style_radius(header, 0, 0);
  lv_obj_remove_flag(header, LV_OBJ_FLAG_SCROLLABLE);

  /* Add date/time label to header, aligned to the right, white color */
  lv_obj_t *dt_label = lv_label_create(header);
  lv_obj_align(dt_label, LV_ALIGN_RIGHT_MID, -10, 0);
  lv_obj_set_style_text_color(dt_label, lv_color_white(), 0);
  lv_label_set_text(dt_label, "Loading...");

  /* Create a timer to update the date/time label every 1 second */
  lv_timer_create(update_datetime_label, 1000, dt_label);

  /* 3) Create a content container inside main container, flex row, green
   * background */
  lv_obj_t *content = lv_obj_create(main_cont);
  lv_obj_set_size(content, LV_PCT(100), 0); /* Width 100%, height set by flex grow */
  lv_obj_set_flex_grow(content, 1);
  lv_obj_set_style_bg_color(content, lv_palette_main(LV_PALETTE_GREEN), 0);
  lv_obj_set_style_border_width(content, 0, 0);
  lv_obj_set_style_radius(content, 0, 0);

  /* 3.1) Add two cards/containers with rounded corners inside the content
   * container. */
  lv_obj_set_flex_flow(content, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(content, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(content, 20, 0);
  lv_obj_set_style_pad_all(content, 10, 0);

  /* The first card take the rest of the width and 100% of the height. */
  lv_obj_t *card1 = lv_obj_create(content);
  lv_obj_set_size(card1, 0, LV_PCT(100));
  lv_obj_set_flex_grow(card1, 1);
  lv_obj_set_flex_flow(card1, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(card1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_gap(card1, 5, 0);
  lv_obj_set_style_radius(card1, 20, 0);
  lv_obj_set_style_bg_color(card1, lv_color_white(), 0);
  lv_obj_set_style_border_width(card1, 0, 0);

  lv_obj_t *l1 = lv_label_create(card1);
  lv_label_set_text(l1, "Main Card Content");
  lv_obj_set_style_text_color(l1, lv_color_black(), 0);
  lv_obj_set_style_text_font(l1, &lv_font_montserrat_16, 0);

  /* Create a dedicated container for logs to keep them between title and button
   */
  lv_obj_t *log_cont = lv_obj_create(card1);
  lv_obj_set_width(log_cont, LV_PCT(100));
  lv_obj_set_flex_grow(log_cont, 1);
  lv_obj_set_flex_flow(log_cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(log_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_all(log_cont, 0, 0);
  lv_obj_set_style_border_width(log_cont, 0, 0);
  lv_obj_set_style_bg_opa(log_cont, LV_OPA_TRANSP, 0);
  lv_obj_remove_flag(log_cont, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *b1 = lv_btn_create(card1);
  lv_obj_t *bl1 = lv_label_create(b1);
  lv_label_set_text(bl1, "Confirm");

  /* Set data time to 2026-01-21 11:06:00 */
  tesa_set_rtc_time(11, 18, 0, 21, 1, 2026);

  /* Start periodic logging to the log container */
  lv_timer_create(datetime_log_timer_cb, 100, log_cont);

  /* The second card take 30% (the rest) and has the same height as the first
   * card. */
  lv_obj_t *card2 = lv_obj_create(content);
  lv_obj_set_size(card2, LV_PCT(30), LV_PCT(100));
  lv_obj_set_style_radius(card2, 20, 0);
  lv_obj_set_style_bg_color(card2, lv_palette_lighten(LV_PALETTE_GREY, 4), 0);
  lv_obj_set_style_border_width(card2, 0, 0);

  lv_obj_t *l2 = lv_label_create(card2);
  lv_label_set_text(l2, "Details");
  lv_obj_set_style_text_color(l2, lv_color_black(), 0);
  lv_obj_align(l2, LV_ALIGN_TOP_MID, 0, 10);

  lv_obj_t *b2 = lv_btn_create(card2);
  lv_obj_set_size(b2, 60, 40);
  lv_obj_align(b2, LV_ALIGN_BOTTOM_MID, 0, -10);
  lv_obj_t *bl2 = lv_label_create(b2);
  lv_label_set_text(bl2, "Info");

  /* 4) Create a footer container inside main container, flex row, yellow
   * background */
  lv_obj_t *footer = lv_obj_create(main_cont);
  lv_obj_set_size(footer, LV_PCT(100), 50);
  lv_obj_set_style_bg_color(footer, lv_palette_main(LV_PALETTE_YELLOW), 0);
  lv_obj_set_style_border_width(footer, 0, 0);
  lv_obj_set_style_radius(footer, 0, 0);
}

typedef struct
{
  lv_obj_t *header_container;  // system status
  lv_obj_t *content_container; // content
  lv_obj_t *footer_container;  // buttons
} main_container_t;

main_container_t main_container = {};

main_container_t *init_main_container(lv_obj_t *parent)
{
  // 1) create wrapper container, flex row, dark teal background
  lv_obj_t *wrapper = lv_obj_create(parent);
  lv_obj_set_size(wrapper, LV_PCT(100), LV_PCT(100));
  lv_obj_set_flex_flow(wrapper, LV_FLEX_FLOW_COLUMN);

  // Simple Solid Deep Teal Background
  lv_obj_set_style_bg_color(wrapper, lv_color_hex(0x002E20), 0);
  lv_obj_set_style_bg_opa(wrapper, LV_OPA_50, 0);
  lv_obj_set_style_border_width(wrapper, 0, 0);
  lv_obj_set_style_radius(wrapper, 0, 0);
  lv_obj_set_style_pad_all(wrapper, 0, 0);
  lv_obj_set_style_pad_gap(wrapper, 0, 0);

  // 2) create header container, flex row, semi-transparent teal
  main_container.header_container = lv_obj_create(wrapper);
  lv_obj_set_size(main_container.header_container, LV_PCT(100), 50);
  lv_obj_set_flex_flow(main_container.header_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(main_container.header_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_bg_color(main_container.header_container, lv_palette_darken(LV_PALETTE_BLUE, 3), 0);
  lv_obj_set_style_bg_opa(main_container.header_container, LV_OPA_50, 0);
  lv_obj_set_style_border_width(main_container.header_container, 0, 0);
  lv_obj_set_style_radius(main_container.header_container, 0, 0);
  lv_obj_remove_flag(main_container.header_container, LV_OBJ_FLAG_SCROLLABLE);

  // 3) create content container, flex column, transparent
  main_container.content_container = lv_obj_create(wrapper);
  lv_obj_set_width(main_container.content_container, LV_PCT(100));
  lv_obj_set_flex_grow(main_container.content_container, 1);
  lv_obj_set_flex_flow(main_container.content_container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_bg_opa(main_container.content_container, LV_OPA_50, 0);
  lv_obj_set_style_border_width(main_container.content_container, 0, 0);
  lv_obj_set_style_radius(main_container.content_container, 0, 0);
  lv_obj_set_style_pad_all(main_container.content_container, 0, 0);

  // 4) create footer container, flex row, semi-transparent teal
  main_container.footer_container = lv_obj_create(wrapper);
  lv_obj_set_size(main_container.footer_container, LV_PCT(100), 80);
  lv_obj_set_flex_flow(main_container.footer_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(main_container.footer_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_bg_color(main_container.footer_container, lv_color_hex(0x374151), 0);
  lv_obj_set_style_bg_opa(main_container.footer_container, LV_OPA_50, 0);
  lv_obj_set_style_border_width(main_container.footer_container, 0, 0);
  lv_obj_set_style_radius(main_container.footer_container, 0, 0);
  lv_obj_remove_flag(main_container.footer_container, LV_OBJ_FLAG_SCROLLABLE);

  return &main_container;
}

static void update_footer_buttons(lv_obj_t *footer_container, uint32_t active_id)
{
  for (uint32_t i = 0; i < lv_obj_get_child_count(footer_container); i++)
  {
    lv_obj_t *btn = lv_obj_get_child(footer_container, i);
    lv_obj_t *bar = lv_obj_get_child(btn, 0);
    lv_obj_t *icon = lv_obj_get_child(btn, 1);
    lv_obj_t *lbl = lv_obj_get_child(btn, 2);

    uint32_t id = (uintptr_t)lv_obj_get_user_data(btn);

    if (id == active_id)
    {
      lv_obj_remove_flag(bar, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
      lv_obj_set_style_image_recolor_opa(icon, 0, 0); // Original color
    }
    else
    {
      lv_obj_add_flag(bar, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_style_text_color(lbl, lv_color_hex(0x9CA3AF), 0);
      lv_obj_set_style_image_recolor(icon, lv_color_hex(0x9CA3AF), 0);
      lv_obj_set_style_image_recolor_opa(icon, LV_OPA_50,
                                         0); // Grayscale effect
    }
  }
}

static void tab_btn_event_cb(lv_event_t *e)
{
  lv_obj_t *btn = lv_event_get_current_target(e);
  lv_obj_t *tv = (lv_obj_t *)lv_event_get_user_data(e);
  uint32_t id = (uintptr_t)lv_obj_get_user_data(btn);
  lv_tabview_set_active(tv, id, LV_ANIM_OFF);
  update_footer_buttons(lv_obj_get_parent(btn), id);
}

static lv_obj_t *create_custom_footer_button(lv_obj_t *parent, const char *text, uint32_t id, lv_obj_t *tv)
{
  lv_obj_t *btn = lv_obj_create(parent);
  lv_obj_set_size(btn, 120, 80 - 12);
  lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(btn, 1, 0);
  lv_obj_set_style_border_color(btn, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
  lv_obj_set_style_outline_width(btn, 0, 0);
  lv_obj_set_style_pad_all(btn, 0, 0);
  lv_obj_remove_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_user_data(btn, (void *)(uintptr_t)id);
  lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);

  // 1. Top Bar Indicator
  lv_obj_t *bar = lv_obj_create(btn);
  lv_obj_set_size(bar, 80, 6);
  lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_bg_color(bar, lv_color_hex(0x48BB78), 0);
  lv_obj_set_style_radius(bar, 3, 0);
  lv_obj_set_style_border_width(bar, 0, 0);
  lv_obj_remove_flag(bar, LV_OBJ_FLAG_CLICKABLE);

  // 2. Circle (Commented out)
  /*
  lv_obj_t *circle = lv_obj_create(btn);
  lv_obj_set_size(circle, 32, 32);
  lv_obj_align(circle, LV_ALIGN_CENTER, 0, -8);
  lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
  lv_obj_remove_flag(circle, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_remove_flag(circle, LV_OBJ_FLAG_CLICKABLE);
  */

  // 2b. Icon
  lv_obj_t *icon = lv_image_create(btn);
  lv_image_set_src(icon, &ui_img_debugging_64x64_png);
  lv_image_set_scale(icon, 128); // Scale to 0.5x (64 -> 32)
  lv_obj_align(icon, LV_ALIGN_CENTER, 0, -8);
  lv_obj_remove_flag(icon, LV_OBJ_FLAG_CLICKABLE);

  // 3. Label
  lv_obj_t *lbl = lv_label_create(btn);
  lv_label_set_text(lbl, text);
  lv_obj_align(lbl, LV_ALIGN_BOTTOM_MID, 0, -10);
  lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
  lv_obj_remove_flag(lbl, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_add_event_cb(btn, tab_btn_event_cb, LV_EVENT_CLICKED, tv);

  return btn;
}

static void debug_panel(lv_obj_t *parent)
{
  // 1. Create a main container, flex row
  lv_obj_t *main_cont = lv_obj_create(parent);
  lv_obj_set_size(main_cont, LV_PCT(100), LV_PCT(100));
  lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_bg_opa(main_cont, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(main_cont, 0, 0);
  lv_obj_set_style_pad_all(main_cont, 0, 0);
  lv_obj_set_style_pad_gap(main_cont, 0, 0);
  lv_obj_remove_flag(main_cont, LV_OBJ_FLAG_SCROLLABLE);

  // 2. Create a container, flex row, 50% width, full height, very dark-gray
  // background
  lv_obj_t *left_panel = lv_obj_create(main_cont);
  lv_obj_set_size(left_panel, LV_PCT(50), LV_PCT(100));
  lv_obj_set_style_bg_color(left_panel, lv_color_hex(0x111827), 0);
  lv_obj_set_style_bg_opa(left_panel, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(left_panel, 1, 0);
  lv_obj_set_style_border_side(left_panel, LV_BORDER_SIDE_RIGHT, 0);
  lv_obj_set_style_border_color(left_panel, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
  lv_obj_set_style_radius(left_panel, 0, 0);
  lv_obj_set_flex_flow(left_panel, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(left_panel, 10, 0);
  lv_obj_set_style_pad_gap(left_panel, 10, 0);

  // Add initial log row and start timer
  add_datetime_log_to_card(left_panel);
  lv_timer_create(datetime_log_timer_cb, 2000, left_panel);

  // 3. Create a container, flex row, 50% width, full height, very dark-gray
  // background
  lv_obj_t *right_panel = lv_obj_create(main_cont);
  lv_obj_set_size(right_panel, LV_PCT(50), LV_PCT(100));
  lv_obj_set_style_bg_color(right_panel, lv_color_hex(0x0F172A),
                            0); // Slate-900
  lv_obj_set_style_bg_opa(right_panel, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(right_panel, 0, 0);
  lv_obj_set_style_radius(right_panel, 0, 0);
  lv_obj_set_flex_flow(right_panel, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(right_panel, 10, 0);
  lv_obj_set_style_pad_gap(right_panel, 10, 0);
}

static void example_6(void)
{
  /* Initialize the main container layout on the active screen */
  main_container_t *mc = init_main_container(lv_screen_active());

  /* Add a title to the header */
  lv_obj_t *title = lv_label_create(mc->header_container);
  lv_obj_add_flag(title, LV_OBJ_FLAG_FLOATING);
  lv_label_set_text(title, "TESAIoT Healthcare Gateway");
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_align(title, LV_ALIGN_LEFT_MID, 20, 0);

  /* Add tabview to the content container. Hide default tab buttons. */
  lv_obj_t *tv = lv_tabview_create(mc->content_container);
  lv_obj_set_style_bg_opa(tv, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(tv, 0, 0);
  lv_tabview_set_tab_bar_position(tv, LV_DIR_TOP);
  lv_tabview_set_tab_bar_size(tv, 0); /* Hide tab bar */
  lv_obj_set_size(tv, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_bg_opa(tv, LV_OPA_TRANSP, 0);
  lv_obj_set_style_pad_all(tv, 0, 0);
  lv_obj_set_style_border_width(tv, 0, 0);

  /* Add 4 tabs */
  lv_obj_t *tab1 = lv_tabview_add_tab(tv, "Tab 1");
  lv_obj_t *tab2 = lv_tabview_add_tab(tv, "Tab 2");
  lv_obj_t *tab3 = lv_tabview_add_tab(tv, "Tab 3");
  lv_obj_t *tab4 = lv_tabview_add_tab(tv, "Tab 4");
  lv_obj_t *tab5 = lv_tabview_add_tab(tv, "Tab 5");

  /* Style tabs (transparent background) */
  lv_obj_t *tabs[] = {tab1, tab2, tab3, tab4, tab5};
  for (int i = 0; i < 5; i++)
  {
    lv_obj_set_style_bg_opa(tabs[i], LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(tabs[i], 0, 0);
    lv_obj_set_style_border_width(tabs[i], 0, 0);
    lv_obj_t *lbl = lv_label_create(tabs[i]);
    lv_label_set_text_fmt(lbl, "Content of Tab %d", i + 1);
    lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_20, 0);
    lv_obj_center(lbl);
  }

  // Remove children from the tab1, and add icon/image
  // ui_img_healthcare_64x64_png.c to the tab1.
  lv_obj_clean(tabs[0]);
  lv_obj_t *img = lv_image_create(tabs[0]);
  lv_image_set_src(img, &ui_img_healthcare_64x64_png);
  lv_obj_center(img);

  /* Add gentle pulsing scale animation */
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, img);
  lv_anim_set_values(&a, 256, 310); // Scale from 1.0x to ~1.2x
  lv_anim_set_duration(&a, 500);    // 1.2 seconds
  lv_anim_set_playback_duration(&a, 500);
  lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
  lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
  lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_image_set_scale);
  lv_anim_start(&a);

  /* Add buttons to the footer */
  const char btn_labels[][16] = {"Home", "Members", "Devices", "Settings", "Debugging"};
  for (int i = 0; i < 5; i++)
  {
    create_custom_footer_button(mc->footer_container, btn_labels[i], i, tv);
  }

  /* Align buttons in the footer using flex properties */
  lv_obj_set_flex_align(mc->footer_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_gap(mc->footer_container, 10, 0);

  /* Add date time to the header */
  lv_obj_t *dt_label = lv_label_create(mc->header_container);
  lv_obj_add_flag(dt_label, LV_OBJ_FLAG_FLOATING);
  lv_obj_set_style_text_color(dt_label, lv_color_white(), 0);
  lv_obj_set_style_text_font(dt_label, &lv_font_montserrat_14, 0);
  lv_label_set_text(dt_label, "Loading...");
  lv_obj_align(dt_label, LV_ALIGN_RIGHT_MID, -20, 0);

  /* Start a timer to update the date/time every second */
  lv_timer_t *timer = lv_timer_create(update_datetime_label, 1000, dt_label);
  /* Force an immediate update */
  if (timer)
  {
    update_datetime_label(timer);
  }

  /* Switch tabview to Debugging tab */
  lv_tabview_set_active(tv, 4, LV_ANIM_OFF);

  /* Set Debugging as active by default */
  update_footer_buttons(mc->footer_container, 4);
  // Debug tab
  debug_panel(tabs[4]);
}

static void example_7(void)
{
  lv_obj_t *scr = lv_screen_active();
  lv_obj_t *label_default = lv_label_create(scr);
  lv_obj_set_style_text_font(label_default, &lv_font_montserrat_16, 0);
  lv_label_set_text(label_default, "Hello LVGL");
  lv_obj_align(label_default, LV_ALIGN_TOP_MID, 0, 40);

  lv_obj_t *label_thai = lv_label_create(scr);
  lv_obj_set_style_text_font(label_thai, &my_thai_font, 0);
  static const char thai_text[] = "\xE0\xB8\x95\xE0\xB8\xAD\xE0\xB8\x99\xE0\xB8\x99\xE0\xB8\xB5\xE0\xB9\x89"
                                  "\xE0\xB9\x80\xE0\xB8\x9B\xE0\xB8\xA5\xE0\xB8\xB5\xE0\xB8\xA2\xE0\xB8\x99"
                                  "\xE0\xB9\x80\xE0\xB8\x9B\xE0\xB9\x87\xE0\xB8\x99"
                                  "\xE0\xB8\x81\xE0\xB8\xB2\xE0\xB8\xA3"
                                  "\xE0\xB8\x97\xE0\xB8\x94\xE0\xB8\xAA\xE0\xB8\xAD\xE0\xB8\x9A"
                                  " LVGL "
                                  "\xE0\xB8\xA0\xE0\xB8\xB2\xE0\xB8\xA9\xE0\xB8\xB2\xE0\xB9\x84\xE0\xB8\x97"
                                  "\xE0\xB8\xA2";
  lv_label_set_text(label_thai, thai_text);
  lv_obj_align(label_thai, LV_ALIGN_CENTER, 0, 0);
}

#define BUTTON_ROW_GAP (6)
#define BUTTON_ICON_WIDTH (24)

static lv_obj_t *button_card_add_row(lv_obj_t *card, lv_color_t icon_color, const char *symbol, const char *text)
{
  lv_obj_t *row = ui_clean_row(card, BUTTON_ROW_GAP, 0);
  ui_width_fill(row);
  ui_height_content(row);
  lv_obj_t *icon_lbl = lv_label_create(row);
  lv_label_set_text(icon_lbl, symbol != NULL ? symbol : "");
  lv_obj_set_style_text_color(icon_lbl, icon_color, 0);
  lv_obj_set_style_text_font(icon_lbl, &lv_font_montserrat_16, 0);
  ui_width(icon_lbl, BUTTON_ICON_WIDTH);
  lv_obj_t *lbl = lv_label_create(row);
  lv_label_set_text(lbl, text);
  lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
  lv_obj_set_style_text_font(lbl, &lv_font_montserrat_16, 0);
  return lbl;
}

typedef struct
{
  lv_obj_t *state_lbl[2];
  lv_obj_t *count_lbl[2];
} button_ui_refs_t;

static void buttons_timer_cb(lv_timer_t *timer);

static void example_9(void)
{
  wifi_operator_dashboard_create(lv_screen_active());
}

static void buttons_timer_cb(lv_timer_t *timer)
{
  button_ui_refs_t *refs = (button_ui_refs_t *)lv_timer_get_user_data(timer);
  if (refs == NULL)
    return;
  for (uint32_t id = 0U; id < 2U; id++)
  {
    uint32_t press_count = 0U;
    bool is_pressed = false;
    if (cm55_get_button_state(id, &press_count, &is_pressed))
    {
      if (refs->state_lbl[id] != NULL)
        lv_label_set_text(refs->state_lbl[id], is_pressed ? "Pressed" : "Released");
      if (refs->count_lbl[id] != NULL)
        lv_label_set_text_fmt(refs->count_lbl[id], "%lu", (unsigned long)press_count);
    }
  }
}

typedef struct
{
  lv_timer_t *timer;
  lv_obj_t *state_lbl[2];
  lv_obj_t *count_lbl[2];
} buttons_example_timers_t;

static void buttons_timer_delete_cb(lv_event_t *e)
{
  if (LV_EVENT_DELETE == lv_event_get_code(e))
  {
    buttons_example_timers_t *t = (buttons_example_timers_t *)lv_event_get_user_data(e);
    if (t != NULL && t->timer != NULL)
      lv_timer_delete(t->timer);
  }
}

static void example_10(void)
{
  lv_obj_t *scr = lv_screen_active();
  lv_obj_clean(scr);
  ui_bg_color(scr, lv_color_hex(0x0F172A));

  lv_obj_t *header = ui_clean_row(scr, 10, 16);
  ui_width_fill(header);
  ui_height(header, 52);
  lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
  ui_bg_color(header, lv_color_hex(0x0F172A));
  ui_bg_opa(header, LV_OPA_COVER);

  lv_obj_t *title = lv_label_create(header);
  lv_label_set_text(title, "User Buttons");
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);

  lv_obj_t *list_cont = ui_clean_scroll_container(scr, LV_DIR_VER);
  ui_width_fill(list_cont);
  lv_obj_set_height(list_cont, LV_PCT(100) - 52);
  lv_obj_align(list_cont, LV_ALIGN_TOP_LEFT, 0, 52);
  ui_pad_all(list_cont, 12);
  ui_gap(list_cont, 8);
  ui_flex_flow(list_cont, LV_FLEX_FLOW_COLUMN);
  ui_flex_align(list_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_scrollbar_mode(list_cont, LV_SCROLLBAR_MODE_AUTO);

  lv_obj_t *card = ui_card_container(list_cont, 8, 12);
  ui_width_fill(card);
  ui_height_content(card);
  ui_bg_color(card, lv_color_hex(0x1E293B));
  ui_bg_opa(card, LV_OPA_COVER);
  ui_gap(card, 4);
  ui_flex_flow(card, LV_FLEX_FLOW_COLUMN);
  ui_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

  lv_color_t color_btn = lv_color_hex(0x38BDF8);
  lv_color_t color_state = lv_color_hex(0xA78BFA);
  lv_color_t color_count = lv_color_hex(0xF59E0B);

  static buttons_example_timers_t timers;
  timers.state_lbl[0] = NULL;
  timers.state_lbl[1] = NULL;
  timers.count_lbl[0] = NULL;
  timers.count_lbl[1] = NULL;

  button_card_add_row(card, color_btn, LV_SYMBOL_KEYBOARD, "Button 0");
  timers.state_lbl[0] = button_card_add_row(card, color_state, LV_SYMBOL_EYE_OPEN, "Released");
  timers.count_lbl[0] = button_card_add_row(card, color_count, LV_SYMBOL_WARNING, "0");

  button_card_add_row(card, color_btn, LV_SYMBOL_KEYBOARD, "Button 1");
  timers.state_lbl[1] = button_card_add_row(card, color_state, LV_SYMBOL_EYE_OPEN, "Released");
  timers.count_lbl[1] = button_card_add_row(card, color_count, LV_SYMBOL_WARNING, "0");

  timers.timer = lv_timer_create(buttons_timer_cb, 150, (button_ui_refs_t *)&timers.state_lbl);
  lv_obj_add_event_cb(scr, buttons_timer_delete_cb, LV_EVENT_DELETE, &timers);
  buttons_timer_cb(timers.timer);
}

void run_example()
{
  const int id = 9;
  switch (id)
  {
  case 1:
    example_1();
    break;
  case 2:
    example_2();
    break;
  case 3:
    example_3();
    break;
  case 4:
    example_4();
    break;
  case 5:
    example_5();
    break;
  case 6:
    example_6();
    break;
  case 7:
    example_7();
    break;

  case 9:
    example_9();
    break;
  case 10:
    example_10();
    break;
  }
}

/* [] END OF FILE */
