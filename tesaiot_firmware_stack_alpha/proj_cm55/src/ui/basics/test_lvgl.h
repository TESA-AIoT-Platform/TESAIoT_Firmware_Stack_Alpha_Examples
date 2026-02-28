/*******************************************************************************
 * File Name        : test_lvgl.h
 *
 * Description      : Header file for LVGL UI testing
 *
 *******************************************************************************/

#ifndef TEST_LVGL_H
#define TEST_LVGL_H

#include "lvgl.h"

/*******************************************************************************
 * Global Variables
 *******************************************************************************/
extern int32_t click_counter;
extern lv_obj_t *g_label;

/*******************************************************************************
 * Function Prototypes
 *******************************************************************************/
void btn_click_callback(lv_event_t *e);
void main_test_vlgl(void);
void draw_lvgl_logo(void);
void draw_fullscreen_logo(void);
void set_screen_brightness(lv_opa_t opacity);

#endif /* TEST_LVGL_H */
