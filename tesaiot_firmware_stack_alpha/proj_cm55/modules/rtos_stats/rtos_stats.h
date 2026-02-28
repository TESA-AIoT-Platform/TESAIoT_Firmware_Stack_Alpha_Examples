/*******************************************************************************
 * File Name        : rtos_stats.h
 *
 * Description      : FreeRTOS run-time statistics: timer setup, counter read,
 *                    and idle percentage (when configGENERATE_RUN_TIME_STATS).
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 *
 *******************************************************************************/

#ifndef RTOS_STATS_H
#define RTOS_STATS_H

#include "FreeRTOS.h"
#include <stdint.h>


#if (configGENERATE_RUN_TIME_STATS == 1)
#define TCPWM_TIMER_INT_PRIORITY (1U) /* NVIC priority for TCPWM timer used by run-time stats (unused in current impl). */

/**
 * Initializes and starts the TCPWM counter used for FreeRTOS run-time stats.
 * Calls cm55_handle_fatal_error and does not return on init failure.
 */
void setup_run_time_stats_timer(void);

/**
 * Returns the current TCPWM counter value driving portGET_RUN_TIME_COUNTER_VALUE.
 */
uint32_t get_run_time_counter_value(void);

/**
 * Computes idle time percentage (0-100) since last call using run-time counters.
 * Uses static state; first call or invalid diff returns 0.
 */
uint32_t calculate_idle_percentage(void);
#endif

#endif /* RTOS_STATS_H */
