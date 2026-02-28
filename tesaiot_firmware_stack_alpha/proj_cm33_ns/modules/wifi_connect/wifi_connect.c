/*******************************************************************************
 * File Name        : wifi_connect.c
 *
 * Description      : Implementation of the Wi-Fi connection manager (connect,
 *                    disconnect, reconnect). Uses WCM and SDIO for CYW55513.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM33 (non-secure), CYW55513 Wi-Fi
 *
 *******************************************************************************/

#include "wifi_connect.h"
#include "wifi_radio.h"

#include "FreeRTOS.h"
#include "cy_wcm.h"
#include "cy_wcm_error.h"
#include "task.h"
#include <stdlib.h>
#include <string.h>

#define WIFI_CONNECT_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE * 4U)
#define WIFI_CONNECT_TASK_PRIORITY (tskIDLE_PRIORITY + 2U)

/*******************************************************************************
 * Types
 *******************************************************************************/

typedef enum
{
  WIFI_STATE_IDLE,
  WIFI_STATE_CONNECTING,
  WIFI_STATE_CONNECTED,
  WIFI_STATE_DISCONNECTED,
  WIFI_STATE_RECONNECTING
} wifi_state_t;

struct wifi_connect
{
  TaskHandle_t task_handle;
  wifi_connect_config_t config;
  wifi_connect_callbacks_t callbacks;
  wifi_connect_params_t params;
  wifi_state_t state;
  cy_wcm_ip_address_t ip_address;
};

/*******************************************************************************
 * Statics / Global Variables
 *******************************************************************************/

static wifi_connect_t *wifi_event_owner = NULL;

/** Builds WCM connect params from wifi_connect_params_t. */
static cy_wcm_connect_params_t build_connect_params(const wifi_connect_params_t *params)
{
  cy_wcm_connect_params_t connect_params;
  memset(&connect_params, 0, sizeof(connect_params));

  strncpy((char *)connect_params.ap_credentials.SSID, params->ssid, CY_WCM_MAX_SSID_LEN);
  connect_params.ap_credentials.SSID[CY_WCM_MAX_SSID_LEN] = '\0';

  strncpy((char *)connect_params.ap_credentials.password, params->password, CY_WCM_MAX_PASSPHRASE_LEN);
  connect_params.ap_credentials.password[CY_WCM_MAX_PASSPHRASE_LEN] = '\0';

  connect_params.ap_credentials.security = params->security;

  if ((params->ip_setting.ip_address.version == CY_WCM_IP_VER_V4) ||
      (params->ip_setting.ip_address.version == CY_WCM_IP_VER_V6))
  {
    connect_params.static_ip_settings = (cy_wcm_ip_setting_t *)&params->ip_setting;
  }
  else
  {
    connect_params.static_ip_settings = NULL;
  }

  return connect_params;
}

/** Primary task for Wi-Fi connection; runs state machine (connect, reconnect, disconnect). */
static void wifi_connect_task(void *pvParameters)
{
  wifi_connect_t *wifi = (wifi_connect_t *)pvParameters;

  for (;;)
  {
    switch (wifi->state)
    {
    case WIFI_STATE_IDLE:
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
      break;

    case WIFI_STATE_CONNECTING:
    {
      printf("Connecting to %s...\n", wifi->params.ssid);
      cy_wcm_connect_params_t connect_params = build_connect_params(&wifi->params);
      cy_rslt_t result = cy_wcm_connect_ap(&connect_params, &wifi->ip_address);
      if (result == CY_RSLT_SUCCESS)
      {
        wifi->state = WIFI_STATE_CONNECTED;
        printf("Wi-Fi connected\n");
        if (wifi->callbacks.on_connected)
        {
          wifi->callbacks.on_connected(wifi, &wifi->ip_address, wifi->callbacks.user_ctx);
        }
      }
      else
      {
        printf("Failed to connect. Retrying in %u ms\n", (unsigned int)wifi->config.reconnect_interval_ms);
        wifi->state = WIFI_STATE_RECONNECTING;
      }
      break;
    }

    case WIFI_STATE_CONNECTED:
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
      break;

    case WIFI_STATE_DISCONNECTED:
      if (wifi->callbacks.on_disconnected)
      {
        wifi->callbacks.on_disconnected(wifi, wifi->callbacks.user_ctx);
      }
      if (wifi->config.reconnect_interval_ms > 0)
      {
        wifi->state = WIFI_STATE_RECONNECTING;
      }
      else
      {
        wifi->state = WIFI_STATE_IDLE;
      }
      break;

    case WIFI_STATE_RECONNECTING:
      vTaskDelay(pdMS_TO_TICKS(wifi->config.reconnect_interval_ms));
      wifi->state = WIFI_STATE_CONNECTING;
      break;
    }
  }
}

/** WCM event callback; handles disconnect and notifies task. */
static void wcm_event_callback(cy_wcm_event_t event, cy_wcm_event_data_t *event_data)
{
  (void)event_data;
  wifi_connect_t *wifi = wifi_event_owner;
  if ((wifi == NULL) || (wifi->task_handle == NULL))
  {
    return;
  }

  if (event == CY_WCM_EVENT_DISCONNECTED)
  {
    wifi->state = WIFI_STATE_DISCONNECTED;
    xTaskNotifyGive(wifi->task_handle);
  }
}

/*******************************************************************************
 * Public API
 *******************************************************************************/

/** Allocates handle, initializes SDIO and WCM, creates connection task. Returns CY_RSLT_SUCCESS on success. */
cy_rslt_t wifi_connect_init(wifi_connect_t **wifi,
                            const wifi_connect_config_t *config,
                            const wifi_connect_callbacks_t *callbacks)
{
  cy_rslt_t result;
  wifi_connect_t *wifi_handle = calloc(1, sizeof(wifi_connect_t));
  if (wifi_handle == NULL)
  {
    return CY_RSLT_TYPE_ERROR;
  }

  wifi_handle->config = *config;
  wifi_handle->callbacks = *callbacks;
  wifi_handle->state = WIFI_STATE_IDLE;

  result = wifi_radio_init();
  if (result != CY_RSLT_SUCCESS)
  {
    free(wifi_handle);
    return result;
  }

  result = cy_wcm_register_event_callback(wcm_event_callback);
  if (result != CY_RSLT_SUCCESS)
  {
    cy_wcm_deinit();
    free(wifi_handle);
    return result;
  }

  wifi_event_owner = wifi_handle;

  BaseType_t task_result =
      xTaskCreate(wifi_connect_task, "wifi_connect_task", WIFI_CONNECT_TASK_STACK_SIZE, wifi_handle,
                  WIFI_CONNECT_TASK_PRIORITY, &wifi_handle->task_handle);
  if (task_result != pdPASS)
  {
    wifi_event_owner = NULL;
    cy_wcm_deregister_event_callback(wcm_event_callback);
    cy_wcm_deinit();
    free(wifi_handle);
    return CY_RSLT_TYPE_ERROR;
  }

  *wifi = wifi_handle;
  return CY_RSLT_SUCCESS;
}

/** Starts connection to AP. Copies params and notifies task. Returns CY_RSLT_SUCCESS on success. */
cy_rslt_t wifi_connect_start(wifi_connect_t *wifi, const wifi_connect_params_t *params)
{
  if (!wifi || !params)
  {
    return CY_RSLT_TYPE_ERROR;
  }

  wifi->params = *params;
  wifi->state = WIFI_STATE_CONNECTING;
  xTaskNotifyGive(wifi->task_handle);

  return CY_RSLT_SUCCESS;
}

/** Disconnects from AP and sets state to IDLE. Returns CY_RSLT_SUCCESS on success. */
cy_rslt_t wifi_connect_stop(wifi_connect_t *wifi)
{
  if (!wifi)
  {
    return CY_RSLT_TYPE_ERROR;
  }

  cy_wcm_disconnect_ap();
  wifi->state = WIFI_STATE_IDLE;

  return CY_RSLT_SUCCESS;
}

/** Deletes task, deregisters WCM callback, deinits WCM and frees handle. Returns CY_RSLT_SUCCESS on success. */
cy_rslt_t wifi_connect_deinit(wifi_connect_t *wifi)
{
  if (!wifi)
  {
    return CY_RSLT_TYPE_ERROR;
  }

  if (wifi->task_handle)
  {
    vTaskDelete(wifi->task_handle);
  }

  if (wifi_event_owner == wifi)
  {
    wifi_event_owner = NULL;
    cy_wcm_deregister_event_callback(wcm_event_callback);
  }

  cy_wcm_deinit();
  free(wifi);

  return CY_RSLT_SUCCESS;
}

/** Returns true if currently connected to AP. */
bool wifi_connect_is_connected(wifi_connect_t *wifi)
{
  return wifi && wifi->state == WIFI_STATE_CONNECTED;
}

/* [] END OF FILE */
