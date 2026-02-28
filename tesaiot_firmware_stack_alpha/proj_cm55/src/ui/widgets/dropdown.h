/*******************************************************************************
 * File Name        : dropdown.h
 *
 * Description      : Header file for LVGL Dropdown widget
 *
 *******************************************************************************/

#ifndef DROPDOWN_H
#define DROPDOWN_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Macros
 *******************************************************************************/
#define DROPDOWN_DEFAULT_WIDTH (391U)
#define DROPDOWN_DEFAULT_HEIGHT (LV_SIZE_CONTENT)

/*******************************************************************************
 * Type Definitions
 *******************************************************************************/
typedef struct {
  lv_obj_t *dropdown;
  const char *options;
  uint16_t selected_index;
} dropdown_handle_t;

/*******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/**
 * @brief Create a Dropdown widget
 *
 * @param parent Parent object (usually a screen or container)
 * @param options Options string (newline-separated, e.g.,
 * "Option1\nOption2\nOption3")
 * @param selected_index Initial selected option index (0-based)
 * @param x X position (0 for default)
 * @param y Y position (0 for default)
 * @param width Width in pixels (0 for default width)
 * @param height Height in pixels (LV_SIZE_CONTENT for auto)
 * @return lv_obj_t* Pointer to the created dropdown object
 */
lv_obj_t *dropdown_create(lv_obj_t *parent, const char *options,
                          uint16_t selected_index, lv_coord_t x, lv_coord_t y,
                          lv_coord_t width, lv_coord_t height);

/**
 * @brief Create a Dropdown widget with default settings
 *
 * @param parent Parent object (usually a screen or container)
 * @param options Options string (newline-separated)
 * @param selected_index Initial selected option index (0-based)
 * @return lv_obj_t* Pointer to the created dropdown object
 */
lv_obj_t *dropdown_create_simple(lv_obj_t *parent, const char *options,
                                 uint16_t selected_index);

/**
 * @brief Set dropdown options
 *
 * @param dropdown Dropdown object
 * @param options Options string (newline-separated)
 */
void dropdown_set_options(lv_obj_t *dropdown, const char *options);

/**
 * @brief Get dropdown options
 *
 * @param dropdown Dropdown object
 * @return const char* Options string
 */
const char *dropdown_get_options(lv_obj_t *dropdown);

/**
 * @brief Set selected option by index
 *
 * @param dropdown Dropdown object
 * @param index Option index (0-based)
 */
void dropdown_set_selected(lv_obj_t *dropdown, uint16_t index);

/**
 * @brief Get selected option index
 *
 * @param dropdown Dropdown object
 * @return uint16_t Selected option index (0-based)
 */
uint16_t dropdown_get_selected(lv_obj_t *dropdown);

/**
 * @brief Get selected option text
 *
 * @param dropdown Dropdown object
 * @return const char* Selected option text
 */
const char *dropdown_get_selected_str(lv_obj_t *dropdown);

/**
 * @brief Apply dark theme styling to a dropdown
 *
 * @param dropdown Dropdown object to style
 */
void dropdown_apply_dark_theme(lv_obj_t *dropdown);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DROPDOWN_H */
