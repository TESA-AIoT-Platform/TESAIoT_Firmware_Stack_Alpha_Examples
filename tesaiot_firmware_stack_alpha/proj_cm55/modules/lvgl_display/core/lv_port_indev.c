/*******************************************************************************
 * File Name        : lv_port_indev.c
 *
 * Description      : This file provides implementation of low level input
 * device driver for LVGL.
 *
 * Related Document : See README.md
 *
 ********************************************************************************
 * (c) 2025, Infineon Technologies AG, or an affiliate of Infineon
 * Technologies AG. All rights reserved.
 * This software, associated documentation and materials ("Software") is
 * owned by Infineon Technologies AG or one of its affiliates ("Infineon")
 * and is protected by and subject to worldwide patent protection, worldwide
 * copyright laws, and international treaty provisions. Therefore, you may use
 * this Software only as provided in the license agreement accompanying the
 * software package from which you obtained this Software. If no license
 * agreement applies, then any use, reproduction, modification, translation, or
 * compilation of this Software is prohibited without the express written
 * permission of Infineon.
 *
 * Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
 * IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
 * THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
 * SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
 * Infineon reserves the right to make changes to the Software without notice.
 * You are responsible for properly designing, programming, and testing the
 * functionality and safety of your intended application of the Software, as
 * well as complying with any legal requirements related to its use. Infineon
 * does not guarantee that the Software will be free from intrusion, data theft
 * or loss, or other breaches ("Security Breaches"), and Infineon shall have
 * no liability arising out of any Security Breaches. Unless otherwise
 * explicitly approved by Infineon, the Software may not be used in any
 * application where a failure of the Product or any consequences of the use
 * thereof can reasonably be expected to result in personal injury.
 *******************************************************************************/

/*******************************************************************************
 * Header Files
 *******************************************************************************/
#include "lv_port_indev.h"
#include "cy_utils.h"
#include "lv_indev_private.h"
#include "lv_port_disp.h"
#if defined(TOUCH_VIA_IPC)
#else
#if defined(MTB_CTP_GT911)
#include "mtb_ctp_gt911.h"
#elif defined(MTB_CTP_ILI2511)
#include "mtb_ctp_ili2511.h"
#elif defined(MTB_CTP_FT5406)
#include "mtb_ctp_ft5406.h"
#endif
#endif
#include "cybsp.h"
#include "display_i2c_config.h"

/*******************************************************************************
 * Macros
 *******************************************************************************/
#if defined(MTB_CTP_ILI2511)
#define CTP_RESET_PORT GPIO_PRT17
#define CTP_RESET_PIN (3U)
#define CTP_IRQ_PORT GPIO_PRT17
#define CTP_IRQ_PIN (2U)
#endif

#define INDEV_READ_PERIOD_MS                                                   \
  10U /* Increased from 100ms to 10ms for better touch responsiveness */

/*******************************************************************************
 * Global Variables
 *******************************************************************************/
lv_indev_t *indev_touchpad;

#if defined(MTB_CTP_ILI2511)
/* ILI2511 touch controller configuration */
mtb_ctp_ili2511_config_t ctp_ili2511_cfg = {
    .scb_inst = DISPLAY_I2C_CONTROLLER_HW,
    .i2c_context = &disp_touch_i2c_controller_context,
    .rst_port = CTP_RESET_PORT,
    .rst_pin = CTP_RESET_PIN,
    .irq_port = CTP_IRQ_PORT,
    .irq_pin = CTP_IRQ_PIN,
    .irq_num = ioss_interrupts_gpio_17_IRQn,
    .touch_event = false,
};
#endif /* MTB_CTP_ILI2511 */

#if defined(MTB_CTP_FT5406)
mtb_ctp_ft5406_config_t ctp_ft5406_cfg = {
    .i2c_base = DISPLAY_I2C_CONTROLLER_HW,
    .i2c_context = &disp_touch_i2c_controller_context,
};
#endif /* MTB_CTP_FT5406 */

#if defined(TOUCH_VIA_IPC)
static volatile int16_t s_touch_ipc_x = 0;
static volatile int16_t s_touch_ipc_y = 0;
static volatile uint8_t s_touch_ipc_pressed = 0U;

void lv_port_indev_touch_from_ipc_set(int16_t x, int16_t y, uint8_t pressed)
{
  int16_t cx = x;
  int16_t cy = y;
  if (cx < 0) { cx = 0; }
  if (cy < 0) { cy = 0; }
  if (cx >= (int16_t)ACTUAL_DISP_HOR_RES) { cx = (int16_t)ACTUAL_DISP_HOR_RES - 1; }
  if (cy >= (int16_t)ACTUAL_DISP_VER_RES) { cy = (int16_t)ACTUAL_DISP_VER_RES - 1; }
  s_touch_ipc_x = cx;
  s_touch_ipc_y = cy;
  s_touch_ipc_pressed = pressed;
}
#endif

/*******************************************************************************
 * Function Name: touchpad_init
 ********************************************************************************
 * Summary:
 *  Initialization function for touchpad supported by LVGL.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
static void touchpad_init(void) {
#if defined(TOUCH_VIA_IPC)
  (void)0;
#else
  cy_rslt_t result = CY_RSLT_SUCCESS;

#if defined(MTB_CTP_GT911)
  result = mtb_gt911_init(DISPLAY_I2C_CONTROLLER_HW,
                          &disp_touch_i2c_controller_context);

#elif defined(MTB_CTP_ILI2511)
  result = mtb_ctp_ili2511_init(&ctp_ili2511_cfg);
#elif defined(MTB_CTP_FT5406)
  result = (cy_rslt_t)mtb_ctp_ft5406_init(&ctp_ft5406_cfg);
#endif

  if (CY_RSLT_SUCCESS != result) {
    CY_ASSERT(0);
  }
#endif
}

/*******************************************************************************
 * Function Name: touchpad_read
 ********************************************************************************
 * Summary:
 *  Touchpad read function called by the LVGL library.
 *  Here you will find example implementation of input devices supported by
 *  LVGL:
 *   - Touchpad
 *   - Mouse (with cursor support)
 *   - Keypad (supports GUI usage only with key)
 *   - Encoder (supports GUI usage only with: left, right, push)
 *   - Button (external buttons to press points on the screen)
 *
 *   The `..._read()` function are only examples.
 *   You should shape them according to your hardware.
 *
 *
 * Parameters:
 *  *indev_drv: Pointer to the input driver structure to be registered by LVGL.
 *  *data: Pointer to the data buffer holding touch coordinates.
 *
 * Return:
 *  void
 *
 *******************************************************************************/
LV_ATTRIBUTE_FAST_MEM void touchpad_read(lv_indev_t *indev_drv,
                                         lv_indev_data_t *data) {
  (void)indev_drv;

  data->state = LV_INDEV_STATE_REL;

#if defined(TOUCH_VIA_IPC)
  data->point.x = (lv_coord_t)s_touch_ipc_x;
  data->point.y = (lv_coord_t)s_touch_ipc_y;
  if (0U != s_touch_ipc_pressed) {
    data->state = LV_INDEV_STATE_PR;
  }
#else
  static int touch_x = 0;
  static int touch_y = 0;
  cy_rslt_t result = CY_RSLT_SUCCESS;

#if defined(MTB_CTP_GT911)
  result = mtb_gt911_get_single_touch(DISPLAY_I2C_CONTROLLER_HW,
                                      &disp_touch_i2c_controller_context,
                                      &touch_x, &touch_y);

  if (CY_RSLT_SUCCESS == result) {
    data->state = LV_INDEV_STATE_PR;
  }
#elif defined(MTB_CTP_ILI2511)
  result = mtb_ctp_ili2511_get_single_touch(&touch_x, &touch_y);

  if (CY_RSLT_SUCCESS == result) {
    data->state = LV_INDEV_STATE_PR;
  }
#elif defined(MTB_CTP_FT5406)
  mtb_ctp_touch_event_t touch_event;
  result = (cy_rslt_t)mtb_ctp_ft5406_get_single_touch(&touch_event, &touch_x,
                                                      &touch_y);

  if ((CY_RSLT_SUCCESS == result) && ((MTB_CTP_TOUCH_DOWN == touch_event) ||
                                      (MTB_CTP_TOUCH_CONTACT == touch_event))) {
    data->state = LV_INDEV_STATE_PR;
  }

#endif

#if defined(MTB_CTP_FT5406)
  data->point.x = ACTUAL_DISP_HOR_RES - touch_x;
  data->point.y = ACTUAL_DISP_VER_RES - touch_y;
#elif defined(MTB_CTP_ILI2511) || defined(MTB_CTP_GT911)
  data->point.x = touch_x;
  data->point.y = touch_y;
#endif
#endif
}

/*******************************************************************************
 * Function Name: lv_port_indev_init
 ********************************************************************************
 * Summary:
 *  Initialization function for input devices supported by LittelvGL.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void lv_port_indev_init(void) {
  /* Initialize your touchpad if you have. */
  touchpad_init();

  /* Register a touchpad input device */
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchpad_read);
  lv_timer_pause(indev->read_timer);
  lv_timer_reset(indev->read_timer);
  lv_timer_set_period(indev->read_timer, INDEV_READ_PERIOD_MS);
  lv_timer_resume(indev->read_timer);
}

/* [] END OF FILE */
