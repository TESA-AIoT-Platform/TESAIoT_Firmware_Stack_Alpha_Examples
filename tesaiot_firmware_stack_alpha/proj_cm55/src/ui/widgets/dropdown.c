/*******************************************************************************
 * File Name        : dropdown.c
 *
 * Description      : Implementation of LVGL Dropdown widget
 *
 *******************************************************************************/

#include "dropdown.h"
#include "lvgl.h"
#include <stdio.h>
#include <string.h>

/*******************************************************************************
 * Function Name: dropdown_create
 ********************************************************************************
 * Summary:
 *  Create a Dropdown widget with specified options and position.
 *
 * Parameters:
 *  parent: Parent object (usually a screen or container)
 *  options: Options string (newline-separated, e.g.,
 * "Option1\nOption2\nOption3") selected_index: Initial selected option index
 * (0-based) x: X position (0 for default) y: Y position (0 for default) width:
 * Width in pixels (0 for default width) height: Height in pixels
 * (LV_SIZE_CONTENT for auto)
 *
 * Return:
 *  lv_obj_t*: Pointer to the created dropdown object
 *
 *******************************************************************************/
lv_obj_t *dropdown_create(lv_obj_t *parent, const char *options,
                          uint16_t selected_index, lv_coord_t x, lv_coord_t y,
                          lv_coord_t width, lv_coord_t height) {
  if (parent == NULL || options == NULL) {
    return NULL;
  }

  /* Create the Dropdown */
  lv_obj_t *dd = lv_dropdown_create(parent);
  if (dd == NULL) {
    return NULL;
  }

  /* Set position */
  if (x != 0 || y != 0) {
    lv_obj_set_pos(dd, x, y);
  }

  /* Set size */
  if (width == 0) {
    width = DROPDOWN_DEFAULT_WIDTH;
  }
  if (height == 0) {
    height = DROPDOWN_DEFAULT_HEIGHT;
  }
  lv_obj_set_size(dd, width, height);

  /* Set options */
  lv_dropdown_set_options(dd, options);

  /* Set selected option */
  lv_dropdown_set_selected(dd, selected_index);

  return dd;
}

/*******************************************************************************
 * Function Name: dropdown_create_simple
 ********************************************************************************
 * Summary:
 *  Create a Dropdown widget with default settings.
 *
 * Parameters:
 *  parent: Parent object (usually a screen or container)
 *  options: Options string (newline-separated)
 *  selected_index: Initial selected option index (0-based)
 *
 * Return:
 *  lv_obj_t*: Pointer to the created dropdown object
 *
 *******************************************************************************/
lv_obj_t *dropdown_create_simple(lv_obj_t *parent, const char *options,
                                 uint16_t selected_index) {
  return dropdown_create(parent, options, selected_index, 0, 0, 0, 0);
}

/*******************************************************************************
 * Function Name: dropdown_set_options
 ********************************************************************************
 * Summary:
 *  Set dropdown options.
 *
 * Parameters:
 *  dropdown: Dropdown object
 *  options: Options string (newline-separated)
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void dropdown_set_options(lv_obj_t *dropdown, const char *options) {
  if (dropdown == NULL || options == NULL) {
    return;
  }

  lv_dropdown_set_options(dropdown, options);
}

/*******************************************************************************
 * Function Name: dropdown_get_options
 ********************************************************************************
 * Summary:
 *  Get dropdown options.
 *
 * Parameters:
 *  dropdown: Dropdown object
 *
 * Return:
 *  const char*: Options string
 *
 *******************************************************************************/
const char *dropdown_get_options(lv_obj_t *dropdown) {
  if (dropdown == NULL) {
    return NULL;
  }

  return lv_dropdown_get_options(dropdown);
}

/*******************************************************************************
 * Function Name: dropdown_set_selected
 ********************************************************************************
 * Summary:
 *  Set selected option by index.
 *
 * Parameters:
 *  dropdown: Dropdown object
 *  index: Option index (0-based)
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void dropdown_set_selected(lv_obj_t *dropdown, uint16_t index) {
  if (dropdown == NULL) {
    return;
  }

  lv_dropdown_set_selected(dropdown, index);
}

/*******************************************************************************
 * Function Name: dropdown_get_selected
 ********************************************************************************
 * Summary:
 *  Get selected option index.
 *
 * Parameters:
 *  dropdown: Dropdown object
 *
 * Return:
 *  uint16_t: Selected option index (0-based)
 *
 *******************************************************************************/
uint16_t dropdown_get_selected(lv_obj_t *dropdown) {
  if (dropdown == NULL) {
    return 0;
  }

  return lv_dropdown_get_selected(dropdown);
}

/*******************************************************************************
 * Function Name: dropdown_get_selected_str
 ********************************************************************************
 * Summary:
 *  Get selected option text.
 *
 * Parameters:
 *  dropdown: Dropdown object
 *
 * Return:
 *  const char*: Selected option text (static buffer, don't free)
 *
 * Note:
 *  This function uses a static buffer. For thread safety or multiple calls,
 *  use lv_dropdown_get_selected_str() directly with your own buffer.
 *
 *******************************************************************************/
const char *dropdown_get_selected_str(lv_obj_t *dropdown) {
  if (dropdown == NULL) {
    return NULL;
  }

  /* LVGL 9.2.0 requires a buffer for get_selected_str */
  static char buf[64];
  lv_dropdown_get_selected_str(dropdown, buf, sizeof(buf));
  return buf;
}

/*******************************************************************************
 * Function Name: dropdown_apply_dark_theme
 ********************************************************************************
 * Summary:
 *  Apply dark theme styling to a dropdown widget.
 *
 * Parameters:
 *  dropdown: Dropdown object to style
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void dropdown_apply_dark_theme(lv_obj_t *dropdown) {
  if (dropdown == NULL) {
    return;
  }

  /* Main dropdown button background */
  lv_obj_set_style_bg_color(dropdown, lv_color_hex(0x2A2A2A), 0);
  lv_obj_set_style_bg_opa(dropdown, LV_OPA_100, 0);

  /* Text color */
  lv_obj_set_style_text_color(dropdown, lv_color_white(), 0);

  /* Font size */
  lv_obj_set_style_text_font(dropdown, &lv_font_montserrat_24, 0);

  /* Border color */
  lv_obj_set_style_border_color(dropdown, lv_color_hex(0x555555), 0);
  lv_obj_set_style_border_width(dropdown, 1, 0);

  /* Dropdown list background (when opened) */
  lv_obj_t *list = lv_dropdown_get_list(dropdown);
  if (list != NULL) {
    lv_obj_set_style_bg_color(list, lv_color_hex(0x2A2A2A), 0);
    lv_obj_set_style_bg_opa(list, LV_OPA_100, 0);
    lv_obj_set_style_text_color(list, lv_color_white(), 0);
    lv_obj_set_style_text_font(list, &lv_font_montserrat_24, 0);
    lv_obj_set_style_border_color(list, lv_color_hex(0x555555), 0);
  }
}

/* [] END OF FILE */
