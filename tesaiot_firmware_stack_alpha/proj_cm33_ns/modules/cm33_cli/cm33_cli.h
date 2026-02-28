/*******************************************************************************
 * File Name        : cm33_cli.h
 *
 * Description      : API and configuration for the CM33 CLI module (init, start,
 *                    stop; line length, history count, task stack, priority).
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM33 (non-secure)
 *
 *******************************************************************************/

#ifndef CM33_CLI_H
#define CM33_CLI_H

#include <stdbool.h>

#define CM33_CLI_LINE_MAX      128U       /* Maximum length of one input line in characters. */
#define CM33_CLI_HISTORY_COUNT  8U        /* Number of completed lines kept in history. */
#define CM33_CLI_STACK_SIZE     1024U     /* Stack size in words for the CLI FreeRTOS task. */
#define CM33_CLI_TASK_PRIORITY_OFFSET  4  /* CLI task priority = tskIDLE_PRIORITY + this value. */

/**
 * Initializes the CLI module. Call once after UART/retarget is ready. Returns true.
 */
bool cm33_cli_init(void);

/**
 * Creates the CLI FreeRTOS task; call after IPC (and optionally Wi-Fi) is up. Returns true if
 * xTaskCreate succeeded.
 */
bool cm33_cli_start(void);

/**
 * Deletes the CLI task and clears the task handle. Idempotent if already stopped.
 */
void cm33_cli_stop(void);

#endif
