#include "cm33_ipc_pipe.h"

#include "cy_syslib.h"
#include "cybsp.h"
#include "ipc_log.h"
#include "udp_server_app.h"
#include "user_buttons.h"
#include "wifi_manager.h"
#include <queue.h>
#include <stdio.h>
#include <string.h>

#define IPC_TASK_STACK (2048U)
#define IPC_TASK_PRIO (3U)
#define CM33_APP_DELAY_MS (50U)
#define RESET_VAL (0U)
#define IPC_SEND_QUEUE_LEN (16U)
#define IPC_RECV_RING_LEN (16U)

static TaskHandle_t ipc_task_handle;
CY_SECTION_SHAREDMEM static ipc_msg_t cm33_msg_data;
static int ipc_counter = 0;
static QueueHandle_t s_ipc_send_queue = NULL;
static ipc_msg_t s_ipc_recv_ring[IPC_RECV_RING_LEN];
static volatile uint32_t s_ipc_recv_head = 0U;
static volatile uint32_t s_ipc_recv_tail = 0U;
static volatile uint32_t s_ipc_recv_count = 0U;
static volatile uint32_t s_ipc_recv_total = 0U;

static bool internal_send_message(uint32_t cmd, uint32_t value, const void *data, uint32_t data_size)
{
  if (NULL == s_ipc_send_queue)
  {
    return false;
  }

  ipc_msg_t msg;
  (void)memset(&msg, 0, sizeof(msg));
  msg.cmd = cmd;
  msg.value = value;

  if ((NULL != data) && (data_size > 0U))
  {
    uint32_t copy_size = data_size;
    if (copy_size > IPC_DATA_MAX_LEN)
    {
      copy_size = IPC_DATA_MAX_LEN;
    }
    (void)memcpy(msg.data, data, copy_size);
  }

  return (pdPASS == xQueueSend(s_ipc_send_queue, &msg, 0U));
}

static bool internal_send_message_ticks(uint32_t cmd, uint32_t value, const void *data,
                                        uint32_t data_size, TickType_t timeout_ticks)
{
  if (NULL == s_ipc_send_queue)
  {
    return false;
  }

  ipc_msg_t msg;
  (void)memset(&msg, 0, sizeof(msg));
  msg.cmd = cmd;
  msg.value = value;

  if ((NULL != data) && (data_size > 0U))
  {
    uint32_t copy_size = data_size;
    if (copy_size > IPC_DATA_MAX_LEN)
    {
      copy_size = IPC_DATA_MAX_LEN;
    }
    (void)memcpy(msg.data, data, copy_size);
  }

  return (pdPASS == xQueueSend(s_ipc_send_queue, &msg, timeout_ticks));
}

static void cm33_msg_callback(uint32_t *msg_data)
{
  uint32_t next_head;

  if (NULL == msg_data)
  {
    return;
  }

  if (s_ipc_recv_count >= IPC_RECV_RING_LEN)
  {
    return;
  }

  (void)memcpy(&s_ipc_recv_ring[s_ipc_recv_head], msg_data, sizeof(ipc_msg_t));
  next_head = s_ipc_recv_head + 1U;
  if (next_head >= IPC_RECV_RING_LEN)
  {
    next_head = 0U;
  }
  s_ipc_recv_head = next_head;
  s_ipc_recv_count++;
  s_ipc_recv_total++;
}

static void ipc_button_event_handler(user_buttons_t switch_handle, const button_event_t *evt)
{
  (void)switch_handle;

  if (NULL != evt)
  {
    const char *action = evt->is_pressed ? "PRESSED" : "RELEASED";
    (void)printf("[CM33] BUTTON_%lu %s\n", (unsigned long)evt->button_id, action);
  }

  if ((NULL != evt) && (evt->is_pressed) && ((uint32_t)BUTTON_ID_0 == evt->button_id))
  {
    udp_server_app_send_led_toggle();
  }

  (void)cm33_ipc_send_button_event(evt);
}

static void cm33_wifi_manager_event_callback(wifi_manager_event_t event, const void *data, uint32_t count, void *user_data)
{
  (void)user_data;

  if ((WIFI_MANAGER_EVENT_SCAN_RESULT == event) && (NULL != data))
  {
    (void)cm33_ipc_send_wifi_scan_results((const wifi_info_t *)data, count);
  }
  else if ((WIFI_MANAGER_EVENT_SCAN_COMPLETE == event) && (NULL != data))
  {
    (void)cm33_ipc_send_wifi_scan_complete((const ipc_wifi_scan_complete_t *)data);
  }
  else if ((WIFI_MANAGER_EVENT_STATUS == event) && (NULL != data))
  {
    const ipc_wifi_status_t *status = (const ipc_wifi_status_t *)data;
    if ((uint8_t)IPC_WIFI_LINK_CONNECTED == status->state)
    {
      udp_server_app_start();
    }
    else if ((uint8_t)IPC_WIFI_LINK_DISCONNECTED == status->state)
    {
      udp_server_app_stop();
    }
    (void)cm33_ipc_send_wifi_status(status);
  }
}

static void ipc_process_incoming(const ipc_msg_t *msg)
{
  if (NULL == msg)
  {
    return;
  }

  if (IPC_CMD_WIFI_SCAN_REQ == msg->cmd)
  {
    ipc_wifi_scan_request_t req;
    (void)memset(&req, 0, sizeof(req));
    (void)memcpy(&req, msg->data, sizeof(req));
    (void)wifi_manager_request_scan(&req);
  }
  else if (IPC_CMD_WIFI_CONNECT_REQ == msg->cmd)
  {
    ipc_wifi_connect_request_t req;
    (void)memset(&req, 0, sizeof(req));
    (void)memcpy(&req, msg->data, sizeof(req));
    req.ssid[sizeof(req.ssid) - 1U] = '\0';
    req.password[sizeof(req.password) - 1U] = '\0';
    (void)wifi_manager_request_connect(&req);
  }
  else if (IPC_CMD_WIFI_DISCONNECT_REQ == msg->cmd)
  {
    (void)wifi_manager_request_disconnect();
  }
  else if (IPC_CMD_WIFI_STATUS_REQ == msg->cmd)
  {
    (void)wifi_manager_request_status();
  }
  else if (IPC_CMD_PRINT == msg->cmd)
  {
    char buf[IPC_DATA_MAX_LEN];
    (void)memcpy(buf, msg->data, IPC_DATA_MAX_LEN - 1UL);
    buf[IPC_DATA_MAX_LEN - 1UL] = '\0';
    (void)printf("%s", buf);
  }
}

static void ipc_task(void *arg)
{
  ipc_msg_t send_msg;
  ipc_msg_t recv_msg;
  TickType_t last_heartbeat = xTaskGetTickCount();
  bool has_recv_msg;
  uint32_t intr_state;

  (void)arg;
  vTaskDelay(pdMS_TO_TICKS(1000U));

  while (true)
  {
    if ((NULL != s_ipc_send_queue) && (pdPASS == xQueueReceive(s_ipc_send_queue, &send_msg, pdMS_TO_TICKS(5U))))
    {
      cm33_msg_data.client_id = CM55_IPC_PIPE_CLIENT_ID;
      cm33_msg_data.intr_mask = CY_IPC_CYPIPE_INTR_MASK_EP1;
      cm33_msg_data.cmd = send_msg.cmd;
      cm33_msg_data.value = send_msg.value;
      (void)memcpy(cm33_msg_data.data, send_msg.data, IPC_DATA_MAX_LEN);

      (void)Cy_IPC_Pipe_SendMessage(CM55_IPC_PIPE_EP_ADDR, CM33_IPC_PIPE_EP_ADDR, (void *)&cm33_msg_data, 0U);
    }

    has_recv_msg = false;
    intr_state = Cy_SysLib_EnterCriticalSection();
    if (s_ipc_recv_count > 0U)
    {
      recv_msg = s_ipc_recv_ring[s_ipc_recv_tail];
      s_ipc_recv_tail++;
      if (s_ipc_recv_tail >= IPC_RECV_RING_LEN)
      {
        s_ipc_recv_tail = 0U;
      }
      s_ipc_recv_count--;
      has_recv_msg = true;
    }
    Cy_SysLib_ExitCriticalSection(intr_state);

    if (has_recv_msg)
    {
      ipc_process_incoming(&recv_msg);
    }

    udp_server_app_process();

    if ((xTaskGetTickCount() - last_heartbeat) >= pdMS_TO_TICKS(500U))
    {
      ipc_counter++;
      last_heartbeat = xTaskGetTickCount();

      cm33_msg_data.client_id = CM55_IPC_PIPE_CLIENT_ID;
      cm33_msg_data.intr_mask = CY_IPC_CYPIPE_INTR_MASK_EP1;
      cm33_msg_data.cmd = RESET_VAL;
      cm33_msg_data.value = (uint32_t)ipc_counter;
      (void)memset(cm33_msg_data.data, 0, IPC_DATA_MAX_LEN);

      if (CY_IPC_PIPE_SUCCESS ==
          Cy_IPC_Pipe_SendMessage(CM55_IPC_PIPE_EP_ADDR, CM33_IPC_PIPE_EP_ADDR, (void *)&cm33_msg_data, 0U))
      {
        Cy_GPIO_Inv(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);
      }
    }
  }
}

bool cm33_ipc_pipe_start(void)
{
  cy_en_ipc_pipe_status_t pipe_status;

  cm33_ipc_communication_setup();
  Cy_SysLib_Delay(CM33_APP_DELAY_MS);

  s_ipc_send_queue = xQueueCreate(IPC_SEND_QUEUE_LEN, sizeof(ipc_msg_t));
  if (NULL == s_ipc_send_queue)
  {
    return false;
  }

  pipe_status = Cy_IPC_Pipe_RegisterCallback(CM33_IPC_PIPE_EP_ADDR, &cm33_msg_callback, (uint32_t)CM33_IPC_PIPE_CLIENT_ID);
  if (CY_IPC_PIPE_SUCCESS != pipe_status)
  {
    return false;
  }

  (void)user_button_on_changed(BUTTON_ID_0, ipc_button_event_handler);
  (void)user_button_on_changed(BUTTON_ID_1, ipc_button_event_handler);
  (void)wifi_manager_set_event_callback(cm33_wifi_manager_event_callback, NULL);

  if (pdPASS != xTaskCreate(ipc_task, "IPC Task", IPC_TASK_STACK, NULL, IPC_TASK_PRIO, &ipc_task_handle))
  {
    return false;
  }

  return true;
}

bool cm33_ipc_send_gyro_data(const gyro_data_t *data, uint32_t sequence)
{
  if (NULL == data)
  {
    return false;
  }
  return internal_send_message(IPC_CMD_GYRO, sequence, data, sizeof(gyro_data_t));
}

bool cm33_ipc_send_button_event(const button_event_t *event)
{
  if (NULL == event)
  {
    return false;
  }
  return internal_send_message(IPC_CMD_BUTTON_EVENT, 0U, event, sizeof(button_event_t));
}

bool cm33_ipc_send_touch(int16_t x, int16_t y, uint8_t pressed)
{
  ipc_touch_event_t evt;
  evt.x = x;
  evt.y = y;
  evt.pressed = pressed;
  return internal_send_message_ticks(IPC_CMD_TOUCH, 0U, &evt, sizeof(ipc_touch_event_t), pdMS_TO_TICKS(2U));
}

bool cm33_ipc_send_wifi_scan_results(const wifi_info_t *results, uint32_t count)
{
  if ((NULL == results) || (0U == count))
  {
    return false;
  }

  for (uint32_t i = 0U; i < count; i++)
  {
    uint32_t value = (count << IPC_WIFI_SCAN_VALUE_COUNT_SHIFT) | (i & IPC_WIFI_SCAN_VALUE_INDEX_MASK);
    if (false == internal_send_message(IPC_EVT_WIFI_SCAN_RESULT, value, &results[i], sizeof(wifi_info_t)))
    {
      return false;
    }
    vTaskDelay(pdMS_TO_TICKS(2U));
  }

  return true;
}

bool cm33_ipc_send_wifi_scan_complete(const ipc_wifi_scan_complete_t *scan_complete)
{
  if (NULL == scan_complete)
  {
    return false;
  }
  return internal_send_message(IPC_EVT_WIFI_SCAN_COMPLETE, 0U, scan_complete, sizeof(ipc_wifi_scan_complete_t));
}

bool cm33_ipc_send_wifi_status(const ipc_wifi_status_t *status)
{
  if (NULL == status)
  {
    return false;
  }
  return internal_send_message(IPC_EVT_WIFI_STATUS, 0U, status, sizeof(ipc_wifi_status_t));
}

bool cm33_ipc_send_ping(void)
{
  return internal_send_message(IPC_CMD_PING, 0U, NULL, 0U);
}

bool cm33_ipc_send_cli_message(const char *text)
{
  if (NULL == text)
  {
    return false;
  }
  {
    size_t len = strlen(text);
    if (len >= IPC_DATA_MAX_LEN)
    {
      len = IPC_DATA_MAX_LEN - 1U;
    }
    return internal_send_message(IPC_CMD_CLI_MSG, 0U, text, (uint32_t)len);
  }
}

uint32_t cm33_ipc_get_recv_pending(void)
{
  return s_ipc_recv_count;
}

uint32_t cm33_ipc_get_recv_total(void)
{
  return s_ipc_recv_total;
}

uint32_t cm33_ipc_get_send_queue_used(void)
{
  if (NULL == s_ipc_send_queue)
  {
    return 0U;
  }
  return (uint32_t)uxQueueMessagesWaiting(s_ipc_send_queue);
}

uint32_t cm33_ipc_get_send_queue_capacity(void)
{
  return IPC_SEND_QUEUE_LEN;
}
