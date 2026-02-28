/*******************************************************************************
 * File Name        : error_handler.c
 *
 * Description      : Implementation of the CM33 centralized error handler.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM33 (non-secure)
 *
 *******************************************************************************/

#include "error_handler.h"
#include "cy_pdl.h"
#include "cybsp.h"
#include "ipc_log.h"
#include <stdio.h>

/**
 * Centrally handles application errors.
 */
void handle_error(const char *message) {
  /* Log the error first so it is visible before UART may stop (interrupts off).
   * When printf is redirected to IPC log, ipc_log_flush() drains the queue so
   * the transport task sends the message to CM55 before we disable interrupts.
   */
  if (message != NULL) {
    printf("\n[ERROR] %s\n", message);
  } else {
    printf("\n[ERROR] Unspecified fatal error occurred.\n");
  }
  fflush(stdout);
  ipc_log_flush(500U);
  Cy_SysLib_Delay(200);

  __disable_irq();

  /* Infinite loop: Blink the User LED to indicate failure */
  while (true) {
    Cy_GPIO_Inv(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);
    Cy_SysLib_Delay(100);
  }
}
