/*******************************************************************************
 * File Name        : wifi_manager.c
 *
 * Description      : Wi-Fi manager task: scan, connect, disconnect, status via
 *                    queue; uses wifi_scanner and wifi_connect; emits events.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM33 (non-secure)
 *
 *******************************************************************************/

#include "wifi_manager.h"

#include "wifi_connect.h"
#include "wifi_scanner.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "cy_wcm.h"
#include <string.h>

#define WIFI_MANAGER_TASK_STACK (2048U)      /* Task stack size in words. */
#define WIFI_MANAGER_TASK_PRIO (2U)          /* Manager task priority. */
#define WIFI_MANAGER_QUEUE_LEN (16U)         /* Command queue length. */
#define WIFI_MANAGER_STATUS_POLL_MS (2000U)  /* Poll interval when idle (ms). */
#define WIFI_MANAGER_RECONNECT_MS (5000U)    /* Reconnect interval for wifi_connect (ms). */
#define WIFI_MANAGER_LAST_SCAN_MAX (32U)     /* Max APs to cache from last scan. */

typedef enum
{
  WIFI_MANAGER_CMD_SCAN = 0U,
  WIFI_MANAGER_CMD_CONNECT = 1U,
  WIFI_MANAGER_CMD_DISCONNECT = 2U,
  WIFI_MANAGER_CMD_STATUS = 3U,
  WIFI_MANAGER_CMD_CONNECTED = 4U,
  WIFI_MANAGER_CMD_DISCONNECTED = 5U
} wifi_manager_cmd_t;

typedef struct
{
  wifi_manager_cmd_t cmd;
  ipc_wifi_scan_request_t scan_request;
  ipc_wifi_connect_request_t connect_request;
} wifi_manager_msg_t;

typedef struct
{
  QueueHandle_t queue;               /* Command queue. */
  TaskHandle_t task;                 /* Manager task. */
  wifi_connect_t *wifi_connect;      /* Lazy-initialized connect handle. */
  wifi_connect_config_t connect_config;
  wifi_connect_callbacks_t connect_callbacks;
  wifi_manager_event_cb_t callback;  /* Event callback. */
  void *callback_user_data;
  ipc_wifi_status_t status;          /* Current status. */
  bool started;                      /* True if start() succeeded. */
} wifi_manager_ctx_t;

static wifi_manager_ctx_t s_ctx;
static wifi_info_t s_last_scan[WIFI_MANAGER_LAST_SCAN_MAX];
static uint32_t s_last_scan_count = 0U;

/**
 * Invokes the registered event callback with event, data, count and user_data. No-op if no callback.
 */
static void wifi_manager_emit(wifi_manager_event_t event, const void *data, uint32_t count)
{
  if (NULL != s_ctx.callback)
  {
    s_ctx.callback(event, data, count, s_ctx.callback_user_data);
  }
}

/**
 * Updates status reason and emits WIFI_MANAGER_EVENT_STATUS.
 */
static void wifi_manager_emit_status(ipc_wifi_reason_t reason)
{
  s_ctx.status.reason = (uint16_t)reason;
  wifi_manager_emit(WIFI_MANAGER_EVENT_STATUS, &s_ctx.status, 1U);
}

/**
 * If connected, fetches RSSI from WCM associated AP and emits status.
 */
static void wifi_manager_update_rssi(void)
{
  if (IPC_WIFI_LINK_CONNECTED != (ipc_wifi_link_state_t)s_ctx.status.state)
  {
    return;
  }

  cy_wcm_associated_ap_info_t ap_info;
  (void)memset(&ap_info, 0, sizeof(ap_info));
  if (CY_RSLT_SUCCESS == cy_wcm_get_associated_ap_info(&ap_info))
  {
    s_ctx.status.rssi = ap_info.signal_strength;
    wifi_manager_emit_status(IPC_WIFI_REASON_NONE);
  }
}

/**
 * Callback from wifi_scanner when scan completes; caches results and emits SCAN_RESULT + SCAN_COMPLETE.
 */
static void wifi_manager_scan_complete_cb(void *user_data, const wifi_info_t *results, uint32_t count)
{
  (void)user_data;

  if ((NULL != results) && (count > 0U))
  {
    wifi_manager_emit(WIFI_MANAGER_EVENT_SCAN_RESULT, results, count);
    {
      uint32_t copy_count = count;
      if (copy_count > WIFI_MANAGER_LAST_SCAN_MAX)
      {
        copy_count = WIFI_MANAGER_LAST_SCAN_MAX;
      }
      (void)memcpy(s_last_scan, results, copy_count * sizeof(wifi_info_t));
      s_last_scan_count = copy_count;
    }
  }
  else
  {
    s_last_scan_count = 0U;
  }

  ipc_wifi_scan_complete_t scan_complete;
  scan_complete.total_count = (uint16_t)count;
  scan_complete.status = 0U;
  wifi_manager_emit(WIFI_MANAGER_EVENT_SCAN_COMPLETE, &scan_complete, 1U);

  s_ctx.status.state = (uint8_t)IPC_WIFI_LINK_DISCONNECTED;
  s_ctx.status.rssi = -127;
  wifi_manager_emit_status(IPC_WIFI_REASON_NONE);
}

/**
 * Callback from wifi_connect when connected; queues WIFI_MANAGER_CMD_CONNECTED for task.
 */
static void wifi_manager_on_connected(wifi_connect_t *wifi, const cy_wcm_ip_address_t *ip_addr, void *user_ctx)
{
  (void)wifi;
  (void)ip_addr;
  (void)user_ctx;

  if ((NULL != s_ctx.queue) && (NULL != s_ctx.task))
  {
    wifi_manager_msg_t msg;
    (void)memset(&msg, 0, sizeof(msg));
    msg.cmd = WIFI_MANAGER_CMD_CONNECTED;
    (void)xQueueSend(s_ctx.queue, &msg, 0U);
  }
}

/**
 * Callback from wifi_connect when disconnected; queues WIFI_MANAGER_CMD_DISCONNECTED for task.
 */
static void wifi_manager_on_disconnected(wifi_connect_t *wifi, void *user_ctx)
{
  (void)wifi;
  (void)user_ctx;

  if ((NULL != s_ctx.queue) && (NULL != s_ctx.task))
  {
    wifi_manager_msg_t msg;
    (void)memset(&msg, 0, sizeof(msg));
    msg.cmd = WIFI_MANAGER_CMD_DISCONNECTED;
    (void)xQueueSend(s_ctx.queue, &msg, 0U);
  }
}

/**
 * Lazily creates wifi_connect handle and registers callbacks. Returns true if ready.
 */
static bool wifi_manager_ensure_connect_handle(void)
{
  if (NULL != s_ctx.wifi_connect)
  {
    return true;
  }

  s_ctx.connect_config.reconnect_interval_ms = WIFI_MANAGER_RECONNECT_MS;
  s_ctx.connect_callbacks.on_connected = wifi_manager_on_connected;
  s_ctx.connect_callbacks.on_disconnected = wifi_manager_on_disconnected;
  s_ctx.connect_callbacks.user_ctx = NULL;

  if (CY_RSLT_SUCCESS != wifi_connect_init(&s_ctx.wifi_connect, &s_ctx.connect_config, &s_ctx.connect_callbacks))
  {
    s_ctx.status.state = (uint8_t)IPC_WIFI_LINK_ERROR;
    s_ctx.status.rssi = -127;
    wifi_manager_emit_status(IPC_WIFI_REASON_CONNECT_FAILED);
    return false;
  }

  return true;
}

/**
 * Handles scan command: starts scanner if disconnected; emits status.
 */
static void wifi_manager_handle_scan(const ipc_wifi_scan_request_t *request)
{
  if (IPC_WIFI_LINK_DISCONNECTED != (ipc_wifi_link_state_t)s_ctx.status.state)
  {
    wifi_manager_emit_status(IPC_WIFI_REASON_SCAN_BLOCKED_CONNECTED);
    return;
  }

  if (NULL == request)
  {
    return;
  }

  s_ctx.status.state = (uint8_t)IPC_WIFI_LINK_SCANNING;
  s_ctx.status.rssi = -127;
  wifi_manager_emit_status(IPC_WIFI_REASON_NONE);

  if (false == wifi_scanner_scan(request->use_filter ? (wifi_filter_config_t *)&request->filter : NULL))
  {
    s_ctx.status.state = (uint8_t)IPC_WIFI_LINK_ERROR;
    s_ctx.status.rssi = -127;
    wifi_manager_emit_status(IPC_WIFI_REASON_SCAN_FAILED);
  }
}

/**
 * Handles connect command: ensures connect handle, starts wifi_connect with request params.
 */
static void wifi_manager_handle_connect(const ipc_wifi_connect_request_t *request)
{
  if (NULL == request)
  {
    return;
  }

  if (false == wifi_manager_ensure_connect_handle())
  {
    return;
  }

  wifi_connect_params_t params;
  (void)memset(&params, 0, sizeof(params));
  (void)strncpy(params.ssid, request->ssid, sizeof(params.ssid) - 1U);
  (void)strncpy(params.password, request->password, sizeof(params.password) - 1U);
  if (0U == request->security)
  {
    params.security = CY_WCM_SECURITY_WPA2_AES_PSK;
  }
  else
  {
    params.security = (cy_wcm_security_t)request->security;
  }

  s_ctx.status.state = (uint8_t)IPC_WIFI_LINK_CONNECTING;
  s_ctx.status.rssi = -127;
  (void)strncpy(s_ctx.status.ssid, request->ssid, sizeof(s_ctx.status.ssid) - 1U);
  s_ctx.status.ssid[sizeof(s_ctx.status.ssid) - 1U] = '\0';
  wifi_manager_emit_status(IPC_WIFI_REASON_NONE);

  if (CY_RSLT_SUCCESS != wifi_connect_start(s_ctx.wifi_connect, &params))
  {
    s_ctx.status.state = (uint8_t)IPC_WIFI_LINK_ERROR;
    s_ctx.status.rssi = -127;
    wifi_manager_emit_status(IPC_WIFI_REASON_CONNECT_FAILED);
  }
}

/**
 * Handles disconnect command: stops wifi_connect and emits DISCONNECTED status.
 */
static void wifi_manager_handle_disconnect(void)
{
  if (NULL != s_ctx.wifi_connect)
  {
    (void)wifi_connect_stop(s_ctx.wifi_connect);
  }
  s_ctx.status.state = (uint8_t)IPC_WIFI_LINK_DISCONNECTED;
  s_ctx.status.rssi = -127;
  wifi_manager_emit_status(IPC_WIFI_REASON_DISCONNECTED);
}

/**
 * Manager task: receives commands from queue (scan, connect, disconnect, status) and dispatches;
 * when idle polls RSSI. Runs until task deleted.
 */
static void wifi_manager_task(void *arg)
{
  (void)arg;
  wifi_manager_msg_t msg;

  while (true)
  {
    if (xQueueReceive(s_ctx.queue, &msg, pdMS_TO_TICKS(WIFI_MANAGER_STATUS_POLL_MS)) == pdPASS)
    {
      switch (msg.cmd)
      {
      case WIFI_MANAGER_CMD_SCAN:
        wifi_manager_handle_scan(&msg.scan_request);
        break;
      case WIFI_MANAGER_CMD_CONNECT:
        wifi_manager_handle_connect(&msg.connect_request);
        break;
      case WIFI_MANAGER_CMD_DISCONNECT:
        wifi_manager_handle_disconnect();
        break;
      case WIFI_MANAGER_CMD_STATUS:
        wifi_manager_emit_status(IPC_WIFI_REASON_NONE);
        break;
      case WIFI_MANAGER_CMD_CONNECTED:
        s_ctx.status.state = (uint8_t)IPC_WIFI_LINK_CONNECTED;
        wifi_manager_update_rssi();
        wifi_manager_emit_status(IPC_WIFI_REASON_NONE);
        break;
      case WIFI_MANAGER_CMD_DISCONNECTED:
        s_ctx.status.state = (uint8_t)IPC_WIFI_LINK_DISCONNECTED;
        s_ctx.status.rssi = -127;
        wifi_manager_emit_status(IPC_WIFI_REASON_DISCONNECTED);
        break;
      default:
        break;
      }
    }
    else
    {
      wifi_manager_update_rssi();
    }
  }
}

/**
 * Initializes internal state. Call before start. Returns true.
 */
bool wifi_manager_init(void)
{
  (void)memset(&s_ctx, 0, sizeof(s_ctx));
  s_ctx.status.state = (uint8_t)IPC_WIFI_LINK_DISCONNECTED;
  s_ctx.status.rssi = -127;
  s_ctx.status.reason = (uint16_t)IPC_WIFI_REASON_NONE;
  return true;
}

/**
 * Creates queue, sets up wifi_scanner and scan callback, creates manager task. Idempotent. Returns true on success.
 */
bool wifi_manager_start(void)
{
  wifi_scanner_config_t scanner_config;

  if (s_ctx.started)
  {
    return true;
  }

  s_ctx.queue = xQueueCreate(WIFI_MANAGER_QUEUE_LEN, sizeof(wifi_manager_msg_t));
  if (NULL == s_ctx.queue)
  {
    return false;
  }

  (void)memset(&scanner_config, 0, sizeof(scanner_config));
  scanner_config.task_priority = 1U;
  scanner_config.task_stack_size = (1024U * 4U);
  scanner_config.filter_mode = WIFI_FILTER_MODE_NONE;

  if (false == wifi_scanner_setup(&scanner_config, true))
  {
    return false;
  }

  if (false == wifi_scanner_on_scan_complete(wifi_manager_scan_complete_cb, NULL))
  {
    return false;
  }

  if (xTaskCreate(wifi_manager_task, "wifi_manager", WIFI_MANAGER_TASK_STACK, NULL, WIFI_MANAGER_TASK_PRIO,
                  &s_ctx.task) != pdPASS)
  {
    return false;
  }

  s_ctx.started = true;
  return true;
}

/**
 * Registers event callback and user_data. Replaces previous. Returns true.
 */
bool wifi_manager_set_event_callback(wifi_manager_event_cb_t callback, void *user_data)
{
  s_ctx.callback = callback;
  s_ctx.callback_user_data = user_data;
  return true;
}

/**
 * Queues scan request. Fails if queue NULL or request NULL. Returns true if queued.
 */
bool wifi_manager_request_scan(const ipc_wifi_scan_request_t *request)
{
  wifi_manager_msg_t msg;

  if ((NULL == s_ctx.queue) || (NULL == request))
  {
    return false;
  }

  (void)memset(&msg, 0, sizeof(msg));
  msg.cmd = WIFI_MANAGER_CMD_SCAN;
  msg.scan_request = *request;
  return (pdPASS == xQueueSend(s_ctx.queue, &msg, 0U));
}

/**
 * Queues connect request. Returns true if queued.
 */
bool wifi_manager_request_connect(const ipc_wifi_connect_request_t *request)
{
  wifi_manager_msg_t msg;

  if ((NULL == s_ctx.queue) || (NULL == request))
  {
    return false;
  }

  (void)memset(&msg, 0, sizeof(msg));
  msg.cmd = WIFI_MANAGER_CMD_CONNECT;
  msg.connect_request = *request;
  return (pdPASS == xQueueSend(s_ctx.queue, &msg, 0U));
}

/**
 * Queues disconnect request. Returns true if queued.
 */
bool wifi_manager_request_disconnect(void)
{
  wifi_manager_msg_t msg;

  if (NULL == s_ctx.queue)
  {
    return false;
  }

  (void)memset(&msg, 0, sizeof(msg));
  msg.cmd = WIFI_MANAGER_CMD_DISCONNECT;
  return (pdPASS == xQueueSend(s_ctx.queue, &msg, 0U));
}

/**
 * Queues status request; callback will receive current status. Returns true if queued.
 */
bool wifi_manager_request_status(void)
{
  wifi_manager_msg_t msg;

  if (NULL == s_ctx.queue)
  {
    return false;
  }

  (void)memset(&msg, 0, sizeof(msg));
  msg.cmd = WIFI_MANAGER_CMD_STATUS;
  return (pdPASS == xQueueSend(s_ctx.queue, &msg, 0U));
}

/**
 * Copies last scan results into out (up to max_count). Sets *out_count. Returns true if *out_count > 0.
 */
bool wifi_manager_get_last_scan(wifi_info_t *out, uint32_t max_count, uint32_t *out_count)
{
  if ((NULL == out) || (NULL == out_count) || (0U == max_count))
  {
    return false;
  }
  if (0U == s_last_scan_count)
  {
    *out_count = 0U;
    return false;
  }
  {
    uint32_t n = s_last_scan_count;
    if (n > max_count)
    {
      n = max_count;
    }
    (void)memcpy(out, s_last_scan, n * sizeof(wifi_info_t));
    *out_count = n;
    return true;
  }
}
