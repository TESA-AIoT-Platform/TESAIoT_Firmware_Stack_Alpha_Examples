/*******************************************************************************
 * File Name        : textarea.h
 *
 * Description      : Header file for LVGL Textarea widget
 *
 *******************************************************************************/

#ifndef TEXTAREA_H
#define TEXTAREA_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/**
 * @brief Create a textarea widget
 *
 * @param parent Parent object (usually a screen or container)
 * @param x X position (0 for default)
 * @param y Y position (0 for default)
 * @param width Width in pixels (0 for default)
 * @param height Height in pixels (0 for default)
 * @param max_length Maximum text length (0 for no limit)
 * @param one_line Single line mode (true) or multi-line (false)
 * @return lv_obj_t* Pointer to the created textarea object
 */
lv_obj_t *textarea_create(lv_obj_t *parent, lv_coord_t x, lv_coord_t y,
                          lv_coord_t width, lv_coord_t height,
                          uint32_t max_length, bool one_line);

/**
 * @brief Create a textarea widget with default settings
 *
 * @param parent Parent object (usually a screen or container)
 * @return lv_obj_t* Pointer to the created textarea object
 */
lv_obj_t *textarea_create_simple(lv_obj_t *parent);

/**
 * @brief Apply dark theme styling to a textarea
 *
 * @param textarea Textarea object to style
 */
void textarea_apply_dark_theme(lv_obj_t *textarea);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TEXTAREA_H */
