/*******************************************************************************
 * File Name        : cm55_system.h
 *
 * Description      : Function prototypes and definitions for the CM55 system
 *                    initialization (BSP, RTC, retarget-io, LPTimer tickless idle).
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM55
 *
 *******************************************************************************/

#ifndef CM55_SYSTEM_H_
#define CM55_SYSTEM_H_

#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
 * Tick Hook Callback Types
 *******************************************************************************/

typedef struct
{
  uint32_t tick_count;  /* Current RTOS tick from xTaskGetTickCountFromISR. */
  void *user_data;      /* Pointer passed to cm55_system_register_tick_callback. */
} system_tick_hook_params_t;

typedef void (*system_tick_hook_cb_t)(const system_tick_hook_params_t *params);

/*******************************************************************************
 * Public API
 *******************************************************************************/

/**
 * Performs BSP, CLIB, LPTimer, retarget I/O, enables IRQ. Returns true on
 * success, false if cybsp_init fails. On LPTimer/CLIB failure does not return.
 */
bool cm55_system_init(void);

/**
 * Registers the tick hook callback and user_data; invoked each RTOS tick.
 * NULL callback allowed. Overwrites any previous registration.
 */
void cm55_system_register_tick_callback(system_tick_hook_cb_t callback, void *user_data);

#endif /* CM55_SYSTEM_H_ */
