#include "cm55_ipc_app.h"
#include "cm55_ipc_pipe.h"

#include "ipc_communication.h"
#include "queue.h"
#include "task.h"
#include "user_buttons_types.h"
#if defined(TOUCH_VIA_IPC)
#include "lv_port_indev.h"
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define IPC_RECEIVER_TASK_STACK (1024U)
#define IPC_RECEIVER_TASK_PRIO (2U)
#define CM55_LOG_QUEUE_LENGTH (64U)
#define IPC_WORK_QUEUE_LEN (16U)
#define CM55_IPC_PIPE_WIFI_LIST_MAX (32U)
#define CM55_WIFI_DEBUG_LINE_MAX (96U)
#define CM55_WIFI_DEBUG_LINE_COUNT (48U)

typedef struct
{
  uint8_t event_type;
  uint8_t reserved;
  uint16_t value;
} ipc_work_item_t;

typedef struct
{
  char buf[IPC_DATA_MAX_LEN];
} app_log_msg_t;

static QueueHandle_t s_ipc_work_queue = NULL;
static QueueHandle_t s_log_queue = NULL;
static bool s_draining_log = false;
static char s_log_text_buf[IPC_DATA_MAX_LEN];

static wifi_info_t s_wifi_list[CM55_IPC_PIPE_WIFI_LIST_MAX];
static volatile uint32_t s_wifi_list_count = 0U;
static volatile bool s_wifi_list_ready = false;
static ipc_wifi_status_t s_wifi_status;

static gyro_data_t s_gyro_data;
static volatile uint32_t s_gyro_sequence = 0U;

static volatile uint32_t s_btn_press_count[BUTTON_ID_MAX];
static volatile bool s_btn_is_pressed[BUTTON_ID_MAX];
static char s_wifi_debug_lines[CM55_WIFI_DEBUG_LINE_COUNT][CM55_WIFI_DEBUG_LINE_MAX];
static uint32_t s_wifi_debug_head = 0U;
static uint32_t s_wifi_debug_count = 0U;
static volatile uint32_t s_wifi_debug_sequence = 0U;

static ipc_wifi_status_t s_wifi_status_last_printed;
static bool s_wifi_status_printed_once = false;

static void app_wifi_debug_append(const char *tag, const char *text)
{
  char line[CM55_WIFI_DEBUG_LINE_MAX];
  uint32_t idx;

  if ((NULL == tag) || (NULL == text))
  {
    return;
  }

  (void)snprintf(line, sizeof(line), "[%s] %s", tag, text);
  idx = s_wifi_debug_head;
  (void)strncpy(s_wifi_debug_lines[idx], line, CM55_WIFI_DEBUG_LINE_MAX - 1U);
  s_wifi_debug_lines[idx][CM55_WIFI_DEBUG_LINE_MAX - 1U] = '\0';

  s_wifi_debug_head++;
  if (s_wifi_debug_head >= CM55_WIFI_DEBUG_LINE_COUNT)
  {
    s_wifi_debug_head = 0U;
  }
  if (s_wifi_debug_count < CM55_WIFI_DEBUG_LINE_COUNT)
  {
    s_wifi_debug_count++;
  }
  s_wifi_debug_sequence++;
}

static void app_push_work_item_from_isr(uint8_t event_type, uint16_t value, BaseType_t *pxHigherPriorityTaskWoken)
{
  if (NULL != s_ipc_work_queue)
  {
    ipc_work_item_t work_item;
    work_item.event_type = event_type;
    work_item.reserved = 0U;
    work_item.value = value;
    (void)xQueueSendFromISR(s_ipc_work_queue, &work_item, pxHigherPriorityTaskWoken);
  }
}

static bool app_log_push_from_isr(const char *data, uint32_t data_len, BaseType_t *pxHigherPriorityTaskWoken)
{
  app_log_msg_t log_item;
  uint32_t copy_len;

  if ((NULL == s_log_queue) || (NULL == data) || (NULL == pxHigherPriorityTaskWoken))
  {
    return false;
  }

  (void)memset(&log_item, 0, sizeof(log_item));
  copy_len = (data_len > (IPC_DATA_MAX_LEN - 1U)) ? (IPC_DATA_MAX_LEN - 1U) : data_len;
  (void)memcpy(log_item.buf, data, copy_len);
  log_item.buf[IPC_DATA_MAX_LEN - 1U] = '\0';
  return (pdPASS == xQueueSendFromISR(s_log_queue, &log_item, pxHigherPriorityTaskWoken));
}

static bool app_receive_next(cm55_ipc_event_t *event, cm55_ipc_event_payload_t *payload, TickType_t timeout_ticks)
{
  ipc_work_item_t work_item;
  app_log_msg_t log_item;

  if ((NULL == event) || (NULL == payload))
  {
    return false;
  }

  if (s_draining_log && (NULL != s_log_queue))
  {
    if (pdPASS == xQueueReceive(s_log_queue, &log_item, 0U))
    {
      (void)memcpy(s_log_text_buf, log_item.buf, sizeof(s_log_text_buf));
      s_log_text_buf[sizeof(s_log_text_buf) - 1U] = '\0';
      payload->log.text = s_log_text_buf;
      *event = CM55_IPC_EVENT_LOG;
      return true;
    }
    s_draining_log = false;
  }

  if (pdPASS != xQueueReceive(s_ipc_work_queue, &work_item, timeout_ticks))
  {
    return false;
  }

  switch ((cm55_ipc_event_t)work_item.event_type)
  {
  case CM55_IPC_EVENT_LOG:
    if ((NULL != s_log_queue) && (pdPASS == xQueueReceive(s_log_queue, &log_item, 0U)))
    {
      (void)memcpy(s_log_text_buf, log_item.buf, sizeof(s_log_text_buf));
      s_log_text_buf[sizeof(s_log_text_buf) - 1U] = '\0';
      payload->log.text = s_log_text_buf;
      *event = CM55_IPC_EVENT_LOG;
      s_draining_log = true;
      return true;
    }
    return false;
  case CM55_IPC_EVENT_GYRO:
    payload->gyro.data = &s_gyro_data;
    payload->gyro.sequence = s_gyro_sequence;
    *event = CM55_IPC_EVENT_GYRO;
    return true;
  case CM55_IPC_EVENT_WIFI_STATUS:
    payload->wifi_status.status = &s_wifi_status;
    *event = CM55_IPC_EVENT_WIFI_STATUS;
    return true;
  case CM55_IPC_EVENT_WIFI_COMPLETE:
    payload->wifi_complete.list = s_wifi_list;
    payload->wifi_complete.count = s_wifi_list_count;
    *event = CM55_IPC_EVENT_WIFI_COMPLETE;
    return true;
  case CM55_IPC_EVENT_BUTTON:
    if (work_item.value < BUTTON_ID_MAX)
    {
      payload->button.button_id = work_item.value;
      payload->button.press_count = s_btn_press_count[work_item.value];
      payload->button.is_pressed = s_btn_is_pressed[work_item.value];
      *event = CM55_IPC_EVENT_BUTTON;
      return true;
    }
    return false;
  default:
    return false;
  }
}

static void cm55_ipc_app_data_received_cb(uint32_t *msg_data)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  ipc_msg_t *msg;

  if (NULL == msg_data)
  {
    return;
  }

  msg = (ipc_msg_t *)msg_data;

  if (IPC_EVT_WIFI_SCAN_RESULT == msg->cmd)
  {
    uint32_t packed = msg->value;
    uint32_t total_count = (packed >> IPC_WIFI_SCAN_VALUE_COUNT_SHIFT) & IPC_WIFI_SCAN_VALUE_INDEX_MASK;
    uint32_t index = packed & IPC_WIFI_SCAN_VALUE_INDEX_MASK;
    if ((index < CM55_IPC_PIPE_WIFI_LIST_MAX) && (index < total_count))
    {
      (void)memcpy(&s_wifi_list[index], msg->data, sizeof(wifi_info_t));
    }
  }
  else if (IPC_EVT_WIFI_SCAN_COMPLETE == msg->cmd)
  {
    ipc_wifi_scan_complete_t complete;
    (void)memset(&complete, 0, sizeof(complete));
    (void)memcpy(&complete, msg->data, sizeof(complete));
    s_wifi_list_count = complete.total_count;
    s_wifi_list_ready = true;
    app_push_work_item_from_isr((uint8_t)CM55_IPC_EVENT_WIFI_COMPLETE, complete.total_count, &xHigherPriorityTaskWoken);
  }
  else if (IPC_EVT_WIFI_STATUS == msg->cmd)
  {
    (void)memcpy(&s_wifi_status, msg->data, sizeof(ipc_wifi_status_t));
    app_push_work_item_from_isr((uint8_t)CM55_IPC_EVENT_WIFI_STATUS, 0U, &xHigherPriorityTaskWoken);
  }
  else if (IPC_CMD_BUTTON_EVENT == msg->cmd)
  {
    button_event_t evt;
    (void)memcpy(&evt, msg->data, sizeof(evt));
    if (evt.button_id < BUTTON_ID_MAX)
    {
      s_btn_press_count[evt.button_id] = evt.press_count;
      s_btn_is_pressed[evt.button_id] = evt.is_pressed;
      app_push_work_item_from_isr((uint8_t)CM55_IPC_EVENT_BUTTON, (uint16_t)evt.button_id, &xHigherPriorityTaskWoken);
    }
  }
  else if (IPC_CMD_GYRO == msg->cmd)
  {
    (void)memcpy(&s_gyro_data, msg->data, sizeof(gyro_data_t));
    s_gyro_sequence = msg->value;
    app_push_work_item_from_isr((uint8_t)CM55_IPC_EVENT_GYRO, 0U, &xHigherPriorityTaskWoken);
  }
#if defined(TOUCH_VIA_IPC)
  else if (IPC_CMD_TOUCH == msg->cmd)
  {
    ipc_touch_event_t evt;
    (void)memcpy(&evt, msg->data, sizeof(ipc_touch_event_t));
    lv_port_indev_touch_from_ipc_set(evt.x, evt.y, evt.pressed);
  }
#endif

  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void cm55_ipc_app_event_cb(cm55_ipc_event_t event, const cm55_ipc_event_payload_t *payload, void *user_data)
{
  (void)user_data;
  if (NULL == payload)
  {
    return;
  }

  if (CM55_IPC_EVENT_LOG == event)
  {
    (void)payload;
  }
  else if ((CM55_IPC_EVENT_GYRO == event) && (NULL != payload->gyro.data))
  {
    (void)printf("cm55_ipc_task: Gyro data ax=%.3f ay=%.3f az=%.3f seq=%lu\n\r",
                 (double)payload->gyro.data->ax, (double)payload->gyro.data->ay, (double)payload->gyro.data->az,
                 (unsigned long)payload->gyro.sequence);
  }
  else if ((CM55_IPC_EVENT_WIFI_STATUS == event) && (NULL != payload->wifi_status.status))
  {
    const ipc_wifi_status_t *s = payload->wifi_status.status;
    bool changed = false;
    if (!s_wifi_status_printed_once)
    {
      changed = true;
      s_wifi_status_printed_once = true;
    }
    else if ((s_wifi_status_last_printed.state != s->state) ||
             (s_wifi_status_last_printed.rssi != s->rssi) ||
             (s_wifi_status_last_printed.reason != s->reason) ||
             (0 != strcmp(s_wifi_status_last_printed.ssid, s->ssid)))
    {
      changed = true;
    }
    if (changed)
    {
      (void)memcpy(&s_wifi_status_last_printed, s, sizeof(ipc_wifi_status_t));
      (void)printf("[CM55.WiFi.Status] state=%u rssi=%d ssid=%s reason=%u\n",
                   (unsigned int)s->state,
                   (int)s->rssi,
                   s->ssid,
                   (unsigned int)s->reason);
    }
    {
      char line[CM55_WIFI_DEBUG_LINE_MAX];
      (void)snprintf(line, sizeof(line), "state=%u rssi=%d reason=%u ssid=%s",
                     (unsigned int)s->state,
                     (int)s->rssi,
                     (unsigned int)s->reason,
                     s->ssid);
      app_wifi_debug_append("WIFI", line);
    }
  }
  else if ((CM55_IPC_EVENT_WIFI_COMPLETE == event) && (NULL != payload->wifi_complete.list) &&
           (payload->wifi_complete.count > 0U))
  {
    char line[CM55_WIFI_DEBUG_LINE_MAX];
    (void)printf("\n[CM55.WiFi.Scan] networks found: %lu\n", (unsigned long)payload->wifi_complete.count);
    (void)snprintf(line, sizeof(line), "scan_complete count=%lu", (unsigned long)payload->wifi_complete.count);
    app_wifi_debug_append("SCAN", line);
    for (uint32_t i = 0U; i < payload->wifi_complete.count; i++)
    {
      const wifi_info_t *info = &payload->wifi_complete.list[i];
      (void)printf("[CM55.WiFi.Scan] %2u. SSID: %-24s | RSSI: %4ld | Ch: %3lu | MAC: %02X:%02X:%02X:%02X:%02X:%02X | Security: %s\n",
                   (unsigned int)i, info->ssid, (long)info->rssi, (unsigned long)info->channel,
                   info->mac[0], info->mac[1], info->mac[2], info->mac[3], info->mac[4], info->mac[5], info->security);
    }
  }
}

static void cm55_ipc_app_receiver_task(void *arg)
{
  cm55_ipc_event_t event;
  cm55_ipc_event_payload_t payload;

  (void)arg;

  while (true)
  {
    if (app_receive_next(&event, &payload, portMAX_DELAY))
    {
      cm55_ipc_app_event_cb(event, &payload, NULL);
    }
  }
}

bool cm55_get_button_state(uint32_t button_id, uint32_t *press_count, bool *is_pressed)
{
  if (button_id >= BUTTON_ID_MAX)
  {
    return false;
  }
  if (NULL != press_count)
  {
    *press_count = s_btn_press_count[button_id];
  }
  if (NULL != is_pressed)
  {
    *is_pressed = s_btn_is_pressed[button_id];
  }
  return true;
}

bool cm55_get_wifi_list(wifi_info_t *out_list, uint32_t max_count, uint32_t *out_count)
{
  if ((NULL == out_list) || (NULL == out_count))
  {
    return false;
  }

  *out_count = 0U;
  if (false == s_wifi_list_ready)
  {
    return false;
  }

  {
    uint32_t n = (s_wifi_list_count < max_count) ? s_wifi_list_count : max_count;
    for (uint32_t i = 0U; i < n; i++)
    {
      (void)memcpy(&out_list[i], &s_wifi_list[i], sizeof(wifi_info_t));
    }
    *out_count = n;
  }

  s_wifi_list_ready = false;
  return true;
}

bool cm55_get_wifi_status(ipc_wifi_status_t *out_status)
{
  if (NULL == out_status)
  {
    return false;
  }
  (void)memcpy(out_status, &s_wifi_status, sizeof(ipc_wifi_status_t));
  return true;
}

bool cm55_get_wifi_debug_text(char *out_text, uint32_t out_size)
{
  uint32_t start_idx;
  uint32_t i;
  uint32_t pos;

  if ((NULL == out_text) || (0U == out_size))
  {
    return false;
  }

  out_text[0] = '\0';
  if (0U == s_wifi_debug_count)
  {
    return true;
  }

  if (s_wifi_debug_head >= s_wifi_debug_count)
  {
    start_idx = s_wifi_debug_head - s_wifi_debug_count;
  }
  else
  {
    start_idx = (CM55_WIFI_DEBUG_LINE_COUNT + s_wifi_debug_head) - s_wifi_debug_count;
  }

  pos = 0U;
  for (i = 0U; i < s_wifi_debug_count; i++)
  {
    uint32_t idx = start_idx + i;
    int written;
    if (idx >= CM55_WIFI_DEBUG_LINE_COUNT)
    {
      idx -= CM55_WIFI_DEBUG_LINE_COUNT;
    }
    written = snprintf(&out_text[pos], out_size - pos, "%s%s",
                       s_wifi_debug_lines[idx],
                       (i + 1U < s_wifi_debug_count) ? "\n" : "");
    if ((written < 0) || ((uint32_t)written >= (out_size - pos)))
    {
      out_text[out_size - 1U] = '\0';
      return true;
    }
    pos += (uint32_t)written;
  }
  return true;
}

uint32_t cm55_get_wifi_debug_sequence(void)
{
  return s_wifi_debug_sequence;
}

void cm55_trigger_scan_all(void)
{
  ipc_wifi_scan_request_t request;
  (void)memset(&request, 0, sizeof(request));
  request.use_filter = false;
  app_wifi_debug_append("TX", "scan_all");
  (void)cm55_ipc_pipe_push_request(IPC_CMD_WIFI_SCAN_REQ, &request, sizeof(request));
}

void cm55_trigger_scan_ssid(const char *ssid)
{
  ipc_wifi_scan_request_t request;
  (void)memset(&request, 0, sizeof(request));
  request.use_filter = true;
  request.filter.mode = WIFI_FILTER_MODE_SSID;
  if (NULL != ssid)
  {
    (void)strncpy((char *)request.filter.ssid, ssid, WIFI_SSID_MAX_LEN - 1U);
    request.filter.ssid[WIFI_SSID_MAX_LEN - 1U] = '\0';
  }
  app_wifi_debug_append("TX", "scan_ssid");
  (void)cm55_ipc_pipe_push_request(IPC_CMD_WIFI_SCAN_REQ, &request, sizeof(request));
}

void cm55_trigger_connect(const char *ssid, const char *password, uint32_t security)
{
  ipc_wifi_connect_request_t request;
  (void)memset(&request, 0, sizeof(request));
  if (NULL != ssid)
  {
    (void)strncpy(request.ssid, ssid, sizeof(request.ssid) - 1U);
  }
  if (NULL != password)
  {
    (void)strncpy(request.password, password, sizeof(request.password) - 1U);
  }
  request.security = security;
  app_wifi_debug_append("TX", "connect_req");
  (void)cm55_ipc_pipe_push_request(IPC_CMD_WIFI_CONNECT_REQ, &request, sizeof(request));
}

void cm55_trigger_disconnect(void)
{
  app_wifi_debug_append("TX", "disconnect_req");
  (void)cm55_ipc_pipe_push_request(IPC_CMD_WIFI_DISCONNECT_REQ, NULL, 0U);
}

void cm55_trigger_status_request(void)
{
  app_wifi_debug_append("TX", "status_req");
  (void)cm55_ipc_pipe_push_request(IPC_CMD_WIFI_STATUS_REQ, NULL, 0U);
}

bool cm55_ipc_app_init(void)
{
  s_draining_log = false;
  (void)memset(&s_wifi_status, 0, sizeof(s_wifi_status));
  s_wifi_status.state = (uint8_t)IPC_WIFI_LINK_DISCONNECTED;
  s_wifi_status.rssi = -127;
  s_wifi_status.reason = (uint16_t)IPC_WIFI_REASON_NONE;
  s_wifi_status_printed_once = false;
  s_wifi_debug_head = 0U;
  s_wifi_debug_count = 0U;
  s_wifi_debug_sequence = 0U;

  cm55_ipc_pipe_init(&CM55_GET_CONFIG_DEFAULT());

  s_log_queue = xQueueCreate(CM55_LOG_QUEUE_LENGTH, sizeof(app_log_msg_t));
  if (NULL == s_log_queue)
  {
    return false;
  }

  s_ipc_work_queue = xQueueCreate(IPC_WORK_QUEUE_LEN, sizeof(ipc_work_item_t));
  if (NULL == s_ipc_work_queue)
  {
    vQueueDelete(s_log_queue);
    s_log_queue = NULL;
    return false;
  }

  if (false == cm55_ipc_pipe_start(cm55_ipc_app_data_received_cb))
  {
    vQueueDelete(s_ipc_work_queue);
    vQueueDelete(s_log_queue);
    s_ipc_work_queue = NULL;
    s_log_queue = NULL;
    return false;
  }

  if (pdPASS != xTaskCreate(cm55_ipc_app_receiver_task, "IPC Receiver", IPC_RECEIVER_TASK_STACK, NULL,
                            IPC_RECEIVER_TASK_PRIO, NULL))
  {
    return false;
  }

  return true;
}
