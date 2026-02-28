/*******************************************************************************
 * File Name        : ipc_log.c
 *
 * Description      : IPC logging queue and printf-style API for CM33.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM33 (non-secure)
 *
 *******************************************************************************/

#include "ipc_log.h"

#ifndef DISABLE_IPC_LOGGING

#include "FreeRTOS.h"
#include "task.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

QueueHandle_t log_queue = NULL;

/**
 * Creates the log queue if not already created. Safe to call multiple times. Returns true on success.
 */
bool ipc_log_init(void)
{
  if (NULL == log_queue)
  {
    log_queue = xQueueCreate(LOG_QUEUE_LENGTH, sizeof(log_msg_t));
  }
  return (NULL != log_queue);
}

/**
 * Queues a printf-style message for transport to CM55. Blocks if queue full. No-op if queue not initialized.
 */
void ipc_log_printf(const char *format, ...)
{
  if (NULL == log_queue)
  {
    return;
  }
  log_msg_t msg;
  va_list args;
  va_start(args, format);
  vsnprintf(msg.message, LOG_MESSAGE_SIZE, format, args);
  va_end(args);
  (void)xQueueSend(log_queue, &msg, portMAX_DELAY);
}

void ipc_log_flush(unsigned int timeout_ms)
{
  if (NULL == log_queue)
  {
    return;
  }
  unsigned int elapsed = 0U;
  while ((elapsed < timeout_ms) && (0U != uxQueueMessagesWaiting(log_queue)))
  {
    vTaskDelay(pdMS_TO_TICKS(10U));
    elapsed += 10U;
  }
}

#endif
