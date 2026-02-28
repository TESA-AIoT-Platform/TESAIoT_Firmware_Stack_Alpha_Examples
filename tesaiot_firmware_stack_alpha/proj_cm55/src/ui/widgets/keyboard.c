/*******************************************************************************
 * File Name        : keyboard.c
 *
 * Description      : Implementation of LVGL Keyboard widget
 *
 *******************************************************************************/

#include "keyboard.h"
#include "lvgl.h"
#include <string.h>

/*******************************************************************************
 * Static Variables for Manual Keyboard Event Handling
 *******************************************************************************/
static uint32_t last_processed_btn_id =
    0xFFFFFFFF; /* Track last processed button to filter duplicates */

/*******************************************************************************
 * Function Name: keyboard_manual_event_handler
 ********************************************************************************
 * Summary:
 *  Manual event handler for keyboard widget to prevent double character input.
 *  Processes VALUE_CHANGED events and manually handles all textarea operations.
 *
 * Parameters:
 *  e: LVGL event object
 *
 * Return:
 *  void
 *
 *******************************************************************************/
static void keyboard_manual_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);

  /* Only process VALUE_CHANGED events */
  if (code != LV_EVENT_VALUE_CHANGED) {
    return;
  }

  lv_obj_t *keyboard = lv_event_get_target(e);
  keyboard_textarea_pair_t *pair =
      (keyboard_textarea_pair_t *)lv_event_get_user_data(e);

  if (pair == NULL || pair->textarea == NULL) {
    return;
  }

  lv_obj_t *textarea = pair->textarea;

  /* Get the selected button ID */
  uint32_t btn_id = lv_buttonmatrix_get_selected_button(keyboard);

  /* Filter out NONE button events (releases) */
  if (btn_id == LV_BUTTONMATRIX_BUTTON_NONE) {
    last_processed_btn_id = 0xFFFFFFFF;
    return;
  }

  /* Filter duplicates - only process on button state transition */
  /* If this is the same button as the last processed one, skip it */
  if (btn_id == last_processed_btn_id) {
    return;
  }

  /* Update tracking */
  last_processed_btn_id = btn_id;

  /* Get button text */
  const char *txt = lv_buttonmatrix_get_button_text(keyboard, btn_id);
  if (txt == NULL) {
    return;
  }

  /* Handle special keys - mode changes are handled by default handler */
  /* We only handle keys that affect the textarea */

  /* Close/Hide keyboard */
  if (lv_strcmp(txt, LV_SYMBOL_CLOSE) == 0 ||
      lv_strcmp(txt, LV_SYMBOL_KEYBOARD) == 0) {
    lv_obj_send_event(keyboard, LV_EVENT_CANCEL, NULL);
    lv_obj_send_event(textarea, LV_EVENT_CANCEL, NULL);
    return;
  }

  /* OK button */
  if (lv_strcmp(txt, LV_SYMBOL_OK) == 0) {
    lv_obj_send_event(keyboard, LV_EVENT_READY, NULL);
    lv_obj_send_event(textarea, LV_EVENT_READY, NULL);
    return;
  }

  /* Enter/Newline */
  if (lv_strcmp(txt, "Enter") == 0 || lv_strcmp(txt, LV_SYMBOL_NEW_LINE) == 0) {
    lv_textarea_add_char(textarea, '\n');
    if (lv_textarea_get_one_line(textarea)) {
      lv_obj_send_event(textarea, LV_EVENT_READY, NULL);
    }
    return;
  }

  /* Cursor left */
  if (lv_strcmp(txt, LV_SYMBOL_LEFT) == 0) {
    lv_textarea_cursor_left(textarea);
    return;
  }

  /* Cursor right */
  if (lv_strcmp(txt, LV_SYMBOL_RIGHT) == 0) {
    lv_textarea_cursor_right(textarea);
    return;
  }

  /* Backspace */
  if (lv_strcmp(txt, LV_SYMBOL_BACKSPACE) == 0) {
    lv_textarea_delete_char(textarea);
    return;
  }

  /* +/- sign toggle (for numeric input) */
  if (lv_strcmp(txt, "+/-") == 0) {
    uint32_t cur = lv_textarea_get_cursor_pos(textarea);
    const char *ta_txt = lv_textarea_get_text(textarea);
    if (ta_txt != NULL && ta_txt[0] == '-') {
      lv_textarea_set_cursor_pos(textarea, 1);
      lv_textarea_delete_char(textarea);
      lv_textarea_add_char(textarea, '+');
      lv_textarea_set_cursor_pos(textarea, cur);
    } else if (ta_txt != NULL && ta_txt[0] == '+') {
      lv_textarea_set_cursor_pos(textarea, 1);
      lv_textarea_delete_char(textarea);
      lv_textarea_add_char(textarea, '-');
      lv_textarea_set_cursor_pos(textarea, cur);
    } else {
      lv_textarea_set_cursor_pos(textarea, 0);
      lv_textarea_add_char(textarea, '-');
      lv_textarea_set_cursor_pos(textarea, cur + 1);
    }
    return;
  }

  /* Regular character input */
  lv_textarea_add_text(textarea, txt);
}

/*******************************************************************************
 * Function Name: keyboard_create
 ********************************************************************************
 * Summary:
 *  Create a keyboard widget with specified position and size.
 *
 * Parameters:
 *  parent: Parent object (usually a screen or container)
 *  x: X position (0 for default)
 *  y: Y position (0 for default)
 *  width: Width in pixels (0 for default)
 *  height: Height in pixels (0 for default)
 *
 * Return:
 *  lv_obj_t*: Pointer to the created keyboard object
 *
 *******************************************************************************/
lv_obj_t *keyboard_create(lv_obj_t *parent, lv_coord_t x, lv_coord_t y,
                          lv_coord_t width, lv_coord_t height) {
  if (parent == NULL) {
    return NULL;
  }

  /* Create the keyboard */
  lv_obj_t *kb = lv_keyboard_create(parent);
  if (kb == NULL) {
    return NULL;
  }

  /* Set position */
  if (x != 0 || y != 0) {
    lv_obj_set_pos(kb, x, y);
  }

  /* Set size */
  if (width != 0 && height != 0) {
    lv_obj_set_size(kb, width, height);
  }

  return kb;
}

/*******************************************************************************
 * Function Name: keyboard_create_simple
 ********************************************************************************
 * Summary:
 *  Create a keyboard widget with default size.
 *
 * Parameters:
 *  parent: Parent object (usually a screen or container)
 *
 * Return:
 *  lv_obj_t*: Pointer to the created keyboard object
 *
 *******************************************************************************/
lv_obj_t *keyboard_create_simple(lv_obj_t *parent) {
  return keyboard_create(parent, 0, 0, 0, 0);
}

/*******************************************************************************
 * Function Name: keyboard_set_textarea
 ********************************************************************************
 * Summary:
 *  Associate a keyboard with a textarea widget.
 *
 * Parameters:
 *  keyboard: Keyboard object
 *  textarea: Textarea object to associate with keyboard
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void keyboard_set_textarea(lv_obj_t *keyboard, lv_obj_t *textarea) {
  if (keyboard == NULL || textarea == NULL) {
    return;
  }

  lv_keyboard_set_textarea(keyboard, textarea);
}

/*******************************************************************************
 * Function Name: keyboard_attach_textarea_manual
 ********************************************************************************
 * Summary:
 *  Attach a textarea to a keyboard using manual event handling to prevent
 *  double character input. This function creates a keyboard-textarea pair
 *  and registers a custom event handler.
 *
 * Parameters:
 *  keyboard: Keyboard object
 *  textarea: Textarea object to attach to keyboard
 *
 * Return:
 *  void
 *
 * Note:
 *  The keyboard-textarea pair is stored statically. This function is designed
 *  for single keyboard usage. For multiple keyboards, this function would need
 *  to be enhanced to use dynamic allocation or a pool of pairs.
 *
 *******************************************************************************/
void keyboard_attach_textarea_manual(lv_obj_t *keyboard, lv_obj_t *textarea) {
  if (keyboard == NULL || textarea == NULL) {
    return;
  }

  /* Static storage for keyboard-textarea pair (persists for event handler
   * lifetime) */
  static keyboard_textarea_pair_t pair;
  pair.keyboard = keyboard;
  pair.textarea = textarea;

  /* Register manual event handler with pair as user_data */
  lv_obj_add_event_cb(keyboard, keyboard_manual_event_handler,
                      LV_EVENT_VALUE_CHANGED, &pair);
}

/*******************************************************************************
 * Function Name: keyboard_set_mode
 ********************************************************************************
 * Summary:
 *  Set keyboard mode (text lower, text upper, number, etc.).
 *
 * Parameters:
 *  keyboard: Keyboard object
 *  mode: Keyboard mode (LV_KEYBOARD_MODE_TEXT_LOWER,
 * LV_KEYBOARD_MODE_TEXT_UPPER, etc.)
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void keyboard_set_mode(lv_obj_t *keyboard, lv_keyboard_mode_t mode) {
  if (keyboard == NULL) {
    return;
  }

  lv_keyboard_set_mode(keyboard, mode);
}

/*******************************************************************************
 * Function Name: keyboard_apply_dark_theme
 ********************************************************************************
 * Summary:
 *  Apply dark theme styling to a keyboard widget.
 *
 * Parameters:
 *  keyboard: Keyboard object to style
 *
 * Return:
 *  void
 *
 * Note:
 *  Known limitation: LVGL keyboard widget repaints the entire keyboard on
 *  each button press (see https://github.com/lvgl/lvgl/issues/4295). This
 *  can cause noticeable delays, especially on slower displays.
 *
 *******************************************************************************/
void keyboard_apply_dark_theme(lv_obj_t *keyboard) {
  if (keyboard == NULL) {
    return;
  }

  /* Keyboard background */
  lv_obj_set_style_bg_color(keyboard, lv_color_hex(0x2A2A2A), 0);
  lv_obj_set_style_bg_opa(keyboard, LV_OPA_100, 0);

  /* Keyboard button background */
  lv_obj_set_style_bg_color(keyboard, lv_color_hex(0x3A3A3A), LV_PART_ITEMS);
  lv_obj_set_style_bg_opa(keyboard, LV_OPA_100, LV_PART_ITEMS);

  /* Keyboard button text color */
  lv_obj_set_style_text_color(keyboard, lv_color_white(), LV_PART_ITEMS);

  /* Font size (same as dropdown) */
  lv_obj_set_style_text_font(keyboard, &lv_font_montserrat_24, 0);

  /* Instant transition for keyboard buttons (set before pressed state) */
  static const lv_style_prop_t trans_props[] = {LV_STYLE_BG_OPA,
                                                LV_STYLE_BG_COLOR, 0};
  static lv_style_transition_dsc_t trans_dsc;
  lv_style_transition_dsc_init(&trans_dsc, trans_props, lv_anim_path_linear, 1,
                               0, NULL);
  lv_obj_set_style_transition(keyboard, &trans_dsc, LV_PART_ITEMS);

  /* Keyboard button pressed state */
  lv_obj_set_style_bg_color(keyboard, lv_color_hex(0x1A1A1A),
                            LV_PART_ITEMS | LV_STATE_PRESSED);
  lv_obj_set_style_bg_opa(keyboard, LV_OPA_100,
                          LV_PART_ITEMS | LV_STATE_PRESSED);

  /* Keyboard button border */
  lv_obj_set_style_border_color(keyboard, lv_color_hex(0x555555),
                                LV_PART_ITEMS);
  lv_obj_set_style_border_width(keyboard, 1, LV_PART_ITEMS);
}

/* [] END OF FILE */
