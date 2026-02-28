#include "tesa_event_bus.h"
#include "cy_time.h"
#include "portmacro.h"
#include <string.h>

static tesa_event_channel_t channel_registry[TESA_EVENT_BUS_MAX_CHANNELS];
static tesa_event_t event_pool[TESA_EVENT_BUS_EVENT_POOL_SIZE];
static bool event_pool_used[TESA_EVENT_BUS_EVENT_POOL_SIZE];
static uint8_t payload_buffers[TESA_EVENT_BUS_EVENT_POOL_SIZE]
                              [TESA_EVENT_BUS_PAYLOAD_BUFFER_SIZE];
static bool payload_buffer_used[TESA_EVENT_BUS_EVENT_POOL_SIZE];
static bool event_bus_initialized = false;
static uint8_t registered_channel_count = 0;

static tesa_event_t
    *post_events_buffer[TESA_EVENT_BUS_MAX_SUBSCRIBERS_PER_CHANNEL];
static void *post_payloads_buffer[TESA_EVENT_BUS_MAX_SUBSCRIBERS_PER_CHANNEL];

static tesa_event_t *allocate_event(void);
static void free_event(tesa_event_t *event);
static void *allocate_payload(size_t size);
static void *allocate_payload_from_isr(size_t size);
static void free_payload(void *payload);
static tesa_event_channel_t *find_channel(tesa_event_channel_id_t channel_id);
static uint32_t get_system_time_ms(void);
static uint32_t get_system_time_ms_from_isr(void);
static void update_subscriber_stats(tesa_event_channel_t *channel,
                                    uint8_t index, bool success);
static void update_subscriber_stats_with_time(tesa_event_channel_t *channel,
                                              uint8_t index, bool success,
                                              uint32_t timestamp_ms);

tesa_event_bus_result_t tesa_event_bus_init(void) {
  if (false != event_bus_initialized) {
    return TESA_EVENT_BUS_ERROR_INVALID_PARAM;
  }

  for (uint8_t i = 0U; i < TESA_EVENT_BUS_MAX_CHANNELS; i++) {
    channel_registry[i].channel_id = 0U;
    channel_registry[i].channel_name = NULL;
    channel_registry[i].registered = false;
    channel_registry[i].subscriber_count = 0U;
    channel_registry[i].config.queue_policy = TESA_EVENT_BUS_QUEUE_DROP_NEWEST;
    channel_registry[i].config.queue_timeout_ticks =
        pdMS_TO_TICKS(TESA_EVENT_BUS_DEFAULT_QUEUE_TIMEOUT_MS);
    for (uint8_t j = 0U; j < TESA_EVENT_BUS_MAX_SUBSCRIBERS_PER_CHANNEL; j++) {
      channel_registry[i].subscribers[j] = NULL;
      channel_registry[i].subscriber_stats[j].posts_successful = 0U;
      channel_registry[i].subscriber_stats[j].posts_dropped = 0U;
      channel_registry[i].subscriber_stats[j].last_drop_timestamp_ms = 0U;
    }
  }

  for (uint8_t i = 0U; i < TESA_EVENT_BUS_MAX_SUBSCRIBERS_PER_CHANNEL; i++) {
    post_events_buffer[i] = NULL;
    post_payloads_buffer[i] = NULL;
  }

  for (uint8_t i = 0U; i < TESA_EVENT_BUS_EVENT_POOL_SIZE; i++) {
    event_pool_used[i] = false;
    payload_buffer_used[i] = false;
  }

  registered_channel_count = 0U;
  event_bus_initialized = true;

  return TESA_EVENT_BUS_SUCCESS;
}

tesa_event_bus_result_t
tesa_event_bus_register_channel(tesa_event_channel_id_t channel_id,
                                const char *channel_name) {
  tesa_event_bus_channel_config_t default_config = {
      .queue_policy = TESA_EVENT_BUS_QUEUE_DROP_NEWEST,
      .queue_timeout_ticks =
          pdMS_TO_TICKS(TESA_EVENT_BUS_DEFAULT_QUEUE_TIMEOUT_MS)};
  return tesa_event_bus_register_channel_with_config(channel_id, channel_name,
                                                     &default_config);
}

tesa_event_bus_result_t tesa_event_bus_register_channel_with_config(
    tesa_event_channel_id_t channel_id, const char *channel_name,
    const tesa_event_bus_channel_config_t *config) {

  if ((false == event_bus_initialized) || (0U == channel_id) ||
      (NULL == channel_name)) {
    return TESA_EVENT_BUS_ERROR_INVALID_PARAM;
  }

  tesa_event_bus_channel_config_t default_config = {
      .queue_policy = TESA_EVENT_BUS_QUEUE_DROP_NEWEST,
      .queue_timeout_ticks =
          pdMS_TO_TICKS(TESA_EVENT_BUS_DEFAULT_QUEUE_TIMEOUT_MS)};
  if (NULL == config) {
    config = &default_config;
  }

  taskENTER_CRITICAL();

  tesa_event_channel_t *existing = find_channel(channel_id);
  if ((NULL != existing) && (false != existing->registered)) {
    taskEXIT_CRITICAL();
    return TESA_EVENT_BUS_ERROR_INVALID_PARAM;
  }

  if (TESA_EVENT_BUS_MAX_CHANNELS <= registered_channel_count) {
    taskEXIT_CRITICAL();
    return TESA_EVENT_BUS_ERROR_CHANNEL_FULL;
  }

  for (uint8_t i = 0U; i < TESA_EVENT_BUS_MAX_CHANNELS; i++) {
    if (false == channel_registry[i].registered) {
      channel_registry[i].channel_id = channel_id;
      channel_registry[i].channel_name = channel_name;
      channel_registry[i].registered = true;
      channel_registry[i].subscriber_count = 0U;
      channel_registry[i].config = *config;
      for (uint8_t j = 0U; j < TESA_EVENT_BUS_MAX_SUBSCRIBERS_PER_CHANNEL;
           j++) {
        channel_registry[i].subscriber_stats[j].posts_successful = 0;
        channel_registry[i].subscriber_stats[j].posts_dropped = 0;
        channel_registry[i].subscriber_stats[j].last_drop_timestamp_ms = 0;
      }
      registered_channel_count++;
      taskEXIT_CRITICAL();
      return TESA_EVENT_BUS_SUCCESS;
    }
  }

  taskEXIT_CRITICAL();
  return TESA_EVENT_BUS_ERROR_CHANNEL_FULL;
}

tesa_event_bus_result_t
tesa_event_bus_unregister_channel(tesa_event_channel_id_t channel_id) {

  if ((false == event_bus_initialized) || (0U == channel_id)) {
    return TESA_EVENT_BUS_ERROR_INVALID_PARAM;
  }

  taskENTER_CRITICAL();

  tesa_event_channel_t *channel = find_channel(channel_id);
  if ((NULL == channel) || (false == channel->registered)) {
    taskEXIT_CRITICAL();
    return TESA_EVENT_BUS_ERROR_CHANNEL_NOT_FOUND;
  }

  if (0U < channel->subscriber_count) {
    taskEXIT_CRITICAL();
    return TESA_EVENT_BUS_ERROR_HAS_SUBSCRIBERS;
  }

  channel->registered = false;
  channel->channel_id = 0U;
  channel->channel_name = NULL;
  channel->subscriber_count = 0U;
  if (0U < registered_channel_count) {
    registered_channel_count--;
  }

  taskEXIT_CRITICAL();
  return TESA_EVENT_BUS_SUCCESS;
}

tesa_event_bus_result_t
tesa_event_bus_subscribe(tesa_event_channel_id_t channel_id,
                         QueueHandle_t queue_handle) {

  if ((false == event_bus_initialized) || (0U == channel_id) ||
      (NULL == queue_handle)) {
    return TESA_EVENT_BUS_ERROR_INVALID_PARAM;
  }

  taskENTER_CRITICAL();

  tesa_event_channel_t *channel = find_channel(channel_id);
  if ((NULL == channel) || (false == channel->registered)) {
    taskEXIT_CRITICAL();
    return TESA_EVENT_BUS_ERROR_CHANNEL_NOT_FOUND;
  }

  for (uint8_t i = 0U; i < channel->subscriber_count; i++) {
    if (queue_handle == channel->subscribers[i]) {
      taskEXIT_CRITICAL();
      return TESA_EVENT_BUS_SUCCESS;
    }
  }

  if (TESA_EVENT_BUS_MAX_SUBSCRIBERS_PER_CHANNEL <= channel->subscriber_count) {
    taskEXIT_CRITICAL();
    return TESA_EVENT_BUS_ERROR_SUBSCRIBER_FULL;
  }

  channel->subscribers[channel->subscriber_count] = queue_handle;
  channel->subscriber_count++;

  taskEXIT_CRITICAL();
  return TESA_EVENT_BUS_SUCCESS;
}

tesa_event_bus_result_t
tesa_event_bus_unsubscribe(tesa_event_channel_id_t channel_id,
                           QueueHandle_t queue_handle) {

  if ((false == event_bus_initialized) || (0U == channel_id) ||
      (NULL == queue_handle)) {
    return TESA_EVENT_BUS_ERROR_INVALID_PARAM;
  }

  taskENTER_CRITICAL();

  tesa_event_channel_t *channel = find_channel(channel_id);
  if ((NULL == channel) || (false == channel->registered)) {
    taskEXIT_CRITICAL();
    return TESA_EVENT_BUS_ERROR_CHANNEL_NOT_FOUND;
  }

  for (uint8_t i = 0U; i < channel->subscriber_count; i++) {
    if (queue_handle == channel->subscribers[i]) {
      for (uint8_t j = i; j < (channel->subscriber_count - 1U); j++) {
        channel->subscribers[j] = channel->subscribers[j + 1];
      }
      channel->subscribers[channel->subscriber_count - 1] = NULL;
      channel->subscriber_count--;
      taskEXIT_CRITICAL();
      return TESA_EVENT_BUS_SUCCESS;
    }
  }

  taskEXIT_CRITICAL();
  return TESA_EVENT_BUS_ERROR_INVALID_PARAM;
}

tesa_event_bus_result_t tesa_event_bus_post(tesa_event_channel_id_t channel_id,
                                            tesa_event_type_t event_type,
                                            const void *payload,
                                            size_t payload_size) {

  if ((false == event_bus_initialized) || (0U == channel_id)) {
    return TESA_EVENT_BUS_ERROR_INVALID_PARAM;
  }

  if (TESA_EVENT_BUS_MAX_PAYLOAD_SIZE < payload_size) {
    return TESA_EVENT_BUS_ERROR_PAYLOAD_TOO_LARGE;
  }

  if ((0U < payload_size) && (NULL == payload)) {
    return TESA_EVENT_BUS_ERROR_INVALID_PARAM;
  }

  taskENTER_CRITICAL();

  tesa_event_channel_t *channel = find_channel(channel_id);
  if ((NULL == channel) || (false == channel->registered)) {
    taskEXIT_CRITICAL();
    return TESA_EVENT_BUS_ERROR_CHANNEL_NOT_FOUND;
  }

  if (0U == channel->subscriber_count) {
    taskEXIT_CRITICAL();
    return TESA_EVENT_BUS_SUCCESS;
  }

  uint32_t timestamp_ms = get_system_time_ms();
  TaskHandle_t source_task = xTaskGetCurrentTaskHandle();
  uint8_t subscriber_count = channel->subscriber_count;
  tesa_event_bus_queue_policy_t policy = channel->config.queue_policy;
  TickType_t timeout = channel->config.queue_timeout_ticks;

  taskEXIT_CRITICAL();

  for (uint8_t i = 0U; i < subscriber_count; i++) {
    post_events_buffer[i] = allocate_event();
    if (NULL == post_events_buffer[i]) {
      for (uint8_t j = 0U; j < i; j++) {
        free_event(post_events_buffer[j]);
        post_events_buffer[j] = NULL;
      }
      return TESA_EVENT_BUS_ERROR_MEMORY;
    }

    post_events_buffer[i]->channel_id = channel_id;
    post_events_buffer[i]->event_type = event_type;
    post_events_buffer[i]->payload_size = payload_size;
    post_events_buffer[i]->timestamp_ms = timestamp_ms;
    post_events_buffer[i]->source_task = source_task;
    post_events_buffer[i]->from_isr = false;

    if ((NULL != payload) && (0U < payload_size)) {
      post_payloads_buffer[i] = allocate_payload(payload_size);
      if (NULL == post_payloads_buffer[i]) {
        for (uint8_t j = 0U; j <= i; j++) {
          free_event(post_events_buffer[j]);
          post_events_buffer[j] = NULL;
        }
        for (uint8_t j = 0U; j < i; j++) {
          free_payload(post_payloads_buffer[j]);
          post_payloads_buffer[j] = NULL;
        }
        return TESA_EVENT_BUS_ERROR_MEMORY;
      }
      (void)memcpy(post_payloads_buffer[i], payload, payload_size);
      post_events_buffer[i]->payload = post_payloads_buffer[i];
    } else {
      post_payloads_buffer[i] = NULL;
      post_events_buffer[i]->payload = NULL;
    }
  }

  BaseType_t all_queues_ok = pdTRUE;
  BaseType_t any_queues_ok = pdFALSE;
  for (uint8_t i = 0U; i < subscriber_count; i++) {
    BaseType_t queue_result = pdFALSE;

    if (NULL == channel->subscribers[i]) {
      all_queues_ok = pdFALSE;
      taskENTER_CRITICAL();
      update_subscriber_stats(channel, i, false);
      taskEXIT_CRITICAL();
      free_payload(post_events_buffer[i]->payload);
      free_event(post_events_buffer[i]);
      post_events_buffer[i] = NULL;
      continue;
    }

    switch (policy) {
    case TESA_EVENT_BUS_QUEUE_DROP_NEWEST:
      queue_result =
          xQueueSend(channel->subscribers[i], &post_events_buffer[i], 0);
      break;
    case TESA_EVENT_BUS_QUEUE_DROP_OLDEST:
      queue_result =
          xQueueOverwrite(channel->subscribers[i], &post_events_buffer[i]);
      break;
    case TESA_EVENT_BUS_QUEUE_NO_DROP:
      queue_result =
          xQueueSend(channel->subscribers[i], &post_events_buffer[i], timeout);
      break;
    case TESA_EVENT_BUS_QUEUE_WAIT:
      queue_result =
          xQueueSend(channel->subscribers[i], &post_events_buffer[i], timeout);
      break;
    default:
      queue_result = pdFALSE;
      break;
    }

    if (pdTRUE != queue_result) {
      all_queues_ok = pdFALSE;
      taskENTER_CRITICAL();
      update_subscriber_stats(channel, i, false);
      taskEXIT_CRITICAL();
      free_payload(post_events_buffer[i]->payload);
      free_event(post_events_buffer[i]);
      post_events_buffer[i] = NULL;
    } else {
      any_queues_ok = pdTRUE;
      taskENTER_CRITICAL();
      update_subscriber_stats(channel, i, true);
      taskEXIT_CRITICAL();
    }
  }

  for (uint8_t i = 0U; i < subscriber_count; i++) {
    post_events_buffer[i] = NULL;
    post_payloads_buffer[i] = NULL;
  }

  if (pdTRUE != all_queues_ok) {
    if (pdTRUE == any_queues_ok) {
      return TESA_EVENT_BUS_ERROR_PARTIAL_SUCCESS;
    }
    return TESA_EVENT_BUS_ERROR_QUEUE_FULL;
  }

  return TESA_EVENT_BUS_SUCCESS;
}

BaseType_t tesa_event_bus_post_from_isr(tesa_event_channel_id_t channel_id,
                                        tesa_event_type_t event_type,
                                        const void *payload,
                                        size_t payload_size,
                                        BaseType_t *pxHigherPriorityTaskWoken) {

  if ((false == event_bus_initialized) || (0U == channel_id) ||
      (NULL == pxHigherPriorityTaskWoken)) {
    return pdFALSE;
  }

  BaseType_t xYieldRequired = pdFALSE;
  BaseType_t xReturn = pdTRUE;

  UBaseType_t saved_ux_saved_interrupt_status = taskENTER_CRITICAL_FROM_ISR();

  tesa_event_channel_t *channel = find_channel(channel_id);
  if ((NULL == channel) || (false == channel->registered)) {
    taskEXIT_CRITICAL_FROM_ISR(saved_ux_saved_interrupt_status);
    return pdFALSE;
  }

  if (0U == channel->subscriber_count) {
    taskEXIT_CRITICAL_FROM_ISR(saved_ux_saved_interrupt_status);
    return pdTRUE;
  }

  uint32_t timestamp_ms = get_system_time_ms_from_isr();
  uint8_t subscriber_count = channel->subscriber_count;
  tesa_event_bus_queue_policy_t policy = channel->config.queue_policy;

  for (uint8_t i = 0U; i < subscriber_count; i++) {
    post_events_buffer[i] = allocate_event();
    if (NULL == post_events_buffer[i]) {
      for (uint8_t j = 0U; j < i; j++) {
        free_event(post_events_buffer[j]);
        post_events_buffer[j] = NULL;
      }
      taskEXIT_CRITICAL_FROM_ISR(saved_ux_saved_interrupt_status);
      return pdFALSE;
    }

    post_events_buffer[i]->channel_id = channel_id;
    post_events_buffer[i]->event_type = event_type;
    post_events_buffer[i]->payload_size = payload_size;
    post_events_buffer[i]->timestamp_ms = timestamp_ms;
    post_events_buffer[i]->source_task = NULL;
    post_events_buffer[i]->from_isr = true;

    if ((NULL != payload) && (0U < payload_size)) {
      post_payloads_buffer[i] = allocate_payload_from_isr(payload_size);
      if (NULL == post_payloads_buffer[i]) {
        for (uint8_t j = 0U; j <= i; j++) {
          free_event(post_events_buffer[j]);
          post_events_buffer[j] = NULL;
        }
        for (uint8_t j = 0U; j < i; j++) {
          free_payload(post_payloads_buffer[j]);
          post_payloads_buffer[j] = NULL;
        }
        taskEXIT_CRITICAL_FROM_ISR(saved_ux_saved_interrupt_status);
        return pdFALSE;
      }
      (void)memcpy(post_payloads_buffer[i], payload, payload_size);
      post_events_buffer[i]->payload = post_payloads_buffer[i];
    } else {
      post_payloads_buffer[i] = NULL;
      post_events_buffer[i]->payload = NULL;
    }
  }

  taskEXIT_CRITICAL_FROM_ISR(saved_ux_saved_interrupt_status);

  for (uint8_t i = 0U; i < subscriber_count; i++) {
    BaseType_t xHigherPriorityTaskWokenLocal = pdFALSE;
    BaseType_t queue_result = pdFALSE;

    if (NULL == channel->subscribers[i]) {
      xReturn = pdFALSE;
      UBaseType_t saved_int_status = taskENTER_CRITICAL_FROM_ISR();
      update_subscriber_stats_with_time(channel, i, false, timestamp_ms);
      taskEXIT_CRITICAL_FROM_ISR(saved_int_status);
      free_payload(post_events_buffer[i]->payload);
      free_event(post_events_buffer[i]);
      post_events_buffer[i] = NULL;
      continue;
    }

    switch (policy) {
    case TESA_EVENT_BUS_QUEUE_DROP_NEWEST:
    case TESA_EVENT_BUS_QUEUE_NO_DROP:
    case TESA_EVENT_BUS_QUEUE_WAIT:
      queue_result =
          xQueueSendFromISR(channel->subscribers[i], &post_events_buffer[i],
                            &xHigherPriorityTaskWokenLocal);
      break;
    case TESA_EVENT_BUS_QUEUE_DROP_OLDEST:
      queue_result = xQueueOverwriteFromISR(channel->subscribers[i],
                                            &post_events_buffer[i],
                                            &xHigherPriorityTaskWokenLocal);
      break;
    default:
      queue_result = pdFALSE;
      break;
    }

    if (pdTRUE != queue_result) {
      xReturn = pdFALSE;
      UBaseType_t saved_int_status = taskENTER_CRITICAL_FROM_ISR();
      update_subscriber_stats_with_time(channel, i, false, timestamp_ms);
      taskEXIT_CRITICAL_FROM_ISR(saved_int_status);
      free_payload(post_events_buffer[i]->payload);
      free_event(post_events_buffer[i]);
      post_events_buffer[i] = NULL;
    } else {
      UBaseType_t saved_int_status = taskENTER_CRITICAL_FROM_ISR();
      update_subscriber_stats_with_time(channel, i, true, timestamp_ms);
      taskEXIT_CRITICAL_FROM_ISR(saved_int_status);
    }

    if (pdTRUE == xHigherPriorityTaskWokenLocal) {
      xYieldRequired = pdTRUE;
    }
  }

  for (uint8_t i = 0U; i < subscriber_count; i++) {
    post_events_buffer[i] = NULL;
    post_payloads_buffer[i] = NULL;
  }

  if (pdTRUE == xYieldRequired) {
    *pxHigherPriorityTaskWoken = pdTRUE;
  }

  return xReturn;
}

uint32_t tesa_event_bus_get_timestamp_ms(void) { return get_system_time_ms(); }

void tesa_event_bus_free_event(tesa_event_t *event) {
  if (NULL == event) {
    return;
  }

  free_payload(event->payload);
  free_event(event);
}

tesa_event_bus_result_t
tesa_event_bus_get_subscriber_stats(tesa_event_channel_id_t channel_id,
                                    QueueHandle_t queue_handle,
                                    tesa_event_bus_subscriber_stats_t *stats) {

  if ((false == event_bus_initialized) || (0U == channel_id) ||
      (NULL == queue_handle) || (NULL == stats)) {
    return TESA_EVENT_BUS_ERROR_INVALID_PARAM;
  }

  taskENTER_CRITICAL();

  tesa_event_channel_t *channel = find_channel(channel_id);
  if ((NULL == channel) || (false == channel->registered)) {
    taskEXIT_CRITICAL();
    return TESA_EVENT_BUS_ERROR_CHANNEL_NOT_FOUND;
  }

  for (uint8_t i = 0U; i < channel->subscriber_count; i++) {
    if (queue_handle == channel->subscribers[i]) {
      *stats = channel->subscriber_stats[i];
      taskEXIT_CRITICAL();
      return TESA_EVENT_BUS_SUCCESS;
    }
  }

  taskEXIT_CRITICAL();
  return TESA_EVENT_BUS_ERROR_INVALID_PARAM;
}

tesa_event_bus_result_t
tesa_event_bus_get_channel_stats(tesa_event_channel_id_t channel_id,
                                 tesa_event_bus_subscriber_stats_t *total_stats,
                                 uint8_t *subscriber_count) {

  if ((false == event_bus_initialized) || (0U == channel_id) ||
      (NULL == total_stats) || (NULL == subscriber_count)) {
    return TESA_EVENT_BUS_ERROR_INVALID_PARAM;
  }

  taskENTER_CRITICAL();

  tesa_event_channel_t *channel = find_channel(channel_id);
  if ((NULL == channel) || (false == channel->registered)) {
    taskEXIT_CRITICAL();
    return TESA_EVENT_BUS_ERROR_CHANNEL_NOT_FOUND;
  }

  total_stats->posts_successful = 0U;
  total_stats->posts_dropped = 0U;
  total_stats->last_drop_timestamp_ms = 0U;

  for (uint8_t i = 0U; i < channel->subscriber_count; i++) {
    total_stats->posts_successful =
        total_stats->posts_successful +
        channel->subscriber_stats[i].posts_successful;
    total_stats->posts_dropped =
        total_stats->posts_dropped + channel->subscriber_stats[i].posts_dropped;
    if (channel->subscriber_stats[i].last_drop_timestamp_ms >
        total_stats->last_drop_timestamp_ms) {
      total_stats->last_drop_timestamp_ms =
          channel->subscriber_stats[i].last_drop_timestamp_ms;
    }
  }

  *subscriber_count = channel->subscriber_count;

  taskEXIT_CRITICAL();
  return TESA_EVENT_BUS_SUCCESS;
}

static void update_subscriber_stats(tesa_event_channel_t *channel,
                                    uint8_t index, bool success) {
  uint32_t timestamp = get_system_time_ms();
  update_subscriber_stats_with_time(channel, index, success, timestamp);
}

static void update_subscriber_stats_with_time(tesa_event_channel_t *channel,
                                              uint8_t index, bool success,
                                              uint32_t timestamp_ms) {
  if ((NULL == channel) || (channel->subscriber_count <= index)) {
    return;
  }

  if (false != success) {
    if (TESA_EVENT_BUS_STATS_MAX_VALUE >
        channel->subscriber_stats[index].posts_successful) {
      channel->subscriber_stats[index].posts_successful =
          channel->subscriber_stats[index].posts_successful + 1U;
    }
  } else {
    if (TESA_EVENT_BUS_STATS_MAX_VALUE >
        channel->subscriber_stats[index].posts_dropped) {
      channel->subscriber_stats[index].posts_dropped =
          channel->subscriber_stats[index].posts_dropped + 1U;
    }
    channel->subscriber_stats[index].last_drop_timestamp_ms = timestamp_ms;
  }
}

static tesa_event_t *allocate_event(void) {
  for (uint8_t i = 0U; i < TESA_EVENT_BUS_EVENT_POOL_SIZE; i++) {
    if (false == event_pool_used[i]) {
      event_pool_used[i] = true;
      return &event_pool[i];
    }
  }
  return NULL;
}

static void free_event(tesa_event_t *event) {
  if (NULL == event) {
    return;
  }

  for (uint8_t i = 0U; i < TESA_EVENT_BUS_EVENT_POOL_SIZE; i++) {
    if (event == &event_pool[i]) {
      event_pool_used[i] = false;
      event->payload = NULL;
      event->payload_size = 0U;
      break;
    }
  }
}

static void *allocate_payload(size_t size) {
  if (0U == size) {
    return NULL;
  }

  if (TESA_EVENT_BUS_MAX_PAYLOAD_SIZE < size) {
    return NULL;
  }

  for (uint8_t i = 0U; i < TESA_EVENT_BUS_EVENT_POOL_SIZE; i++) {
    if (false == payload_buffer_used[i]) {
      payload_buffer_used[i] = true;
      return payload_buffers[i];
    }
  }

  return NULL;
}

static void *allocate_payload_from_isr(size_t size) {
  if (0U == size) {
    return NULL;
  }

  if (TESA_EVENT_BUS_PAYLOAD_BUFFER_SIZE < size) {
    return NULL;
  }

  for (uint8_t i = 0U; i < TESA_EVENT_BUS_EVENT_POOL_SIZE; i++) {
    if (false == payload_buffer_used[i]) {
      payload_buffer_used[i] = true;
      return payload_buffers[i];
    }
  }

  return NULL;
}

static void free_payload(void *payload) {
  if (NULL == payload) {
    return;
  }

  for (uint8_t i = 0U; i < TESA_EVENT_BUS_EVENT_POOL_SIZE; i++) {
    if (payload == payload_buffers[i]) {
      payload_buffer_used[i] = false;
      return;
    }
  }
}

static tesa_event_channel_t *find_channel(tesa_event_channel_id_t channel_id) {
  for (uint8_t i = 0U; i < TESA_EVENT_BUS_MAX_CHANNELS; i++) {
    if ((false != channel_registry[i].registered) &&
        (channel_id == channel_registry[i].channel_id)) {
      return &channel_registry[i];
    }
  }
  return NULL;
}

static uint32_t get_system_time_ms(void) {
  TickType_t tick_count = xTaskGetTickCount();
  return (uint32_t)((uint32_t)tick_count * (uint32_t)portTICK_PERIOD_MS);
}

static uint32_t get_system_time_ms_from_isr(void) {
  TickType_t tick_count = xTaskGetTickCountFromISR();
  return (uint32_t)((uint32_t)tick_count * (uint32_t)portTICK_PERIOD_MS);
}

uint32_t tesa_event_bus_check_timestamp_rollover(uint32_t timestamp_ms) {
  if (TESA_EVENT_BUS_TIMESTAMP_ROLLOVER_MS <= timestamp_ms) {
    return 1U;
  }
  return 0U;
}
