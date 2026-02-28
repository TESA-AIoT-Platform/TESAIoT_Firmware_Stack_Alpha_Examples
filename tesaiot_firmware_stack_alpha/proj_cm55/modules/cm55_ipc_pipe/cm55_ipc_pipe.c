/*******************************************************************************
 * File Name        : cm55_ipc_pipe.c
 *
 * Description      : CM55 IPC pipe: send queue, sender task, pipe init/start
 *                    and callback registration; pushes requests to CM33.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM55
 *
 *******************************************************************************/

#include "cm55_ipc_pipe.h"
#include "cm55_fatal_error.h"

#include "cy_syslib.h"
#include "ipc_communication.h"

#include <queue.h>
#include <stdbool.h>
#include <string.h>

#define RESET_VAL (0U)

static TaskHandle_t cm55_ipc_sender_task_handle;
static QueueHandle_t s_ipc_send_queue = NULL;
static cm55_ipc_data_received_cb_t s_data_received_cb = NULL;
CY_SECTION_SHAREDMEM static ipc_msg_t cm55_msg_data;
static cm55_ipc_pipe_config_t s_config = {
    .task_stack = CM55_IPC_PIPE_TASK_STACK_DEFAULT,
    .task_prio = CM55_IPC_PIPE_TASK_PRIO_DEFAULT,
    .send_queue_len = CM55_IPC_PIPE_SEND_QUEUE_LEN_DEFAULT,
    .startup_delay_ms = CM55_IPC_PIPE_STARTUP_DELAY_MS_DEFAULT,
};

/**
 * FreeRTOS task that reads messages from send queue and sends via IPC pipe to CM33.
 */
static void cm55_ipc_sender_task(void *arg)
{
  (void)arg;
  ipc_msg_t send_msg;
  cy_en_ipc_pipe_status_t status;
  int retries;
  while (true)
  {
    if (xQueueReceive(s_ipc_send_queue, &send_msg, portMAX_DELAY) == pdPASS)
    {
      cm55_msg_data.client_id = CM33_IPC_PIPE_CLIENT_ID;
      cm55_msg_data.intr_mask = CY_IPC_CYPIPE_INTR_MASK_EP2;
      cm55_msg_data.cmd = send_msg.cmd;
      cm55_msg_data.value = send_msg.value;
      (void)memcpy(cm55_msg_data.data, send_msg.data, IPC_DATA_MAX_LEN);

      retries = 5;
      do
      {
        status =
            Cy_IPC_Pipe_SendMessage(CM33_IPC_PIPE_EP_ADDR, CM55_IPC_PIPE_EP_ADDR, (void *)&cm55_msg_data, RESET_VAL);
        if (CY_IPC_PIPE_SUCCESS == status)
        {
          break;
        }
        vTaskDelay(pdMS_TO_TICKS(5U));
      } while (--retries > 0);

      vTaskDelay(pdMS_TO_TICKS(10U));
    }
  }
}

/**
 * Queues an IPC request (cmd + optional data) for the sender task to send. data may be NULL when
 * data_len 0; data_len capped to IPC_DATA_MAX_LEN. Returns false if send queue not initialized.
 */
bool cm55_ipc_pipe_push_request(uint32_t cmd, const void *data, uint32_t data_len)
{
  ipc_msg_t msg;

  if (NULL == s_ipc_send_queue)
  {
    return false;
  }

  (void)memset(&msg, 0, sizeof(msg));
  msg.cmd = cmd;
  msg.value = RESET_VAL;
  if ((NULL != data) && (data_len > 0U))
  {
    uint32_t copy_len = (data_len > IPC_DATA_MAX_LEN) ? IPC_DATA_MAX_LEN : data_len;
    (void)memcpy(msg.data, data, copy_len);
  }
  return (xQueueSend(s_ipc_send_queue, &msg, 0U) == pdPASS);
}

/**
 * No-op callback used when no data-received callback is registered.
 */
static void cm55_ipc_data_received_noop(uint32_t *msg_data)
{
  (void)msg_data;
}

/**
 * Applies pipe configuration (task stack, prio, queue length, startup delay). config NULL leaves
 * defaults unchanged.
 */
void cm55_ipc_pipe_init(const cm55_ipc_pipe_config_t *config)
{
  if (NULL != config)
  {
    s_config.task_stack = config->task_stack;
    s_config.task_prio = config->task_prio;
    s_config.send_queue_len = config->send_queue_len;
    s_config.startup_delay_ms = config->startup_delay_ms;
  }
}

/**
 * Set callback for raw received data; used when pipe is started without a callback argument. NULL
 * allowed.
 */
void cm55_ipc_pipe_set_data_received_callback(cm55_ipc_data_received_cb_t cb)
{
  s_data_received_cb = cb;
}

/**
 * Start pipe and RX path: creates send queue, runs communication setup, waits startup_delay_ms,
 * registers callback, creates sender task. On failure may call cm55_handle_fatal_error. cb NULL uses set
 * callback or noop. Returns false on queue or register or task create failure.
 */
bool cm55_ipc_pipe_start(cm55_ipc_data_received_cb_t cb)
{
  cm55_ipc_data_received_cb_t reg_cb = (NULL != cb) ? cb : s_data_received_cb;
  if (NULL == reg_cb)
  {
    reg_cb = cm55_ipc_data_received_noop;
  }

  s_ipc_send_queue = xQueueCreate(s_config.send_queue_len, sizeof(ipc_msg_t));
  if (NULL == s_ipc_send_queue)
  {
    return false;
  }

  cm55_ipc_communication_setup();

  Cy_SysLib_Delay(s_config.startup_delay_ms);

  cy_en_ipc_pipe_status_t pipe_status =
      Cy_IPC_Pipe_RegisterCallback(CM55_IPC_PIPE_EP_ADDR, reg_cb, (uint32_t)CM55_IPC_PIPE_CLIENT_ID);
  if (CY_IPC_PIPE_SUCCESS != pipe_status)
  {
    vQueueDelete(s_ipc_send_queue);
    s_ipc_send_queue = NULL;
    cm55_handle_fatal_error("IPC pipe callback registration failed: %d", pipe_status);
    return false;
  }

  if (pdPASS != xTaskCreate(cm55_ipc_sender_task, "IPC Sender", s_config.task_stack, NULL,
                            s_config.task_prio, &cm55_ipc_sender_task_handle))
  {
    vQueueDelete(s_ipc_send_queue);
    s_ipc_send_queue = NULL;
    cm55_handle_fatal_error("IPC sender task create failed");
    return false;
  }

  return true;
}
