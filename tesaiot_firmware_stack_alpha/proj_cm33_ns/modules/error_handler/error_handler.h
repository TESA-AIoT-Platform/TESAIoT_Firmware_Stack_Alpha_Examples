/*******************************************************************************
 * File Name        : error_handler.h
 *
 * Description      : Public interface for the CM33 centralized error handler.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM33 (non-secure)
 *
 *******************************************************************************/

#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <stdbool.h>

/**
 * Centrally handles application errors by disabling interrupts,
 * logging the error, and blinking the user LED in an infinite loop.
 * Does not return. message may be NULL for a generic error log.
 */
void handle_error(const char *message);

#endif /* ERROR_HANDLER_H */
