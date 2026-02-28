/*******************************************************************************
 * File Name        : display_controller.h
 *
 * Description      : TESA display controller: GFXSS, vg_lite, LVGL init,
 *                    display/indev ports, and graphics task (CM55).
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 *
 *******************************************************************************/

#ifndef DISPLAY_CONTROLLER_H
#define DISPLAY_CONTROLLER_H

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "display_i2c_config.h"
#include "lv_port_disp.h"
#include "lvgl.h"
#include "mtb_disp_ws7p0dsi_drv.h"
#include "vg_lite.h"
#include "vg_lite_platform.h"

#define GPU_INT_PRIORITY (3U)              /* NVIC priority for GFXSS GPU interrupt. */
#define DC_INT_PRIORITY (3U)              /* NVIC priority for display controller interrupt. */
#define I2C_CONTROLLER_IRQ_PRIORITY (2UL) /* NVIC priority for display/touch I2C controller. */

#define APP_BUFFER_COUNT (2U)                           /* Number of GPU command buffers. */
#define DEFAULT_GPU_CMD_BUFFER_SIZE ((64U) * (1024U))   /* Size in bytes of one GPU command buffer. */
#define GPU_TESSELLATION_BUFFER_SIZE ((MY_DISP_VER_RES) * 128U)  /* Tessellation buffer size in bytes. */
#define VGLITE_HEAP_SIZE                                                       \
  (((DEFAULT_GPU_CMD_BUFFER_SIZE) * (APP_BUFFER_COUNT)) +                      \
   ((GPU_TESSELLATION_BUFFER_SIZE) * (APP_BUFFER_COUNT)))  /* Total vg_lite heap in bytes. */

#define GPU_MEM_BASE (0x0U)   /* Base address for vg_lite GPU memory. */
#define VG_PARAMS_POS (0UL)   /* Index for vg_lite contiguous memory parameters. */

#define GFX_TASK_NAME ("CM55 Gfx Task")           /* FreeRTOS task name for graphics. */
#define GFX_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE * 16)  /* Stack size in words for GFX task. */
#define GFX_TASK_PRIORITY (configMAX_PRIORITIES - 1)  /* Highest priority for GFX task. */

extern TaskHandle_t rtos_cm55_gfx_task_handle;  /* Handle of the graphics FreeRTOS task. */

/**
 * Creates the graphics FreeRTOS task (display_controller_task). Returns pdPASS on
 * success, otherwise task create failed.
 */
BaseType_t display_controller_init(void);

/**
 * Graphics task: initializes GFXSS, DC, GPU, I2C, panel, vg_lite, LVGL,
 * display/indev ports, runs example; then LVGL timer loop. arg is unused.
 */
void display_controller_task(void *arg);

/**
 * Advances LVGL tick by 1 ms; call from tick hook or timer. Used for LVGL
 * internal timing.
 */
static inline void display_controller_tick(void) { lv_tick_inc(1); }

#endif /* DISPLAY_CONTROLLER_H */
