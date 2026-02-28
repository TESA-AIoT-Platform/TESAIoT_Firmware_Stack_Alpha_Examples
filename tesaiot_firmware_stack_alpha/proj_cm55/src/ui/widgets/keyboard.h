/*******************************************************************************
 * File Name        : keyboard.h
 *
 * Description      : Header file for LVGL Keyboard widget
 *
 *******************************************************************************/

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "lvgl.h"
#include "textarea.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Type Definitions
 *******************************************************************************/
typedef struct {
  lv_obj_t *keyboard;
  lv_obj_t *textarea;
} keyboard_textarea_pair_t;

/*******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/**
 * @brief Create a keyboard widget
 *
 * @param parent Parent object (usually a screen or container)
 * @param x X position (0 for default)
 * @param y Y position (0 for default)
 * @param width Width in pixels (0 for default)
 * @param height Height in pixels (0 for default)
 * @return lv_obj_t* Pointer to the created keyboard object
 */
lv_obj_t *keyboard_create(lv_obj_t *parent, lv_coord_t x, lv_coord_t y,
                          lv_coord_t width, lv_coord_t height);

/**
 * @brief Create a keyboard widget with default size
 *
 * @param parent Parent object (usually a screen or container)
 * @return lv_obj_t* Pointer to the created keyboard object
 */
lv_obj_t *keyboard_create_simple(lv_obj_t *parent);

/**
 * @brief Associate a keyboard with a textarea
 *
 * @param keyboard Keyboard object
 * @param textarea Textarea object to associate with keyboard
 */
void keyboard_set_textarea(lv_obj_t *keyboard, lv_obj_t *textarea);

/**
 * @brief Attach a textarea to a keyboard using manual event handling
 *
 * This function prevents double character input by using a custom event handler
 * instead of the default keyboard-textarea linking mechanism.
 *
 * @param keyboard Keyboard object
 * @param textarea Textarea object to attach to keyboard
 */
void keyboard_attach_textarea_manual(lv_obj_t *keyboard, lv_obj_t *textarea);

/**
 * @brief Set keyboard mode
 *
 * @param keyboard Keyboard object
 * @param mode Keyboard mode (LV_KEYBOARD_MODE_TEXT_LOWER,
 * LV_KEYBOARD_MODE_TEXT_UPPER, etc.)
 */
void keyboard_set_mode(lv_obj_t *keyboard, lv_keyboard_mode_t mode);

/**
 * @brief Apply dark theme styling to a keyboard
 *
 * @param keyboard Keyboard object to style
 */
void keyboard_apply_dark_theme(lv_obj_t *keyboard);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KEYBOARD_H */
