/*******************************************************************************
 * File Name        : cm55_fatal_error.c
 *
 * Description      : Implementation of CM55 fatal error handler: disables IRQ,
 *                    asserts, prints message, blinks user LED indefinitely.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM55
 *
 *******************************************************************************/

#include "cm55_fatal_error.h"
#include "cy_pdl.h"
#include "cybsp.h"
#include <stdarg.h>
#include <stdio.h>

/*******************************************************************************
 * Public API
 *******************************************************************************/

/**
 * Fatal error handler: disables IRQ, asserts, prints formatted message to
 * stdout (printf-style), then blinks user LED in an infinite loop. Does not
 * return. format may be NULL for a generic message.
 */
void cm55_handle_fatal_error(const char *format, ...)
{
  va_list args;

  /* Print the error first so it is visible before UART may stop (IRQ off). */
  (void)printf("\n[CM55 ERROR] ");
  if (NULL != format)
  {
    va_start(args, format);
    (void)vprintf(format, args);
    va_end(args);
  }
  else
  {
    (void)printf("Unspecified fatal error occurred.");
  }
  (void)printf("\n");
  (void)fflush(stdout);
  Cy_SysLib_Delay(200);

  __disable_irq();

  /* Blink user LED indefinitely so the failure is visible. */
  while (true)
  {
    Cy_GPIO_Inv(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);
    Cy_SysLib_Delay(20);
  }
}
