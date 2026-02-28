/*******************************************************************************
 * File Name        : test_lvgl.c
 *
 * Description      : LVGL UI initialization and callbacks
 *
 *******************************************************************************/

#include "test_lvgl.h"
#include "lvgl.h"
#include <stdio.h>

/* External declaration for the LVGL logo image - from img_lvgl_logo_export.c */
extern const lv_image_dsc_t img_lvgl_logo;

/*******************************************************************************
 * Global Variables
 *******************************************************************************/
int32_t click_counter = 0;
lv_obj_t *g_label = NULL;
lv_obj_t *g_gauge = NULL;
lv_obj_t *g_arc = NULL;
lv_obj_t *g_arc_label = NULL;

/*******************************************************************************
 * Function Name: btn_click_callback
 ********************************************************************************
 * Summary:
 *  Callback function for button click event.
 *  Increments the counter and updates the label.
 *
 * Parameters:
 *  lv_event_t *e: Event object
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void btn_click_callback(lv_event_t *e) {
  LV_UNUSED(e);
  click_counter++;
  if (click_counter > 100) {
    click_counter = 0; /* Reset to 0 when exceeding 100 */
  }
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "Counter: %ld", (long)click_counter);
  lv_label_set_text(g_label, buffer);

  /* Update the arc value */
  if (g_arc) {
    lv_arc_set_value(g_arc, click_counter);
  }

  /* Update the arc center label */
  if (g_arc_label) {
    snprintf(buffer, sizeof(buffer), "%ld", (long)click_counter);
    lv_label_set_text(g_arc_label, buffer);
  }

  /* Update the bar value */
  if (g_gauge) {
    lv_bar_set_value(g_gauge, click_counter, LV_ANIM_OFF);
  }
}

/*******************************************************************************
 * Function Name: init_ui
 ********************************************************************************
 * Summary:
 *  Initialize the UI with a label and button.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
/*******************************************************************************
 * Function Name: set_screen_brightness
 ********************************************************************************
 * Summary:
 *  Creates a semi-transparent dark overlay to reduce perceived screen
 * brightness.
 *
 * Parameters:
 *  lv_opa_t opacity: Opacity of the dark overlay (0-255 or LV_OPA_* constants)
 *                    LV_OPA_20 = 20% (slight reduction)
 *                    LV_OPA_30 = 30% (moderate reduction)
 *                    LV_OPA_40 = 40% (strong reduction)
 *                    LV_OPA_50 = 50% (very strong reduction)
 *
 * Return:
 *  lv_obj_t*: Pointer to the brightness overlay object (can be NULL if not
 * needed)
 *
 *******************************************************************************/
static lv_obj_t *brightness_overlay = NULL;

void set_screen_brightness(lv_opa_t opacity) {
  /* Remove existing overlay if it exists */
  if (brightness_overlay != NULL) {
    lv_obj_delete(brightness_overlay);
    brightness_overlay = NULL;
  }

  /* Create a semi-transparent dark overlay to reduce brightness */
  brightness_overlay = lv_obj_create(lv_scr_act());
  lv_obj_set_size(brightness_overlay, LV_PCT(100), LV_PCT(100));
  lv_obj_set_pos(brightness_overlay, 0, 0);
  lv_obj_set_style_bg_color(brightness_overlay, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(brightness_overlay, opacity, 0);
  lv_obj_set_style_border_width(brightness_overlay, 0, 0);
  lv_obj_set_style_pad_all(brightness_overlay, 0, 0);
  lv_obj_clear_flag(brightness_overlay, LV_OBJ_FLAG_SCROLLABLE);
  /* Send to background so UI elements created later will appear on top */
  lv_obj_move_background(brightness_overlay);
}

void main_test_vlgl(void) {
  /* Set screen background color to dark green */
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_100, 0);

  /* Reduce screen brightness by adding a semi-transparent dark overlay */
  /* Adjust the opacity value as needed: LV_OPA_30 = 30% (moderate reduction) */
  set_screen_brightness(LV_OPA_100);

  /* Container to hold all UI objects */
  lv_obj_t *container = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(container, 0, 0);
  lv_obj_set_style_pad_all(container, 10, 0);
  lv_obj_set_style_pad_row(container, 20, 0);
  lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_set_size(container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_center(container);

  /* Create an arc to display counter value */
  lv_obj_t *arc = lv_arc_create(container);
  lv_arc_set_range(arc, 0, 100);
  lv_arc_set_value(arc, 0);
  lv_obj_set_size(arc, 180, 180);
  lv_obj_set_style_arc_color(arc, lv_color_hex(0x1E90FF), 0);
  g_arc = arc;

  /* Create a label at the center of the arc to display the value */
  lv_obj_t *arc_label = lv_label_create(arc);
  lv_label_set_text(arc_label, "0");
  lv_obj_set_style_text_font(arc_label, &lv_font_montserrat_40, 0);
  lv_obj_set_style_text_color(arc_label, lv_color_hex(0xFFFF00), 0);
  lv_obj_center(arc_label);
  g_arc_label = arc_label;

  /* Create a simple label to display */
  lv_obj_t *label = lv_label_create(container);
  lv_label_set_text(label, "Progress: 0");
  lv_obj_set_style_text_font(label, &lv_font_montserrat_40, 0);
  lv_obj_set_style_text_color(label, lv_color_hex(0x33ff33), 0);

  /* Store label reference for button callback */
  g_label = label;

  /* Create a progress bar to display counter value (0-100) */
  lv_obj_t *gauge = lv_bar_create(container);
  lv_bar_set_range(gauge, 0, 100);
  lv_bar_set_value(gauge, 0, LV_ANIM_OFF);
  lv_obj_set_size(gauge, 250, 20);
  g_gauge = gauge;

  /* Create a button */
  lv_obj_t *btn = lv_btn_create(container);
  lv_obj_set_size(btn, 200, 70);
  lv_obj_set_style_bg_color(btn, lv_color_hex(0x1E90FF), 0);

  /* Add button callback for click events */
  lv_obj_add_event_cb(btn, btn_click_callback, LV_EVENT_PRESSING, NULL);

  lv_obj_t *btn_label = lv_label_create(btn);
  lv_label_set_text(btn_label, "RUN");
  lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_20, 0);
  lv_obj_set_style_pad_all(btn_label, 10, 0);
  lv_obj_center(btn_label);
}

/*******************************************************************************
 * Function Name: draw_lvgl_logo
 ********************************************************************************
 * Summary:
 *  Draws the LVGL logo image at the top-left corner of the screen.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void draw_lvgl_logo(void) {
  /* Create an image object on the active screen */
  lv_obj_t *logo_img = lv_image_create(lv_scr_act());

  /* Set the image source to the LVGL logo */
  lv_image_set_src(logo_img, &img_lvgl_logo);

  /* Position the logo at the top-center of the screen (original size is 42x42)
   */
  lv_coord_t screen_width = lv_obj_get_width(lv_scr_act());
  lv_coord_t logo_width = 42;
  lv_coord_t x_pos = (screen_width - logo_width) / 2;
  lv_obj_set_pos(logo_img, x_pos, 0);

  /* Remove any default styling that might interfere */
  lv_obj_clear_flag(logo_img, LV_OBJ_FLAG_SCROLLABLE);
}

/*******************************************************************************
 * Function Name: draw_fullscreen_logo
 ********************************************************************************
 * Summary:
 *  Draws the LVGL logo image stretched to fill the entire screen.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void draw_fullscreen_logo(void) {
  /* Create an image object on the active screen */
  lv_obj_t *fullscreen_img = lv_image_create(lv_scr_act());

  /* Set the image source to the LVGL logo */
  lv_image_set_src(fullscreen_img, &img_lvgl_logo);

  /* Get screen dimensions */
  lv_coord_t screen_width = lv_obj_get_width(lv_scr_act());
  lv_coord_t screen_height = lv_obj_get_height(lv_scr_act());

  /* Set the image object size to full screen */
  lv_obj_set_size(fullscreen_img, screen_width, screen_height);

  /* Position at top-left */
  lv_obj_set_pos(fullscreen_img, 0, 0);

  /* Stretch the image to fill the entire widget area */
  lv_image_set_inner_align(fullscreen_img, LV_IMAGE_ALIGN_STRETCH);

  /* Remove any default styling that might interfere */
  lv_obj_clear_flag(fullscreen_img, LV_OBJ_FLAG_SCROLLABLE);

  /* Send to back so other widgets appear on top if any */
  lv_obj_move_background(fullscreen_img);
}
