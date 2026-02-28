/*******************************************************************************
 * File Name        : wifi_scanner.c
 *
 * Description      : Implementation of the Wi-Fi scanner (scan, filter, publish).
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 *
 *******************************************************************************/

#include "wifi_scanner.h"
#include "wifi_radio.h"

#include "cy_retarget_io.h"
#include "cy_wcm.h"
#include "cybsp.h"
#include "error_handler.h"
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

/*******************************************************************************
 * Macros
 *******************************************************************************/

#define WIFI_SCANNER_RESULT_MAX (20U)

#define SSID_MAX_LEN (32U)

#define ASCII_PRINTABLE_MIN (0x20U)
#define ASCII_PRINTABLE_MAX (0x7EU)

#define RESET_VAL (0U)

#define SECURITY_OPEN "OPEN"
#define SECURITY_WEP_PSK "WEP-PSK"
#define SECURITY_WEP_SHARED "WEP-SHARED"
#define SECURITY_WEP_TKIP_PSK "WEP-TKIP-PSK"
#define SECURITY_WPA_TKIP_PSK "WPA-TKIP-PSK"
#define SECURITY_WPA_AES_PSK "WPA-AES-PSK"
#define SECURITY_WPA_MIXED_PSK "WPA-MIXED-PSK"
#define SECURITY_WPA2_AES_PSK "WPA2-AES-PSK"
#define SECURITY_WPA2_TKIP_PSK "WPA2-TKIP-PSK"
#define SECURITY_WPA2_MIXED_PSK "WPA2-MIXED-PSK"
#define SECURITY_WPA2_FBT_PSK "WPA2-FBT-PSK"
#define SECURITY_WPA3_SAE "WPA3-SAE"
#define SECURITY_WPA2_WPA_AES_PSK "WPA2-WPA-AES-PSK"
#define SECURITY_WPA2_WPA_MIXED_PSK "WPA2-WPA-MIXED-PSK"
#define SECURITY_WPA3_WPA2_PSK "WPA3-WPA2-PSK"
#define SECURITY_WPA_TKIP_ENT "WPA-TKIP-ENT"
#define SECURITY_WPA_AES_ENT "WPA-AES-ENT"
#define SECURITY_WPA_MIXED_ENT "WPA-MIXED-ENT"
#define SECURITY_WPA2_TKIP_ENT "WPA2-TKIP-ENT"
#define SECURITY_WPA2_AES_ENT "WPA2-AES-ENT"
#define SECURITY_WPA2_MIXED_ENT "WPA2-MIXED-ENT"
#define SECURITY_WPA2_FBT_ENT "WPA2-FBT-ENT"
#define SECURITY_IBSS_OPEN "IBSS-OPEN"
#define SECURITY_WPS_SECURE "WPS-SECURE"
#define SECURITY_UNKNOWN "UNKNOWN"

/*******************************************************************************
 * Statics / Global Variables
 *******************************************************************************/

TaskHandle_t wifi_scanner_task_handle = NULL;
static wifi_scanner_config_t s_config;

/* Scan results: working buffer (filled by scan, used for filtering) */
static wifi_info_t s_working_list[WIFI_SCANNER_RESULT_MAX];
static uint32_t s_working_count;

/* Scan results: published buffer */
static wifi_info_t s_published_list[WIFI_SCANNER_RESULT_MAX];
static uint32_t s_published_count;

static volatile bool s_scan_in_progress = false;
static wifi_filter_config_t s_filter_config;

#define MAX_SCAN_COMPLETE_CALLBACKS (8U)
typedef struct
{
  wifi_scanner_callback_t cb;
  void *user_data;
  bool used;
} scan_complete_cb_entry_t;
static scan_complete_cb_entry_t s_scan_complete_cbs[MAX_SCAN_COMPLETE_CALLBACKS] = {0};

/** Returns true if all non-nul bytes in ssid are printable ASCII. */
static bool is_ssid_printable(const uint8_t *ssid)
{
  for (uint32_t i = 0; i < SSID_MAX_LEN; i++)
  {
    uint8_t c = ssid[i];
    if (0 == c)
      break;
    if ((c < ASCII_PRINTABLE_MIN) || (c > ASCII_PRINTABLE_MAX))
      return false;
  }
  return true;
}

/** Maps cy_wcm_security_t to a human-readable string. */
static const char *scan_security_to_string(cy_wcm_security_t sec)
{
  switch (sec)
  {
  case CY_WCM_SECURITY_OPEN:
    return SECURITY_OPEN;
  case CY_WCM_SECURITY_WEP_PSK:
    return SECURITY_WEP_PSK;
  case CY_WCM_SECURITY_WEP_SHARED:
    return SECURITY_WEP_SHARED;
  case CY_WCM_SECURITY_WPA_TKIP_PSK:
    return SECURITY_WPA_TKIP_PSK;
  case CY_WCM_SECURITY_WPA_AES_PSK:
    return SECURITY_WPA_AES_PSK;
  case CY_WCM_SECURITY_WPA_MIXED_PSK:
    return SECURITY_WPA_MIXED_PSK;
  case CY_WCM_SECURITY_WPA2_AES_PSK:
    return SECURITY_WPA2_AES_PSK;
  case CY_WCM_SECURITY_WPA2_TKIP_PSK:
    return SECURITY_WPA2_TKIP_PSK;
  case CY_WCM_SECURITY_WPA2_MIXED_PSK:
    return SECURITY_WPA2_MIXED_PSK;
  case CY_WCM_SECURITY_WPA2_FBT_PSK:
    return SECURITY_WPA2_FBT_PSK;
  case CY_WCM_SECURITY_WPA3_SAE:
    return SECURITY_WPA3_SAE;
  case CY_WCM_SECURITY_WPA3_WPA2_PSK:
    return SECURITY_WPA3_WPA2_PSK;
  case CY_WCM_SECURITY_IBSS_OPEN:
    return SECURITY_IBSS_OPEN;
  case CY_WCM_SECURITY_WPS_SECURE:
    return SECURITY_WPS_SECURE;
  case CY_WCM_SECURITY_UNKNOWN:
    return SECURITY_UNKNOWN;
  case CY_WCM_SECURITY_WPA2_WPA_AES_PSK:
    return SECURITY_WPA2_WPA_AES_PSK;
  case CY_WCM_SECURITY_WPA2_WPA_MIXED_PSK:
    return SECURITY_WPA2_WPA_MIXED_PSK;
  case CY_WCM_SECURITY_WPA_TKIP_ENT:
    return SECURITY_WPA_TKIP_ENT;
  case CY_WCM_SECURITY_WPA_AES_ENT:
    return SECURITY_WPA_AES_ENT;
  case CY_WCM_SECURITY_WPA_MIXED_ENT:
    return SECURITY_WPA_MIXED_ENT;
  case CY_WCM_SECURITY_WPA2_TKIP_ENT:
    return SECURITY_WPA2_TKIP_ENT;
  case CY_WCM_SECURITY_WPA2_AES_ENT:
    return SECURITY_WPA2_AES_ENT;
  case CY_WCM_SECURITY_WPA2_MIXED_ENT:
    return SECURITY_WPA2_MIXED_ENT;
  case CY_WCM_SECURITY_WPA2_FBT_ENT:
    return SECURITY_WPA2_FBT_ENT;
  default:
    return SECURITY_UNKNOWN;
  }
}

/** Sets filter to SSID match using the given ssid string. */
static void wifi_scanner_set_filter_mode_ssid(const char *ssid)
{
  s_filter_config.mode = WIFI_FILTER_MODE_SSID;
  strncpy((char *)s_filter_config.ssid, ssid, SSID_MAX_LEN);
}

/** Sets filter to BSSID (MAC) match using the given bssid string. */
static void wifi_scanner_set_filter_mode_bssid(const char *bssid)
{
  s_filter_config.mode = WIFI_FILTER_MODE_BSSID;
  strncpy((char *)s_filter_config.bssid, bssid, MAC_ADDRESS_LEN);
}

/** Sets filter to security type match. */
static void wifi_scanner_set_filter_mode_security(cy_wcm_security_t security)
{
  s_filter_config.mode = WIFI_FILTER_MODE_SECURITY;
  s_filter_config.security = security;
}

/** Sets filter to channel match. */
static void wifi_scanner_set_filter_mode_channel(uint8_t channel)
{
  s_filter_config.mode = WIFI_FILTER_MODE_CHANNEL;
  s_filter_config.channel = channel;
}

/** Sets filter to minimum RSSI threshold. */
static void wifi_scanner_set_filter_mode_rssi(int32_t rssi)
{
  s_filter_config.mode = WIFI_FILTER_MODE_RSSI;
  s_filter_config.rssi = rssi;
}

/** Clears filter so all scan results are kept. */
static void wifi_scanner_set_filter_mode_none(void)
{
  s_filter_config.mode = WIFI_FILTER_MODE_NONE;
}

/** Converts a cy_wcm_scan_result_t into wifi_info_t. */
static void scan_result_to_wifi_info(cy_wcm_scan_result_t *result, wifi_info_t *out)
{
  size_t ssid_len;

  memset(out, 0, sizeof(wifi_info_t));
  ssid_len = strlen((const char *)result->SSID);
  if (ssid_len >= sizeof(out->ssid))
    ssid_len = sizeof(out->ssid) - 1;

  memcpy(out->ssid, result->SSID, ssid_len);
  out->ssid[ssid_len] = '\0';
  out->rssi = (int32_t)result->signal_strength;
  out->channel = result->channel;
  memcpy(out->mac, result->BSSID, sizeof(out->mac));

  /* Security conversion can be done by user of this module if needed */
  strncpy(out->security, scan_security_to_string(result->security), sizeof(out->security) - 1);
  out->security[sizeof(out->security) - 1] = '\0';
}

/** Sorts s_working_list by RSSI descending (strongest first). */
static void sort_working_by_rssi_desc(void)
{
  for (uint32_t i = 1; i < s_working_count; i++)
  {
    wifi_info_t tmp;
    memcpy(&tmp, &s_working_list[i], sizeof(wifi_info_t));
    uint32_t j = i;
    while ((j > 0) && (s_working_list[j - 1].rssi < tmp.rssi))
    {
      memcpy(&s_working_list[j], &s_working_list[j - 1], sizeof(wifi_info_t));
      j--;
    }
    memcpy(&s_working_list[j], &tmp, sizeof(wifi_info_t));
  }
}

/** Callback from cy_wcm_start_scan; accumulates results, filters, publishes and notifies task on complete. */
static void scanner_callback(cy_wcm_scan_result_t *result_ptr, void *user_data, cy_wcm_scan_status_t status)
{
  (void)user_data;

  if (result_ptr && (s_working_count < WIFI_SCANNER_RESULT_MAX) && is_ssid_printable(result_ptr->SSID))
  {
    scan_result_to_wifi_info(result_ptr, &s_working_list[s_working_count]);
    s_working_count++;
  }

  if (CY_WCM_SCAN_COMPLETE == status)
  {

    /* Filter the scan results based on the configuration */
    uint32_t write_idx = 0;
    for (uint32_t read_idx = 0; read_idx < s_working_count; read_idx++)
    {
      const wifi_info_t *info = &s_working_list[read_idx];
      bool keep = false;

      if (s_filter_config.mode == WIFI_FILTER_MODE_SSID)
      {
        keep = (strcmp(info->ssid, (const char *)s_filter_config.ssid) == 0);
      }
      else if (s_filter_config.mode == WIFI_FILTER_MODE_BSSID)
      {
        keep = (memcmp(info->mac, s_filter_config.bssid, MAC_ADDRESS_LEN) == 0);
      }
      else if (s_filter_config.mode == WIFI_FILTER_MODE_SECURITY)
      {
        keep = (strcmp(info->security, scan_security_to_string((cy_wcm_security_t)s_filter_config.security)) == 0);
      }
      else if (s_filter_config.mode == WIFI_FILTER_MODE_CHANNEL)
      {
        keep = (info->channel == s_filter_config.channel);
      }
      else if (s_filter_config.mode == WIFI_FILTER_MODE_RSSI)
      {
        keep = (info->rssi >= s_filter_config.rssi);
      }
      else
      {
        /* WIFI_FILTER_MODE_NONE or others */
        keep = true;
      }

      if (keep)
      {
        if (write_idx != read_idx)
        {
          s_working_list[write_idx] = *info;
        }
        write_idx++;
      }
    }
    s_working_count = write_idx;

    sort_working_by_rssi_desc();

    taskENTER_CRITICAL();
    memcpy(s_published_list, s_working_list, s_working_count * sizeof(wifi_info_t));
    s_published_count = s_working_count;
    s_working_count = RESET_VAL;
    taskEXIT_CRITICAL();

    s_scan_in_progress = false;

#if 0
    // print debug info
    printf("\n[CM33] WiFi scan completed. Count: %u\n", (unsigned int)s_published_count);
    for (uint32_t i = 0; i < s_published_count; i++)
    {
      const wifi_info_t *info = &s_published_list[i];
      printf("%2u. SSID: %-24s | RSSI: %4ld | Ch: %3lu | MAC: %02X:%02X:%02X:%02X:%02X:%02X | Security: %s\n",
             (unsigned int)i, info->ssid, (long)info->rssi, (unsigned long)info->channel, info->mac[0], info->mac[1],
             info->mac[2], info->mac[3], info->mac[4], info->mac[5], info->security);
#endif

    for (uint32_t i = 0U; i < MAX_SCAN_COMPLETE_CALLBACKS; i++)
    {
      if (s_scan_complete_cbs[i].used && (s_scan_complete_cbs[i].cb != NULL))
      {
        s_scan_complete_cbs[i].cb(s_scan_complete_cbs[i].user_data, s_published_list, s_published_count);
      }
    }
    if (s_config.callback != NULL)
    {
      s_config.callback(s_config.user_data, s_published_list, s_published_count);
    }

    /* Also clear the notification for the task if any */
    if (wifi_scanner_task_handle != NULL)
    {
      xTaskNotifyGive(wifi_scanner_task_handle);
    }
  }
  else
  {
    // printf("[CM33] WiFi scan incomplete\n");
    // printf("[CM33] WiFi scan status: %d\n", status);
  }
}

/** Primary task for Wi-Fi scanning; initializes SDIO and WCM, then runs scan loop. */
static void wifi_scanner_task(void *arg)
{
  (void)arg;

  cy_rslt_t result;

  printf("[CM33] WiFi scanner task started\n");

  result = wifi_radio_init();
  if (CY_RSLT_SUCCESS != result)
  {
    handle_error(NULL);
  }

  while (true)
  {
    /* Wait for scan trigger */
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if (s_scan_in_progress)
      continue;

    s_working_count = RESET_VAL;
    s_scan_in_progress = true;

    /* Start the scan. For now, we always perform a full scan.
       Filtering is done on demand or by the CM55. */
    result = cy_wcm_start_scan(scanner_callback, NULL, NULL);

    if (result == CY_RSLT_SUCCESS)
    {
      /* Wait for scan completion notification from the callback */
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
    else
    {
      s_scan_in_progress = false;
      printf("[CM33] WiFi scan failed (0x%lx)\n", (unsigned long)result);
    }
  }
}

/** Returns true if the scanner task exists and no scan is in progress. */
static bool wifi_scanner_is_ready(void)
{
  return (wifi_scanner_task_handle != NULL) && (s_scan_in_progress == false);
}

/*******************************************************************************
 * Public API
 *******************************************************************************/

/** Copies config and optionally starts the scanner task. Returns true on success. */
bool wifi_scanner_setup(wifi_scanner_config_t *config, bool auto_start)
{
  if (config == NULL)
  {
    return false;
  }

  s_config = *config;

  if (auto_start)
  {
    return wifi_scanner_start();
  }

  return false;
}

/** Registers a callback for scan-complete events. Returns true on success, false if callback is NULL or list is full. */
bool wifi_scanner_on_scan_complete(wifi_scanner_callback_t callback, void *user_data)
{
  if (callback == NULL)
  {
    return false;
  }
  for (uint32_t i = 0U; i < MAX_SCAN_COMPLETE_CALLBACKS; i++)
  {
    if (!s_scan_complete_cbs[i].used)
    {
      s_scan_complete_cbs[i].cb = callback;
      s_scan_complete_cbs[i].user_data = user_data;
      s_scan_complete_cbs[i].used = true;
      return true;
    }
  }
  return false;
}

/** Creates the Wi-Fi scan task. Returns true if task was created or already running. */
bool wifi_scanner_start(void)
{
  if (wifi_scanner_task_handle == NULL)
  {
    BaseType_t result = xTaskCreate(wifi_scanner_task, "WiFi scanner task", s_config.task_stack_size, &s_config,
                                    s_config.task_priority, &wifi_scanner_task_handle);

    return (pdPASS == result);
  }

  return true;
}

/** Deletes the Wi-Fi scan task. Returns true if task was deleted. */
bool wifi_scanner_stop(void)
{
  if (wifi_scanner_task_handle == NULL)
  {
    return false;
  }

  vTaskDelete(wifi_scanner_task_handle);
  wifi_scanner_task_handle = NULL;
  return true;
}

/** Triggers a scan with optional filter; NULL keeps current filter. Returns true if scan was triggered. */
bool wifi_scanner_scan(wifi_filter_config_t *filter_config)
{
  if (!wifi_scanner_is_ready())
    return false;

  if (filter_config != NULL)
  {
    s_filter_config = *filter_config;
  }

  xTaskNotifyGive(wifi_scanner_task_handle);
  return true;
}

/** Returns true if the scanner task exists. */
bool wifi_scanner_is_running(void)
{
  return (wifi_scanner_task_handle != NULL);
}

/** Returns true if a scan is in progress. */
bool wifi_scanner_is_scanning(void)
{
  return s_scan_in_progress;
}

/** Sets SSID filter and triggers a scan. Returns true if scan was triggered. */
bool wifi_scanner_scan_ssid(const char *ssid)
{
  if (!wifi_scanner_is_ready())
    return false;

  wifi_scanner_set_filter_mode_ssid(ssid);
  xTaskNotifyGive(wifi_scanner_task_handle);
  return true;
}

/** Sets BSSID filter and triggers a scan. Returns true if scan was triggered. */
bool wifi_scanner_scan_bssid(const char *bssid)
{
  if (!wifi_scanner_is_ready())
    return false;

  wifi_scanner_set_filter_mode_bssid(bssid);
  xTaskNotifyGive(wifi_scanner_task_handle);
  return true;
}

/** Sets security filter and triggers a scan. Returns true if scan was triggered. */
bool wifi_scanner_scan_security(cy_wcm_security_t security)
{
  if (!wifi_scanner_is_ready())
    return false;

  wifi_scanner_set_filter_mode_security(security);
  xTaskNotifyGive(wifi_scanner_task_handle);
  return true;
}

/** Sets channel filter and triggers a scan. Returns true if scan was triggered. */
bool wifi_scanner_scan_channel(uint8_t channel)
{
  if (!wifi_scanner_is_ready())
    return false;

  wifi_scanner_set_filter_mode_channel(channel);
  xTaskNotifyGive(wifi_scanner_task_handle);
  return true;
}

/** Sets minimum RSSI filter and triggers a scan. Returns true if scan was triggered. */
bool wifi_scanner_scan_rssi(int32_t rssi)
{
  if (!wifi_scanner_is_ready())
    return false;

  wifi_scanner_set_filter_mode_rssi(rssi);
  xTaskNotifyGive(wifi_scanner_task_handle);
  return true;
}

/** Clears filter and triggers a scan (all APs). Returns true if scan was triggered. */
bool wifi_scanner_scan_all(void)
{
  if (!wifi_scanner_is_ready())
    return false;

  wifi_scanner_set_filter_mode_none();
  xTaskNotifyGive(wifi_scanner_task_handle);
  return true;
}

/* [] END OF FILE */
