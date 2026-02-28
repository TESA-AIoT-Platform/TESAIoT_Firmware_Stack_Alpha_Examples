/*******************************************************************************
 * File Name        : wifi_scanner_types.h
 *
 * Description      : Shared types for Wi-Fi scanner (CM33/CM55).
 *                    Minimal dependencies (stdint) for use on both cores.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 *
 *******************************************************************************/

#ifndef WIFI_SCANNER_TYPES_H
#define WIFI_SCANNER_TYPES_H

#include <stdint.h>

#define MAC_ADDRESS_LEN (6U)
#define WIFI_SSID_MAX_LEN (32U)
#define WIFI_BSSID_LEN (6U)

typedef struct
{
  char ssid[64];
  int32_t rssi;
  uint8_t channel;
  uint8_t mac[6];
  char security[32];
} wifi_info_t;

typedef enum
{
  WIFI_FILTER_MODE_NONE = 0,
  WIFI_FILTER_MODE_SSID,
  WIFI_FILTER_MODE_MAC,
  WIFI_FILTER_MODE_BSSID,
  WIFI_FILTER_MODE_BAND,
  WIFI_FILTER_MODE_RSSI,
  WIFI_FILTER_MODE_SECURITY,
  WIFI_FILTER_MODE_CHANNEL,
  WIFI_FILTER_MODE_MAX
} wifi_filter_mode_t;

typedef struct
{
  wifi_filter_mode_t mode;
  uint8_t ssid[WIFI_SSID_MAX_LEN];
  uint8_t bssid[WIFI_BSSID_LEN];
  uint32_t security;
  uint8_t channel;
  int32_t rssi;
} wifi_filter_config_t;

#endif /* WIFI_SCANNER_TYPES_H */
