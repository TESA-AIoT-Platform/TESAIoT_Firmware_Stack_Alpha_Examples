/*******************************************************************************
 * File Name        : cm55_ipc_pipe.h
 *
 * Description      : Public interface for CM55 IPC pipe communication with CM33.
 *                    Wi-Fi scan triggers, button state, and scan results access.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 *
 *******************************************************************************/

#ifndef CM55_IPC_PIPE_H
#define CM55_IPC_PIPE_H

#include "FreeRTOS.h"
#include "ipc_communication.h"
#include "task.h"

#include <stdbool.h>
#include <stdint.h>

/* Wi-Fi list and packed value encoding for IPC messages. */
#define CM55_IPC_PIPE_WIFI_LIST_MAX (32U)        /* Max number of Wi-Fi entries in a scan list (payload and local array size). */
#define CM55_IPC_PIPE_VALUE_INDEX_MASK (0xFFFFU) /* Mask for lower 16 bits of packed value (index) or count after shift. */
#define CM55_IPC_PIPE_VALUE_COUNT_SHIFT (16U)    /* Shift to get total count from upper 16 bits of packed value (packed >> shift). */

/* Default pipe task and queue tuning. */
#define CM55_IPC_PIPE_TASK_STACK_DEFAULT (1024U)     /* Default stack size in words for the IPC pipe task. */
#define CM55_IPC_PIPE_TASK_PRIO_DEFAULT (2U)         /* Default FreeRTOS priority for the pipe task. */
#define CM55_IPC_PIPE_SEND_QUEUE_LEN_DEFAULT (10U)   /* Default length of the send queue to CM33. */
#define CM55_IPC_PIPE_STARTUP_DELAY_MS_DEFAULT (50U) /* Default ms delay after pipe setup, before callback registration. */

/** Run-time configuration for the CM55 IPC pipe task and send queue. */
typedef struct
{
  uint32_t task_stack;     /* Stack size in words for the IPC pipe FreeRTOS task. */
  uint32_t task_prio;      /* FreeRTOS priority of the pipe task. */
  uint32_t send_queue_len; /* Length of the send queue (outgoing requests to CM33). */
  uint32_t
      startup_delay_ms; /* Startup delay in ms after pipe setup, before registering callback (lets CM33/IPC settle). */
} cm55_ipc_pipe_config_t;

/** Initializer for default pipe config (stack, prio, queue length, startup delay). */
#define CM55_GET_CONFIG_DEFAULT()                                 \
  ((cm55_ipc_pipe_config_t){                                      \
      .task_stack = CM55_IPC_PIPE_TASK_STACK_DEFAULT,             \
      .task_prio = CM55_IPC_PIPE_TASK_PRIO_DEFAULT,               \
      .send_queue_len = CM55_IPC_PIPE_SEND_QUEUE_LEN_DEFAULT,     \
      .startup_delay_ms = CM55_IPC_PIPE_STARTUP_DELAY_MS_DEFAULT, \
  })

/** Callback invoked when raw IPC message data is received (msg_data from ipc_msg_t). */
typedef void (*cm55_ipc_data_received_cb_t)(uint32_t *msg_data);

/**
 * One-time init; use config or CM55_GET_CONFIG_DEFAULT(). Must be called before start.
 */
void cm55_ipc_pipe_init(const cm55_ipc_pipe_config_t *config);

/**
 * Set callback for raw received data (alternative to passing cb to start). NULL allowed.
 */
void cm55_ipc_pipe_set_data_received_callback(cm55_ipc_data_received_cb_t cb);

/**
 * Start pipe and RX path: creates send queue, runs communication setup, waits startup_delay_ms,
 * registers callback, creates sender task. On failure caller must treat as error and disable
 * interrupts. Returns false on queue or register or task create failure.
 */
bool cm55_ipc_pipe_start(cm55_ipc_data_received_cb_t cb);

/**
 * Enqueue a request to CM33 (cmd from ipc_communication.h, data/data_len). Returns false if queue
 * full or not initialized.
 */
bool cm55_ipc_pipe_push_request(uint32_t cmd, const void *data, uint32_t data_len);

#endif /* CM55_IPC_PIPE_H */
