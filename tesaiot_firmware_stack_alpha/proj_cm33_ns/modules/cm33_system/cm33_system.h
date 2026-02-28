/*******************************************************************************
 * File Name        : cm33_system.h
 *
 * Description      : Function prototypes and definitions for the CM33 system
 *                    initialization (BSP, RTC, retarget-io, LPTimer, CM55).
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM33 (non-secure)
 *
 *******************************************************************************/

#ifndef CM33_SYSTEM_H_
#define CM33_SYSTEM_H_

#include <stdbool.h>
#include <stdint.h>
#include "mtb_hal_rtc.h"

/*******************************************************************************
 * Tick Hook Callback Types
 *******************************************************************************/

typedef struct
{
  uint32_t tick_count; /**< FreeRTOS tick count when hook was invoked */
  void *user_data;     /**< User context passed to system_register_tick_hook */
} system_tick_hook_params_t;

typedef void (*system_tick_hook_cb_t)(const system_tick_hook_params_t *params);

/*******************************************************************************
 * Public API
 *******************************************************************************/

/** Initializes BSP, RTC, retarget-io, LPTimer (tickless idle). Returns true on success. */
bool cm33_system_init(void);

/** Enables the CM55 core. Call after IPC pipe is started if boot order matters. */
void cm33_system_enable_cm55(void);

/** Registers a callback invoked on each FreeRTOS tick. Call after cm33_system_init(). */
void system_register_tick_hook(system_tick_hook_cb_t callback, void *user_data);

/** Returns the RTC HAL object used for CLIB support. Valid after cm33_system_init(). */
mtb_hal_rtc_t *cm33_system_get_rtc(void);

#endif /* CM33_SYSTEM_H_ */
