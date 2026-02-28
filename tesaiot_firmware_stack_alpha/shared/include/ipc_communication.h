/*******************************************************************************
 * File Name        : ipc_communication.h
 *
 * Description      : Headers and structures for IPC Pipes between CM33 and CM55.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 *
 *******************************************************************************/

#ifndef SOURCE_IPC_COMMUNICATION_H
#define SOURCE_IPC_COMMUNICATION_H

/*******************************************************************************
 * Header Files
 *******************************************************************************/
#include "cy_ipc_pipe.h"
#include "cy_pdl.h"
#include "cybsp.h"
#include "wifi_scanner_types.h"
#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
 * Macros
 *******************************************************************************/
#define CY_IPC_MAX_ENDPOINTS (5UL)
#define CY_IPC_CYPIPE_CLIENT_CNT (8UL)

#define CY_IPC_CHAN_CYPIPE_EP1 (4UL)
#define CY_IPC_INTR_CYPIPE_EP1 (4UL)
#define CY_IPC_CHAN_CYPIPE_EP2 (15UL)
#define CY_IPC_INTR_CYPIPE_EP2 (5UL)

/* IPC Pipe Endpoint-1 config */
#define CY_IPC_CYPIPE_CHAN_MASK_EP1 CY_IPC_CH_MASK(CY_IPC_CHAN_CYPIPE_EP1)
#define CY_IPC_CYPIPE_INTR_MASK_EP1 CY_IPC_INTR_MASK(CY_IPC_INTR_CYPIPE_EP1)
#define CY_IPC_INTR_CYPIPE_PRIOR_EP1 (1UL)
#define CY_IPC_INTR_CYPIPE_MUX_EP1 (CY_IPC0_INTR_MUX(CY_IPC_INTR_CYPIPE_EP1))
#define CM33_IPC_PIPE_EP_ADDR (1UL)
#define CM33_IPC_PIPE_CLIENT_ID (3UL)

/* IPC Pipe Endpoint-2 config */
#define CY_IPC_CYPIPE_CHAN_MASK_EP2 CY_IPC_CH_MASK(CY_IPC_CHAN_CYPIPE_EP2)
#define CY_IPC_CYPIPE_INTR_MASK_EP2 CY_IPC_INTR_MASK(CY_IPC_INTR_CYPIPE_EP2)
#define CY_IPC_INTR_CYPIPE_PRIOR_EP2 (1UL)
#define CY_IPC_INTR_CYPIPE_MUX_EP2 (CY_IPC0_INTR_MUX(CY_IPC_INTR_CYPIPE_EP2))
#define CM55_IPC_PIPE_EP_ADDR (2UL)
#define CM55_IPC_PIPE_CLIENT_ID (5UL)

/* Combined Interrupt Mask */
#define CY_IPC_CYPIPE_INTR_MASK (CY_IPC_CYPIPE_CHAN_MASK_EP1 | CY_IPC_CYPIPE_CHAN_MASK_EP2)

#define IPC_SEMA_INDEX_DEBUG_UART (7UL)

/* Shared command messages */
#define IPC_CMD_LOG (0x90)
#define IPC_CMD_GYRO (0x91)
#define IPC_CMD_BUTTON_EVENT (0x93)
#define IPC_CMD_CLI_MSG (0x94)
#define IPC_CMD_TOUCH (0x95)
#define IPC_CMD_PING (0x9F)
#define IPC_CMD_PRINT (0x96)

/* Wi-Fi command messages sent from CM55 to CM33 */
#define IPC_CMD_WIFI_SCAN_REQ (0xA0)
#define IPC_CMD_WIFI_CONNECT_REQ (0xA1)
#define IPC_CMD_WIFI_DISCONNECT_REQ (0xA2)
#define IPC_CMD_WIFI_STATUS_REQ (0xA3)

/* Wi-Fi event messages sent from CM33 to CM55 */
#define IPC_EVT_WIFI_SCAN_RESULT (0xB0)
#define IPC_EVT_WIFI_SCAN_COMPLETE (0xB1)
#define IPC_EVT_WIFI_STATUS (0xB2)

#define IPC_WIFI_SCAN_VALUE_INDEX_MASK (0xFFFFU)
#define IPC_WIFI_SCAN_VALUE_COUNT_SHIFT (16U)

#define IPC_DATA_MAX_LEN (128UL) /* Max data length in bytes (char elements) */

typedef struct
{
  uint16_t client_id;          /* Bits 0-7: Client ID */
  uint16_t intr_mask;          /* Bits 16-31: Release Mask (MANDATORY for Pipe Driver) */
  uint32_t cmd;                /* Command code (e.g. IPC_CMD_LOG, IPC_CMD_GYRO) */
  uint32_t value;              /* Command argument or flags */
  char data[IPC_DATA_MAX_LEN]; /* Payload buffer, up to IPC_DATA_MAX_LEN bytes */
} ipc_msg_t;

typedef struct
{
  int16_t x;
  int16_t y;
  uint8_t pressed;
} ipc_touch_event_t;

typedef struct
{
  float ax;
  float ay;
  float az;
} gyro_data_t;

typedef enum
{
  IPC_WIFI_LINK_DISCONNECTED = 0U,
  IPC_WIFI_LINK_CONNECTING = 1U,
  IPC_WIFI_LINK_CONNECTED = 2U,
  IPC_WIFI_LINK_SCANNING = 3U,
  IPC_WIFI_LINK_ERROR = 4U
} ipc_wifi_link_state_t;

typedef enum
{
  IPC_WIFI_REASON_NONE = 0U,
  IPC_WIFI_REASON_SCAN_BLOCKED_CONNECTED = 1U,
  IPC_WIFI_REASON_SCAN_FAILED = 2U,
  IPC_WIFI_REASON_CONNECT_FAILED = 3U,
  IPC_WIFI_REASON_DISCONNECTED = 4U
} ipc_wifi_reason_t;

typedef struct
{
  char ssid[WIFI_SSID_MAX_LEN + 1U];
  char password[64U + 1U];
  uint32_t security;
} ipc_wifi_connect_request_t;

typedef struct
{
  bool use_filter;
  wifi_filter_config_t filter;
} ipc_wifi_scan_request_t;

typedef struct
{
  uint8_t state;
  int16_t rssi;
  uint16_t reason;
  char ssid[WIFI_SSID_MAX_LEN + 1U];
} ipc_wifi_status_t;

typedef struct
{
  uint16_t total_count;
  uint16_t status;
} ipc_wifi_scan_complete_t;

/*******************************************************************************
 * Function prototypes
 *******************************************************************************/

/** CM33 */
void cm33_ipc_communication_setup(void);
void cm33_ipc_pipe_isr(void);

/** CM55 */
void cm55_ipc_communication_setup(void);
void cm55_ipc_pipe_isr(void);

#endif /* SOURCE_IPC_COMMUNICATION_H */
