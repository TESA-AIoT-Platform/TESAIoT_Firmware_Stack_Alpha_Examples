/*******************************************************************************
 * File Name        : rtos_stats.c
 *
 * Description      : Implementation of FreeRTOS run-time stats: TCPWM timer
 *                    for run-time counter, idle percentage calculation.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM55
 *
 *******************************************************************************/

#include "rtos_stats.h"
#include "cm55_fatal_error.h"
#include "FreeRTOS.h"
#include "cy_pdl.h"
#include "cybsp.h"
#include "cycfg.h"
#include "task.h"

#if (configGENERATE_RUN_TIME_STATS == 1)

/*******************************************************************************
 * Public API
 *******************************************************************************/

/**
 * Initializes and starts the TCPWM counter used for FreeRTOS run-time stats.
 * Calls cm55_handle_fatal_error and does not return on init failure.
 */
void setup_run_time_stats_timer(void) {
  uint32_t tcpwm_status = Cy_TCPWM_Counter_Init(CYBSP_GENERAL_PURPOSE_TIMER_HW,
                                                CYBSP_GENERAL_PURPOSE_TIMER_NUM,
                                                &CYBSP_GENERAL_PURPOSE_TIMER_config);
  if (CY_TCPWM_SUCCESS != tcpwm_status) {
    cm55_handle_fatal_error("TCPWM counter init failed: %lu", (unsigned long)tcpwm_status);
  }

  /* Enable the counter and start a single run. */
  Cy_TCPWM_Counter_Enable(CYBSP_GENERAL_PURPOSE_TIMER_HW,
                          CYBSP_GENERAL_PURPOSE_TIMER_NUM);

  Cy_TCPWM_TriggerStart_Single(CYBSP_GENERAL_PURPOSE_TIMER_HW,
                               CYBSP_GENERAL_PURPOSE_TIMER_NUM);
}

/**
 * Returns the current TCPWM counter value driving portGET_RUN_TIME_COUNTER_VALUE.
 */
uint32_t get_run_time_counter_value(void) {
  return (Cy_TCPWM_Counter_GetCounter(CYBSP_GENERAL_PURPOSE_TIMER_HW,
                                      CYBSP_GENERAL_PURPOSE_TIMER_NUM));
}

/**
 * Computes idle time percentage (0-100) since last call using run-time counters.
 * Uses static state; first call or invalid diff returns 0.
 */
uint32_t calculate_idle_percentage(void) {
  static uint32_t previousIdleTime = 0;
  static TickType_t previousTick = 0;
  uint32_t time_diff = 0;
  uint32_t idle_percent = 0;

  /* Sample current idle and run-time counter values. */
  uint32_t currentIdleTime = ulTaskGetIdleRunTimeCounter();
  TickType_t currentTick = portGET_RUN_TIME_COUNTER_VALUE();

  time_diff = currentTick - previousTick;

  if ((currentIdleTime >= previousIdleTime) && (currentTick > previousTick)) {
    idle_percent = ((currentIdleTime - previousIdleTime) * 100) / time_diff;
  }

  /* Store for next call. */
  previousIdleTime = ulTaskGetIdleRunTimeCounter();
  previousTick = portGET_RUN_TIME_COUNTER_VALUE();

  return idle_percent;
}
#endif
