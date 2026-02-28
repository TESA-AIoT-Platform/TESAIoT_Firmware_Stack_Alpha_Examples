/*******************************************************************************
 * File Name        : cm55_system.c
 *
 * Description      : Implementation of the CM55 system initialization (BSP,
 *                    RTC, retarget-io, LPTimer tickless idle).
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM55
 *
 *******************************************************************************/

#include "cm55_system.h"

#include "cy_syslib.h"
#include "cy_time.h"
#include "cybsp.h"
#include "cyabs_rtos.h"
#include "cm55_fatal_error.h"
#include "retarget_io_init.h"

#include "mtb_hal_rtc.h"
#include "mtb_hal_lptimer.h"
#include "task.h"

/*******************************************************************************
 * Macros
 *******************************************************************************/

#define LPTIMER_1_WAIT_TIME_USEC (62U)       /* Wait time in usec for MCWDT enable (e.g. 2 CLK_LF cycles). */
#define APP_LPTIMER_INTERRUPT_PRIORITY (1U)  /* LPTimer NVIC priority; 1 is highest. */

/*******************************************************************************
 * Statics / Global Variables
 *******************************************************************************/

static mtb_hal_lptimer_t lptimer_obj;       /* LPTimer HAL instance for tickless idle. */
static mtb_hal_rtc_t rtc_obj;               /* RTC HAL instance for CLIB support. */

static system_tick_hook_cb_t s_tick_hook_cb = NULL;  /* Registered tick hook callback; NULL if none. */
static void *s_tick_hook_user_data = NULL;           /* User data passed to the tick hook. */

/*******************************************************************************
 * Private Functions
 *******************************************************************************/

/**
 * Initializes CLIB support (RTC-based time) for the CM55 application.
 *
 */
static void setup_clib_support(void)
{
  mtb_clib_support_init(&rtc_obj);
}

/**
 * LPTimer interrupt handler; forwards to HAL process function.
 *
 */
static void lptimer_interrupt_handler(void)
{
  mtb_hal_lptimer_process_interrupt(&lptimer_obj);
}

/**
 * Configures LPTimer for FreeRTOS tickless idle: NVIC, MCWDT, HAL, cyabs_rtos.
 * Does not return on failure (calls cm55_handle_fatal_error).
 *
 */
static void setup_tickless_idle_timer(void)
{
  cy_stc_sysint_t lptimer_intr_cfg = {.intrSrc = CYBSP_CM55_LPTIMER_1_IRQ,
                                      .intrPriority = APP_LPTIMER_INTERRUPT_PRIORITY};

  /* Initialize the LPTimer interrupt and specify the interrupt handler. */
  cy_en_sysint_status_t interrupt_init_status =
      Cy_SysInt_Init(&lptimer_intr_cfg, lptimer_interrupt_handler);

  /* LPTimer interrupt initialization failed. Stop program execution. */
  if (CY_SYSINT_SUCCESS != interrupt_init_status)
  {
    cm55_handle_fatal_error("LPTimer interrupt init failed: %d", interrupt_init_status);
  }

  /* Enable NVIC interrupt. */
  NVIC_EnableIRQ(lptimer_intr_cfg.intrSrc);

  /* Initialize the MCWDT block used as LPTimer. */
  cy_en_mcwdt_status_t mcwdt_init_status =
      Cy_MCWDT_Init(CYBSP_CM55_LPTIMER_1_HW, &CYBSP_CM55_LPTIMER_1_config);

  /* MCWDT initialization failed. Stop program execution. */
  if (CY_MCWDT_SUCCESS != mcwdt_init_status)
  {
    cm55_handle_fatal_error("MCWDT init failed: %d", mcwdt_init_status);
  }

  /* Enable MCWDT instance. */
  Cy_MCWDT_Enable(CYBSP_CM55_LPTIMER_1_HW, CY_MCWDT_CTR_Msk, LPTIMER_1_WAIT_TIME_USEC);

  /* Setup LPTimer using the HAL object and BSP config. */
  cy_rslt_t result =
      mtb_hal_lptimer_setup(&lptimer_obj, &CYBSP_CM55_LPTIMER_1_hal_config);

  /* LPTimer setup failed. Stop program execution. */
  if (CY_RSLT_SUCCESS != result)
  {
    cm55_handle_fatal_error("LPTimer setup failed: %lu", (unsigned long)result);
  }

  /* Pass the LPTimer object to the RTOS abstraction for tickless idle. */
  cyabs_rtos_set_lptimer(&lptimer_obj);
}

/*******************************************************************************
 * Public API
 *******************************************************************************/

/**
 * Performs BSP, CLIB, LPTimer, retarget I/O, enables IRQ. Returns true on
 * success, false if cybsp_init fails. On LPTimer/CLIB failure does not return.
 *
 */
bool cm55_system_init(void)
{
  /* BSP init failed; return false so caller can handle. */
  if (CY_RSLT_SUCCESS != cybsp_init())
  {
    return false;
  }

  setup_clib_support();
  setup_tickless_idle_timer();
  init_retarget_io();
  __enable_irq();

  return true;
}

/**
 * Stores callback and user_data for the tick hook; overwrites previous registration.
 *
 */
void cm55_system_register_tick_callback(system_tick_hook_cb_t callback, void *user_data)
{
  s_tick_hook_cb = callback;
  s_tick_hook_user_data = user_data;
}

/**
 * FreeRTOS tick hook; invokes the registered callback with tick count and user_data if set.
 *
 */
void vApplicationTickHook(void)
{
  if (NULL != s_tick_hook_cb)
  {
    TickType_t tick = xTaskGetTickCountFromISR();
    system_tick_hook_params_t params = {
      .tick_count = (uint32_t)tick,
      .user_data = s_tick_hook_user_data,
    };
    s_tick_hook_cb(&params);
  }
}
