/*******************************************************************************
 * File Name        : textarea.c
 *
 * Description      : Implementation of LVGL Textarea widget
 *
 *******************************************************************************/

#include "textarea.h"
#include "lvgl.h"

/*******************************************************************************
 * Function Name: textarea_create
 ********************************************************************************
 * Summary:
 *  Create a textarea widget with specified position, size, and settings.
 *
 * Parameters:
 *  parent: Parent object (usually a screen or container)
 *  x: X position (0 for default)
 *  y: Y position (0 for default)
 *  width: Width in pixels (0 for default)
 *  height: Height in pixels (0 for default)
 *  max_length: Maximum text length (0 for no limit)
 *  one_line: Single line mode (true) or multi-line (false)
 *
 * Return:
 *  lv_obj_t*: Pointer to the created textarea object
 *
 *******************************************************************************/
lv_obj_t *textarea_create(lv_obj_t *parent, lv_coord_t x, lv_coord_t y,
                          lv_coord_t width, lv_coord_t height,
                          uint32_t max_length, bool one_line) {
  if (parent == NULL) {
    return NULL;
  }

  /* Create the textarea */
  lv_obj_t *ta = lv_textarea_create(parent);
  if (ta == NULL) {
    return NULL;
  }

  /* Set position */
  if (x != 0 || y != 0) {
    lv_obj_set_pos(ta, x, y);
  }

  /* Set size */
  if (width != 0 && height != 0) {
    lv_obj_set_size(ta, width, height);
  }

  /* Set maximum length */
  if (max_length > 0) {
    lv_textarea_set_max_length(ta, max_length);
  }

  /* Set single line or multi-line mode */
  lv_textarea_set_one_line(ta, one_line);

  /* Disable password mode by default */
  lv_textarea_set_password_mode(ta, false);

  return ta;
}

/*******************************************************************************
 * Function Name: textarea_create_simple
 ********************************************************************************
 * Summary:
 *  Create a textarea widget with default settings.
 *
 * Parameters:
 *  parent: Parent object (usually a screen or container)
 *
 * Return:
 *  lv_obj_t*: Pointer to the created textarea object
 *
 *******************************************************************************/
lv_obj_t *textarea_create_simple(lv_obj_t *parent) {
  return textarea_create(parent, 0, 0, 0, 0, 0, false);
}

/*******************************************************************************
 * Function Name: textarea_apply_dark_theme
 ********************************************************************************
 * Summary:
 *  Apply dark theme styling to a textarea widget.
 *
 * Parameters:
 *  textarea: Textarea object to style
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void textarea_apply_dark_theme(lv_obj_t *textarea) {
  if (textarea == NULL) {
    return;
  }

  /* Textarea background */
  lv_obj_set_style_bg_color(textarea, lv_color_hex(0x2A2A2A), 0);
  lv_obj_set_style_bg_opa(textarea, LV_OPA_100, 0);

  /* Text color */
  lv_obj_set_style_text_color(textarea, lv_color_white(), 0);

  /* Font size (same as dropdown) */
  lv_obj_set_style_text_font(textarea, &lv_font_montserrat_24, 0);

  /* Border color */
  lv_obj_set_style_border_color(textarea, lv_color_hex(0x555555), 0);
  lv_obj_set_style_border_width(textarea, 1, 0);

  /* Placeholder text color (if any) */
  lv_obj_set_style_text_color(textarea, lv_color_hex(0x888888),
                              LV_PART_TEXTAREA_PLACEHOLDER);

  /* Cursor color */
  lv_obj_set_style_border_color(textarea, lv_color_white(), LV_PART_CURSOR);
  lv_obj_set_style_border_width(textarea, 2, LV_PART_CURSOR);
}

/* [] END OF FILE */
