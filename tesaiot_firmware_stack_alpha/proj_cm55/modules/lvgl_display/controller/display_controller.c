/*******************************************************************************
 * File Name        : display_controller.c
 *
 * Description      : TESA display: GFXSS/DC/GPU init, vg_lite, LVGL, display
 *                    and input ports; graphics task runs LVGL and examples.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM55
 *
 *******************************************************************************/

#include "display_controller.h"
#include "cm55_fatal_error.h"
#include "cy_pdl.h"
#include "cybsp.h"
#include "cycfg.h"
#include "lv_port_indev.h"
#include "retarget_io_init.h"

#if defined(GFXSS_DC_IRQ)
#include "cy_graphics.h"
#include "ui/widgets/examples.h"
#endif

/*******************************************************************************
 * Global Variables
 *******************************************************************************/

TaskHandle_t rtos_cm55_gfx_task_handle = NULL; /* Handle of the graphics FreeRTOS task. */

#if defined(GFXSS_DC_IRQ)
CY_SECTION(".cy_gpu_buf")
uint8_t contiguous_mem[VGLITE_HEAP_SIZE] = {0xFF}; /* vg_lite contiguous heap. */

volatile void *vglite_heap_base = &contiguous_mem; /* Base pointer for vg_lite heap. */

cy_stc_sysint_t dc_irq_cfg = {.intrSrc = GFXSS_DC_IRQ, .intrPriority = DC_INT_PRIORITY}; /* Display controller IRQ config. */

cy_stc_sysint_t gpu_irq_cfg = {.intrSrc = GFXSS_GPU_IRQ, .intrPriority = GPU_INT_PRIORITY}; /* GFXSS GPU IRQ config. */

cy_stc_scb_i2c_context_t disp_touch_i2c_controller_context; /* I2C context for display/touch. */

cy_stc_sysint_t disp_touch_i2c_controller_irq_cfg = {
    .intrSrc = DISPLAY_I2C_CONTROLLER_IRQ,
    .intrPriority = I2C_CONTROLLER_IRQ_PRIORITY,
}; /* I2C controller interrupt configuration. */

extern cy_stc_gfx_context_t gfx_context; /* GFXSS HAL context. */
extern void *frame_buffer1;              /* Display frame buffer used by DC layer. */
#endif /* defined(GFXSS_DC_IRQ) */

/*******************************************************************************
 * Private Functions
 *******************************************************************************/

#if defined(GFXSS_DC_IRQ)
/**
 * Display controller interrupt handler; clears DC interrupt and notifies the
 * graphics task via xTaskNotifyFromISR. Yields if a higher-priority task woken.
 */
static void dc_irq_handler(void)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  Cy_GFXSS_Clear_DC_Interrupt(GFXSS, &gfx_context);

  xTaskNotifyFromISR(rtos_cm55_gfx_task_handle, 1, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);

  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * GPU interrupt handler; clears GFXSS GPU interrupt and forwards to vg_lite
 * IRQ handler.
 */
static void gpu_irq_handler(void)
{
  Cy_GFXSS_Clear_GPU_Interrupt(GFXSS, &gfx_context);
  vg_lite_IRQHandler();
}

/**
 * I2C controller interrupt for display/touch; forwards to Cy_SCB_I2C_Interrupt.
 */
static void disp_touch_i2c_controller_interrupt(void)
{
  Cy_SCB_I2C_Interrupt(DISPLAY_I2C_CONTROLLER_HW, &disp_touch_i2c_controller_context);
}
#endif /* defined(GFXSS_DC_IRQ) */

/*******************************************************************************
 * Public API
 *******************************************************************************/

#if !defined(GFXSS_DC_IRQ)
static void display_controller_task_no_disp(void *arg)
{
  (void)arg;
  for (;;)
  {
    vTaskDelay(portMAX_DELAY);
  }
}
#endif

/**
 * Creates the graphics FreeRTOS task (display_controller_task). Returns pdPASS on
 * success, otherwise task create failed.
 */
BaseType_t display_controller_init(void)
{
#if defined(GFXSS_DC_IRQ)
  return xTaskCreate(display_controller_task, GFX_TASK_NAME, GFX_TASK_STACK_SIZE, NULL, GFX_TASK_PRIORITY, &rtos_cm55_gfx_task_handle);
#else
  return xTaskCreate(display_controller_task_no_disp, GFX_TASK_NAME, GFX_TASK_STACK_SIZE, NULL, GFX_TASK_PRIORITY, &rtos_cm55_gfx_task_handle);
#endif
}

#if defined(GFXSS_DC_IRQ)
/**
 * Graphics task: initializes GFXSS, DC, GPU, I2C, panel, vg_lite, LVGL,
 * display/indev ports, runs example; then LVGL timer loop. arg is unused.
 */
void display_controller_task(void *arg)
{
  CY_UNUSED_PARAMETER(arg);

  cy_en_sysint_status_t sysint_status = CY_SYSINT_SUCCESS;
  cy_en_gfx_status_t gfx_status = CY_GFX_SUCCESS;
  vg_lite_error_t vglite_status = VG_LITE_SUCCESS;
  cy_rslt_t status = CY_RSLT_SUCCESS;
  cy_en_scb_i2c_status_t i2c_result = CY_SCB_I2C_SUCCESS;

  /* Configure GFXSS for display resolution and frame buffer. */
  GFXSS_config.mipi_dsi_cfg = &mtb_disp_ws7p0dsi_dsi_config;

  GFXSS_config.dc_cfg->gfx_layer_config->width = MY_DISP_HOR_RES;
  GFXSS_config.dc_cfg->gfx_layer_config->height = MY_DISP_VER_RES;
  GFXSS_config.dc_cfg->display_width = MY_DISP_HOR_RES;
  GFXSS_config.dc_cfg->display_height = MY_DISP_VER_RES;

  GFXSS_config.dc_cfg->gfx_layer_config->buffer_address = frame_buffer1;
  GFXSS_config.dc_cfg->gfx_layer_config->uv_buffer_address = frame_buffer1;

  gfx_status = Cy_GFXSS_Init(GFXSS, &GFXSS_config, &gfx_context);

  if (CY_GFX_SUCCESS == gfx_status)
  {
    /* Register and enable display controller interrupt. */
    sysint_status = Cy_SysInt_Init(&dc_irq_cfg, dc_irq_handler);

    if (CY_SYSINT_SUCCESS != sysint_status)
    {
      cm55_handle_fatal_error("DC interrupt registration failed: %d", sysint_status);
    }

    NVIC_EnableIRQ(GFXSS_DC_IRQ);

    /* Register and enable GPU interrupt. */
    sysint_status = Cy_SysInt_Init(&gpu_irq_cfg, gpu_irq_handler);

    if (CY_SYSINT_SUCCESS != sysint_status)
    {
      cm55_handle_fatal_error("GPU interrupt registration failed: %d", sysint_status);
    }

    Cy_GFXSS_Enable_GPU_Interrupt(GFXSS);

    NVIC_EnableIRQ(GFXSS_GPU_IRQ);

    /* Initialize I2C controller for display and touch. */
    i2c_result = Cy_SCB_I2C_Init(DISPLAY_I2C_CONTROLLER_HW,
                                 &DISPLAY_I2C_CONTROLLER_config,
                                 &disp_touch_i2c_controller_context);

    if (CY_SCB_I2C_SUCCESS != i2c_result)
    {
      cm55_handle_fatal_error("I2C controller init failed: %d", i2c_result);
    }

    sysint_status = Cy_SysInt_Init(&disp_touch_i2c_controller_irq_cfg,
                                   &disp_touch_i2c_controller_interrupt);

    if (CY_SYSINT_SUCCESS != sysint_status)
    {
      cm55_handle_fatal_error("I2C controller interrupt init failed: %d", sysint_status);
    }

    NVIC_EnableIRQ(disp_touch_i2c_controller_irq_cfg.intrSrc);

    Cy_SCB_I2C_Enable(DISPLAY_I2C_CONTROLLER_HW);

    vTaskDelay(pdMS_TO_TICKS(500));

    /* Initialize Waveshare 7-inch panel via I2C. */
    status = mtb_disp_ws7p0dsi_panel_init(DISPLAY_I2C_CONTROLLER_HW,
                                          &disp_touch_i2c_controller_context);

    if (CY_RSLT_SUCCESS != status)
    {
      cm55_handle_fatal_error("Waveshare 7-Inch R-Pi display init failed: %u",
                              (unsigned int)status);
    }
    /* Configure vg_lite memory and initialize. */
    vg_module_parameters_t vg_params;
    vg_params.register_mem_base = (uint32_t)GFXSS_GFXSS_GPU_GCNANO;
    vg_params.gpu_mem_base[VG_PARAMS_POS] = GPU_MEM_BASE;
    vg_params.contiguous_mem_base[VG_PARAMS_POS] = vglite_heap_base;
    vg_params.contiguous_mem_size[VG_PARAMS_POS] = VGLITE_HEAP_SIZE;

    vg_lite_init_mem(&vg_params);

    vglite_status = vg_lite_init((MY_DISP_HOR_RES) / 4, (MY_DISP_VER_RES) / 4);

    if (VG_LITE_SUCCESS == vglite_status)
    {
      /* Initialize LVGL, display/indev ports, and run UI example. */
      lv_init();
      lv_port_disp_init();
      lv_port_indev_init();
      run_example();
    }
    else
    {
      vg_lite_close();
      cm55_handle_fatal_error("vg_lite_init failed: %d", vglite_status);
    }
  }
  else
  {
    cm55_handle_fatal_error("GFXSS init failed: %d", gfx_status);
  }

  /* Run LVGL timer handler with bounded delay (max 5 ms). */
  for (;;)
  {
    uint32_t time_till_next = lv_timer_handler();
    if (time_till_next > 5)
    {
      time_till_next = 5;
    }
    vTaskDelay(pdMS_TO_TICKS(time_till_next));
  }
}
#endif /* defined(GFXSS_DC_IRQ) */
