#ifndef TESA_EVENT_BUS_H
#define TESA_EVENT_BUS_H

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TESA_EVENT_BUS_MAX_CHANNELS 16
#define TESA_EVENT_BUS_MAX_SUBSCRIBERS_PER_CHANNEL 8
#define TESA_EVENT_BUS_EVENT_POOL_SIZE 32
#define TESA_EVENT_BUS_PAYLOAD_BUFFER_SIZE 256
#define TESA_EVENT_BUS_DEFAULT_QUEUE_TIMEOUT_MS 100

#define TESA_EVENT_BUS_MAX_PAYLOAD_SIZE \
  TESA_EVENT_BUS_PAYLOAD_BUFFER_SIZE
#define TESA_EVENT_BUS_STATS_MAX_VALUE UINT32_MAX
#define TESA_EVENT_BUS_TIMESTAMP_ROLLOVER_MS (4294967295UL)

typedef uint16_t tesa_event_channel_id_t;
typedef uint32_t tesa_event_type_t;

typedef enum {
  TESA_EVENT_BUS_SUCCESS = 0,
  TESA_EVENT_BUS_ERROR_INVALID_PARAM,
  TESA_EVENT_BUS_ERROR_CHANNEL_NOT_FOUND,
  TESA_EVENT_BUS_ERROR_CHANNEL_FULL,
  TESA_EVENT_BUS_ERROR_SUBSCRIBER_FULL,
  TESA_EVENT_BUS_ERROR_MEMORY,
  TESA_EVENT_BUS_ERROR_QUEUE_FULL,
  TESA_EVENT_BUS_ERROR_HAS_SUBSCRIBERS,
  TESA_EVENT_BUS_ERROR_PAYLOAD_TOO_LARGE,
  TESA_EVENT_BUS_ERROR_INVALID_QUEUE,
  TESA_EVENT_BUS_ERROR_PARTIAL_SUCCESS,
  TESA_EVENT_BUS_ERROR_STATS_OVERFLOW
} tesa_event_bus_result_t;

typedef enum {
  TESA_EVENT_BUS_QUEUE_DROP_NEWEST = 0,
  TESA_EVENT_BUS_QUEUE_DROP_OLDEST,
  TESA_EVENT_BUS_QUEUE_NO_DROP,
  TESA_EVENT_BUS_QUEUE_WAIT
} tesa_event_bus_queue_policy_t;

typedef struct {
  tesa_event_channel_id_t channel_id;
  tesa_event_type_t event_type;
  void *payload;
  size_t payload_size;
  uint32_t timestamp_ms;
  TaskHandle_t source_task;
  bool from_isr;
} tesa_event_t;

typedef struct {
  tesa_event_bus_queue_policy_t queue_policy;
  TickType_t queue_timeout_ticks;
} tesa_event_bus_channel_config_t;

typedef struct {
  uint32_t posts_successful;
  uint32_t posts_dropped;
  uint32_t last_drop_timestamp_ms;
} tesa_event_bus_subscriber_stats_t;

typedef struct {
  tesa_event_channel_id_t channel_id;
  const char *channel_name;
  bool registered;
  QueueHandle_t subscribers[TESA_EVENT_BUS_MAX_SUBSCRIBERS_PER_CHANNEL];
  tesa_event_bus_subscriber_stats_t
      subscriber_stats[TESA_EVENT_BUS_MAX_SUBSCRIBERS_PER_CHANNEL];
  tesa_event_bus_channel_config_t config;
  uint8_t subscriber_count;
} tesa_event_channel_t;

tesa_event_bus_result_t tesa_event_bus_init(void);

tesa_event_bus_result_t
tesa_event_bus_register_channel(tesa_event_channel_id_t channel_id,
                                const char *channel_name);

tesa_event_bus_result_t tesa_event_bus_register_channel_with_config(
    tesa_event_channel_id_t channel_id, const char *channel_name,
    const tesa_event_bus_channel_config_t *config);

tesa_event_bus_result_t
tesa_event_bus_unregister_channel(tesa_event_channel_id_t channel_id);

tesa_event_bus_result_t
tesa_event_bus_subscribe(tesa_event_channel_id_t channel_id,
                         QueueHandle_t queue_handle);

tesa_event_bus_result_t
tesa_event_bus_unsubscribe(tesa_event_channel_id_t channel_id,
                           QueueHandle_t queue_handle);

tesa_event_bus_result_t tesa_event_bus_post(tesa_event_channel_id_t channel_id,
                                            tesa_event_type_t event_type,
                                            const void *payload,
                                            size_t payload_size);

BaseType_t tesa_event_bus_post_from_isr(tesa_event_channel_id_t channel_id,
                                        tesa_event_type_t event_type,
                                        const void *payload,
                                        size_t payload_size,
                                        BaseType_t *pxHigherPriorityTaskWoken);

uint32_t tesa_event_bus_get_timestamp_ms(void);

uint32_t tesa_event_bus_check_timestamp_rollover(uint32_t timestamp_ms);

void tesa_event_bus_free_event(tesa_event_t *event);

tesa_event_bus_result_t
tesa_event_bus_get_subscriber_stats(tesa_event_channel_id_t channel_id,
                                    QueueHandle_t queue_handle,
                                    tesa_event_bus_subscriber_stats_t *stats);

tesa_event_bus_result_t
tesa_event_bus_get_channel_stats(tesa_event_channel_id_t channel_id,
                                 tesa_event_bus_subscriber_stats_t *total_stats,
                                 uint8_t *subscriber_count);

#endif
