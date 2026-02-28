/*******************************************************************************
 * File Name        : ipc_log_transport.c
 *
 * Description      : IPC log transport task; prints log queue to CM33 UART.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM33 (non-secure)
 *
 *******************************************************************************/

#include "ipc_log_transport.h"

#ifndef DISABLE_IPC_LOGGING

#include "ipc_log.h"
#include "task.h"
#include <stdint.h>
#include <stdio.h>

#define LOG_TASK_STACK_SIZE (2048U)   /* Stack size for transport task. */
#define LOG_TASK_PRIORITY (configMAX_PRIORITIES - 1)  /* Highest priority. */

/**
 * Worker task: receives from log_queue and prints to CM33 UART.
 */
static void log_ipc_dispatch_worker(void *pvParameters)
{
  (void)pvParameters;
  log_msg_t msg;

  while (true)
  {
    if (pdPASS == xQueueReceive(log_queue, &msg, portMAX_DELAY))
    {
      (void)fputs(msg.message, stdout);
      (void)fflush(stdout);
    }
  }
}

/**
 * Initializes IPC log queue and spawns the transport task. Returns true on success.
 */
bool ipc_log_transport_init(void)
{
  if (!ipc_log_init())
  {
    return false;
  }
  if (pdPASS != xTaskCreate(log_ipc_dispatch_worker, "Log IPC Worker",
                            LOG_TASK_STACK_SIZE, NULL, LOG_TASK_PRIORITY,
                            NULL))
  {
    return false;
  }
  return true;
}

#endif
