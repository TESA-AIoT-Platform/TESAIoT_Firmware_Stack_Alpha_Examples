/*******************************************************************************
 * File Name        : cm33_system.c
 *
 * Description      : Implementation of the CM33 system initialization (BSP,
 *                    RTC, retarget-io, LPTimer tickless idle, CM55 enable).
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM33 (non-secure)
 *
 *******************************************************************************/

#include "cm33_system.h"

#include "cy_syslib.h"
#include "cy_time.h"
#include "cybsp.h"
#include "cyabs_rtos.h"
#include "error_handler.h"
#include "task.h"
#include "retarget_io_init.h"

/*******************************************************************************
 * Macros
 *******************************************************************************/

/* The timeout value in microseconds used to wait for the CM55 core to be booted */
#define CM55_BOOT_WAIT_TIME_USEC (10U)

/* App boot address for CM55 project */
#define CM55_APP_BOOT_ADDR (CYMEM_CM33_0_m55_nvm_START + CYBSP_MCUBOOT_HEADER_SIZE)

/* Enabling or disabling a MCWDT requires a wait time of upto 2 CLK_LF cycles
 * to come into effect. This wait time value will depend on the actual CLK_LF
 * frequency set by the BSP.
 */
#define LPTIMER_0_WAIT_TIME_USEC (62U)

/* Define the LPTimer interrupt priority number. '1' implies highest priority. */
#define APP_LPTIMER_INTERRUPT_PRIORITY (1U)

/*******************************************************************************
 * Statics / Global Variables
 *******************************************************************************/

/* LPTimer HAL object */
static mtb_hal_lptimer_t lptimer_obj;

/* RTC HAL object */
static mtb_hal_rtc_t rtc_obj;

static system_tick_hook_cb_t s_tick_hook_cb = NULL;
static void *s_tick_hook_user_data = NULL;

/*******************************************************************************
 * Private Functions
 *******************************************************************************/

/** Configures RTC and initializes CLIB support library. */
static void setup_clib_support(void)
{
  /* RTC Initialization */
  Cy_RTC_Init(&CYBSP_RTC_config);
  Cy_RTC_SetDateAndTime(&CYBSP_RTC_config);

  /* Initialize the ModusToolbox CLIB support library */
  mtb_clib_support_init(&rtc_obj);
}

/** LPTimer interrupt handler; forwards to HAL. */
static void lptimer_interrupt_handler(void)
{
  mtb_hal_lptimer_process_interrupt(&lptimer_obj);
}

/** Configures LPTimer for RTOS tickless idle mode. */
static void setup_tickless_idle_timer(void)
{
  /* Interrupt configuration structure for LPTimer */
  cy_stc_sysint_t lptimer_intr_cfg = {.intrSrc = CYBSP_CM33_LPTIMER_0_IRQ,
                                      .intrPriority = APP_LPTIMER_INTERRUPT_PRIORITY};

  /* Initialize the LPTimer interrupt and specify the interrupt handler. */
  cy_en_sysint_status_t interrupt_init_status =
      Cy_SysInt_Init(&lptimer_intr_cfg, lptimer_interrupt_handler);

  /* LPTimer interrupt initialization failed. Stop program execution. */
  if (CY_SYSINT_SUCCESS != interrupt_init_status)
  {
    handle_error(NULL);
  }

  /* Enable NVIC interrupt. */
  NVIC_EnableIRQ(lptimer_intr_cfg.intrSrc);

  /* Initialize the MCWDT block */
  cy_en_mcwdt_status_t mcwdt_init_status =
      Cy_MCWDT_Init(CYBSP_CM33_LPTIMER_0_HW, &CYBSP_CM33_LPTIMER_0_config);

  /* MCWDT initialization failed. Stop program execution. */
  if (CY_MCWDT_SUCCESS != mcwdt_init_status)
  {
    handle_error(NULL);
  }

  /* Enable MCWDT instance */
  Cy_MCWDT_Enable(CYBSP_CM33_LPTIMER_0_HW, CY_MCWDT_CTR_Msk, LPTIMER_0_WAIT_TIME_USEC);

  /* Setup LPTimer using the HAL object and desired configuration as defined
   * in the device configurator. */
  cy_rslt_t result =
      mtb_hal_lptimer_setup(&lptimer_obj, &CYBSP_CM33_LPTIMER_0_hal_config);

  /* LPTimer setup failed. Stop program execution. */
  if (CY_RSLT_SUCCESS != result)
  {
    handle_error(NULL);
  }

  /* Pass the LPTimer object to abstraction RTOS library that implements
   * tickless idle mode
   */
  cyabs_rtos_set_lptimer(&lptimer_obj);
}

/*******************************************************************************
 * Public API
 *******************************************************************************/

/** Initializes BSP, RTC, retarget-io, LPTimer (tickless idle). Returns true on success. */
bool cm33_system_init(void)
{
  cy_rslt_t result = CY_RSLT_SUCCESS;

  result = cybsp_init();

  if (CY_RSLT_SUCCESS != result)
  {
    return false;
  }

  setup_clib_support();
  init_retarget_io();
  setup_tickless_idle_timer();

  __enable_irq();

  return true;
}

/** Enables the CM55 core. Call after IPC pipe is started if boot order matters. */
void cm33_system_enable_cm55(void)
{
  Cy_SysEnableCM55(MXCM55, CM55_APP_BOOT_ADDR, CM55_BOOT_WAIT_TIME_USEC);
  Cy_SysLib_Delay(200U);
}

mtb_hal_rtc_t *cm33_system_get_rtc(void)
{
  return &rtc_obj;
}

void system_register_tick_hook(system_tick_hook_cb_t callback, void *user_data)
{
  s_tick_hook_cb = callback;
  s_tick_hook_user_data = user_data;
}

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

/* [] END OF FILE */
