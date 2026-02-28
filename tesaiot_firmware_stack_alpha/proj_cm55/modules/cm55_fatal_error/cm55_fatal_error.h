/*******************************************************************************
 * File Name        : cm55_fatal_error.h
 *
 * Description      : Fatal error handler for CM55: disables IRQ, asserts,
 *                    prints message, blinks user LED in infinite loop.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 *
 *******************************************************************************/

#ifndef CM55_FATAL_ERROR_H
#define CM55_FATAL_ERROR_H

#include <stdbool.h>

/**
 * Fatal error handler: disables IRQ, asserts, prints formatted message to
 * stdout (printf-style format and variadic args), then blinks user LED in an
 * infinite loop. Does not return. format may be NULL for a generic message.
 */
void cm55_handle_fatal_error(const char *format, ...);

#endif /* CM55_FATAL_ERROR_H */
