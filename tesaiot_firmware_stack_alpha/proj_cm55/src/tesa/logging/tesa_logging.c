#include "tesa_logging.h"
#include "../utils/tesa_datetime.h"
#include "queue.h"
#include "task.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
  tesa_log_level_t level;
  char owner[32U];
  char message[188U];
} tesa_log_message_t;

static const char *level_prefixes[TESA_LOG_LEVEL_COUNT] = {
    [TESA_LOG_VERBOSE] = "VERBOSE", [TESA_LOG_DEBUG] = "DEBUG",
    [TESA_LOG_INFO] = "INFO",       [TESA_LOG_WARNING] = "WARN",
    [TESA_LOG_ERROR] = "ERROR",     [TESA_LOG_CRITICAL] = "CRITICAL"};

static QueueHandle_t logging_queue = NULL;
static tesa_logging_config_t logging_config;
static bool logging_initialized = false;

static void logging_task(void *pvParameters);

static int32_t format_timestamp(tesa_log_timestamp_format_t format,
                                uint32_t timestamp_ms, char *buffer,
                                size_t buffer_size) {
  time_t now;
  struct tm *timeinfo;
  int32_t chars_written = 0;

  switch (format) {
  case TESA_LOG_TIMESTAMP_MS:
    chars_written =
        snprintf(buffer, buffer_size, "[%lu]", (unsigned long)timestamp_ms);
    break;

  case TESA_LOG_TIMESTAMP_FULL_DATETIME:
    (void)time(&now);
    timeinfo = localtime(&now);
    if (NULL != timeinfo) {
      chars_written =
          snprintf(buffer, buffer_size, "[%04d-%02d-%02d %02d:%02d:%02d.%03lu]",
                   timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
                   timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
                   timeinfo->tm_sec, timestamp_ms % 1000UL);
    }
    break;

  case TESA_LOG_TIMESTAMP_TIME_ONLY:
    (void)time(&now);
    timeinfo = localtime(&now);
    if (NULL != timeinfo) {
      chars_written = snprintf(buffer, buffer_size, "[%02d:%02d:%02d.%03lu]",
                               timeinfo->tm_hour, timeinfo->tm_min,
                               timeinfo->tm_sec, timestamp_ms % 1000UL);
    }
    break;

  default:
    buffer[0] = '\0';
    chars_written = 0;
    break;
  }

  return chars_written;
}

static void parse_timestamp(const char *timestamp_str,
                            tesa_log_timestamp_format_t format, char *date_buf,
                            size_t date_buf_size, char *time_buf,
                            size_t time_buf_size) {
  if ((NULL == timestamp_str) || (NULL == date_buf) || (NULL == time_buf)) {
    if (NULL != date_buf) {
      date_buf[0] = '\0';
    }
    if (NULL != time_buf) {
      time_buf[0] = '\0';
    }
    return;
  }

  date_buf[0] = '\0';
  time_buf[0] = '\0';

  switch (format) {
  case TESA_LOG_TIMESTAMP_FULL_DATETIME: {
    int32_t year = 0;
    int32_t month = 0;
    int32_t day = 0;
    int32_t hour = 0;
    int32_t min = 0;
    int32_t sec = 0;
    unsigned long msec = 0UL;

    if (7 == sscanf(timestamp_str,
                    "[%04" PRId32 "-%02" PRId32 "-%02" PRId32 " %02" PRId32
                    ":%02" PRId32 ":%02" PRId32 ".%03lu]",
                    &year, &month, &day, &hour, &min, &sec, &msec)) {
      (void)snprintf(date_buf, date_buf_size,
                     "%04" PRId32 "-%02" PRId32 "-%02" PRId32, year, month,
                     day);
      (void)snprintf(time_buf, time_buf_size,
                     "%02" PRId32 ":%02" PRId32 ":%02" PRId32 ".%03lu", hour,
                     min, sec, msec);
    }
    break;
  }

  case TESA_LOG_TIMESTAMP_TIME_ONLY: {
    int32_t hour = 0;
    int32_t min = 0;
    int32_t sec = 0;
    unsigned long msec = 0UL;

    if (4 == sscanf(timestamp_str,
                    "[%02" PRId32 ":%02" PRId32 ":%02" PRId32 ".%03lu]", &hour,
                    &min, &sec, &msec)) {
      date_buf[0] = '\0';
      (void)snprintf(time_buf, time_buf_size,
                     "%02" PRId32 ":%02" PRId32 ":%02" PRId32 ".%03lu", hour,
                     min, sec, (unsigned long)msec);
    }
    break;
  }

  case TESA_LOG_TIMESTAMP_MS: {
    unsigned long msec = 0UL;

    if (1 == sscanf(timestamp_str, "[%lu]", &msec)) {
      date_buf[0] = '\0';
      (void)snprintf(time_buf, time_buf_size, "%lu", (unsigned long)msec);
    }
    break;
  }

  default:
    break;
  }
}

static void logging_task(void *pvParameters) {
  QueueHandle_t queue = (QueueHandle_t)pvParameters;
  tesa_event_t *event = NULL;
  tesa_log_message_t *log_msg = NULL;
  const char *level_prefix = NULL;
  uint32_t timestamp_ms = 0U;
  char timestamp_buffer[40U];
  char date_buffer[16U];
  char time_buffer[16U];
  char output_buffer[384U];
  bool timestamp_enabled = true;
  tesa_log_timestamp_format_t timestamp_format = TESA_LOG_TIMESTAMP_MS;

  (void)pvParameters;

  for (;;) {
    if (pdTRUE == xQueueReceive(queue, &event, pdMS_TO_TICKS(1000U))) {
      if ((NULL != event) && (NULL != event->payload) &&
          (sizeof(tesa_log_message_t) == event->payload_size)) {
        log_msg = (tesa_log_message_t *)event->payload;

        if (TESA_LOG_LEVEL_COUNT > log_msg->level) {
          taskENTER_CRITICAL();
          timestamp_enabled = logging_config.enable_timestamp;
          timestamp_format = logging_config.timestamp_format;
          taskEXIT_CRITICAL();

          level_prefix = level_prefixes[log_msg->level];

          date_buffer[0] = '\0';
          time_buffer[0] = '\0';

          if (timestamp_enabled) {
            timestamp_ms = event->timestamp_ms;
            if (0 < format_timestamp(timestamp_format, timestamp_ms,
                                     timestamp_buffer,
                                     sizeof(timestamp_buffer))) {
              parse_timestamp(timestamp_buffer, timestamp_format, date_buffer,
                              sizeof(date_buffer), time_buffer,
                              sizeof(time_buffer));
            }
          }

          (void)snprintf(output_buffer, sizeof(output_buffer),
                         "[logging|%s|%s|%s|%s|%s]\r\n", level_prefix,
                         date_buffer, time_buffer, log_msg->owner,
                         log_msg->message);
          (void)printf("%s", output_buffer);
          (void)fflush(stdout);
        }

        tesa_event_bus_free_event(event);
        event = NULL;
      }
    }
  }
}

tesa_event_bus_result_t tesa_logging_init(void) {
  tesa_logging_config_t default_config;

  if (TESA_LOG_LEVEL_COUNT <= TESA_LOGGING_DEFAULT_LEVEL) {
    return TESA_EVENT_BUS_ERROR_INVALID_PARAM;
  }

  default_config.min_level = (tesa_log_level_t)TESA_LOGGING_DEFAULT_LEVEL;
#if (1U == TESA_LOGGING_ENABLE_COLORS)
  default_config.enable_colors = true;
#else
  default_config.enable_colors = false;
#endif
#if (1U == TESA_LOGGING_ENABLE_TIMESTAMP)
  default_config.enable_timestamp = true;
#else
  default_config.enable_timestamp = false;
#endif
  default_config.timestamp_format =
      (tesa_log_timestamp_format_t)TESA_LOGGING_TIMESTAMP_FORMAT;

  return tesa_logging_init_with_config(&default_config);
}

tesa_event_bus_result_t
tesa_logging_init_with_config(const tesa_logging_config_t *config) {
  tesa_event_bus_result_t result;
  BaseType_t task_result;

  if (NULL == config) {
    (void)printf("[LOG_INIT] ERROR: config is NULL\r\n");
    (void)fflush(stdout);
    return TESA_EVENT_BUS_ERROR_INVALID_PARAM;
  }

  if (TESA_LOG_LEVEL_COUNT <= config->min_level) {
    (void)printf("[LOG_INIT] ERROR: invalid min_level: %u (max: %u)\r\n",
                 (unsigned int)config->min_level,
                 (unsigned int)(TESA_LOG_LEVEL_COUNT - 1U));
    (void)fflush(stdout);
    return TESA_EVENT_BUS_ERROR_INVALID_PARAM;
  }

  if (config->timestamp_format >= 3U) {
    (void)printf("[LOG_INIT] ERROR: invalid timestamp_format: %u\r\n",
                 (unsigned int)config->timestamp_format);
    (void)fflush(stdout);
    return TESA_EVENT_BUS_ERROR_INVALID_PARAM;
  }

  if (false != logging_initialized) {
    return TESA_EVENT_BUS_SUCCESS;
  }

  result = tesa_event_bus_register_channel(TESA_LOGGING_CHANNEL_ID, "Logging");
  if (TESA_EVENT_BUS_SUCCESS != result) {
    (void)printf(
        "[LOG_INIT] ERROR: register_channel failed: %d (channel: 0x%04X)\r\n",
        result, (unsigned int)TESA_LOGGING_CHANNEL_ID);
    (void)fflush(stdout);
    return result;
  }

  logging_queue =
      xQueueCreate(TESA_LOGGING_QUEUE_LENGTH, sizeof(tesa_event_t *));
  if (NULL == logging_queue) {
    (void)printf("[LOG_INIT] ERROR: queue creation failed\r\n");
    (void)fflush(stdout);
    return TESA_EVENT_BUS_ERROR_MEMORY;
  }

  result = tesa_event_bus_subscribe(TESA_LOGGING_CHANNEL_ID, logging_queue);
  if (TESA_EVENT_BUS_SUCCESS != result) {
    (void)printf("[LOG_INIT] ERROR: subscribe failed: %d\r\n", result);
    (void)fflush(stdout);
    vQueueDelete(logging_queue);
    logging_queue = NULL;
    return result;
  }

  logging_config.min_level = config->min_level;
  logging_config.enable_colors = config->enable_colors;
  logging_config.enable_timestamp = config->enable_timestamp;
  logging_config.timestamp_format = config->timestamp_format;

  task_result = xTaskCreate(logging_task, TESA_LOGGING_TASK_NAME,
                            TESA_LOGGING_TASK_STACK_SIZE, logging_queue,
                            TESA_LOGGING_TASK_PRIORITY, NULL);
  if (pdPASS != task_result) {
    vQueueDelete(logging_queue);
    logging_queue = NULL;
    return TESA_EVENT_BUS_ERROR_MEMORY;
  }

  logging_initialized = true;

  (void)printf("[LOGGING] Logging system initialized successfully\r\n");
  (void)fflush(stdout);

  return TESA_EVENT_BUS_SUCCESS;
}

static tesa_event_bus_result_t tesa_log_internal(tesa_log_level_t level,
                                                 const char *owner,
                                                 const char *format,
                                                 va_list args) {
  tesa_log_message_t log_msg;
  tesa_event_bus_result_t result;
  tesa_log_level_t current_min_level;

  if ((NULL == owner) || (NULL == format)) {
    return TESA_EVENT_BUS_ERROR_INVALID_PARAM;
  }

  if (TESA_LOG_LEVEL_COUNT <= level) {
    return TESA_EVENT_BUS_ERROR_INVALID_PARAM;
  }

  if (false == logging_initialized) {
    return TESA_EVENT_BUS_ERROR_CHANNEL_NOT_FOUND;
  }

  taskENTER_CRITICAL();
  current_min_level = logging_config.min_level;
  taskEXIT_CRITICAL();

  if (level < current_min_level) {
    return TESA_EVENT_BUS_SUCCESS;
  }

  log_msg.level = level;
  (void)strncpy(log_msg.owner, owner, sizeof(log_msg.owner) - 1U);
  log_msg.owner[sizeof(log_msg.owner) - 1U] = '\0';
  (void)vsnprintf(log_msg.message, sizeof(log_msg.message), format, args);
  log_msg.message[sizeof(log_msg.message) - 1U] = '\0';

  result = tesa_event_bus_post(TESA_LOGGING_CHANNEL_ID, TESA_LOGGING_EVENT_TYPE,
                               &log_msg, sizeof(tesa_log_message_t));

  if (TESA_EVENT_BUS_SUCCESS != result) {
    (void)printf("[LOG_ERROR] Failed to post log message: %d\r\n", result);
    (void)fflush(stdout);
  }

  return result;
}

tesa_event_bus_result_t tesa_log_verbose(const char *owner, const char *format,
                                         ...) {
  va_list args;
  tesa_event_bus_result_t result;

  va_start(args, format);
  result = tesa_log_internal(TESA_LOG_VERBOSE, owner, format, args);
  va_end(args);

  return result;
}

tesa_event_bus_result_t tesa_log_debug(const char *owner, const char *format,
                                       ...) {
  va_list args;
  tesa_event_bus_result_t result;

  va_start(args, format);
  result = tesa_log_internal(TESA_LOG_DEBUG, owner, format, args);
  va_end(args);

  return result;
}

tesa_event_bus_result_t tesa_log_info(const char *owner, const char *format,
                                      ...) {
  va_list args;
  tesa_event_bus_result_t result;

  va_start(args, format);
  result = tesa_log_internal(TESA_LOG_INFO, owner, format, args);
  va_end(args);

  return result;
}

tesa_event_bus_result_t tesa_log_warning(const char *owner, const char *format,
                                         ...) {
  va_list args;
  tesa_event_bus_result_t result;

  va_start(args, format);
  result = tesa_log_internal(TESA_LOG_WARNING, owner, format, args);
  va_end(args);

  return result;
}

tesa_event_bus_result_t tesa_log_error(const char *owner, const char *format,
                                       ...) {
  va_list args;
  tesa_event_bus_result_t result;

  va_start(args, format);
  result = tesa_log_internal(TESA_LOG_ERROR, owner, format, args);
  va_end(args);

  return result;
}

tesa_event_bus_result_t tesa_log_critical(const char *owner, const char *format,
                                          ...) {
  va_list args;
  tesa_event_bus_result_t result;

  va_start(args, format);
  result = tesa_log_internal(TESA_LOG_CRITICAL, owner, format, args);
  va_end(args);

  return result;
}

tesa_event_bus_result_t tesa_logging_set_level(tesa_log_level_t min_level) {
  if (TESA_LOG_LEVEL_COUNT <= min_level) {
    return TESA_EVENT_BUS_ERROR_INVALID_PARAM;
  }

  if (false == logging_initialized) {
    return TESA_EVENT_BUS_ERROR_CHANNEL_NOT_FOUND;
  }

  taskENTER_CRITICAL();
  logging_config.min_level = min_level;
  taskEXIT_CRITICAL();

  return TESA_EVENT_BUS_SUCCESS;
}

tesa_log_level_t tesa_logging_get_level(void) {
  tesa_log_level_t level;

  if (false == logging_initialized) {
    return TESA_LOG_LEVEL_COUNT;
  }

  taskENTER_CRITICAL();
  level = logging_config.min_level;
  taskEXIT_CRITICAL();

  return level;
}

tesa_event_bus_result_t tesa_logging_set_colors_enabled(bool enabled) {
  if (false == logging_initialized) {
    return TESA_EVENT_BUS_ERROR_CHANNEL_NOT_FOUND;
  }

  taskENTER_CRITICAL();
  logging_config.enable_colors = enabled;
  taskEXIT_CRITICAL();

  return TESA_EVENT_BUS_SUCCESS;
}

bool tesa_logging_get_colors_enabled(void) {
  bool enabled;

  if (false == logging_initialized) {
    return false;
  }

  taskENTER_CRITICAL();
  enabled = logging_config.enable_colors;
  taskEXIT_CRITICAL();

  return enabled;
}

tesa_event_bus_result_t tesa_logging_set_timestamp_enabled(bool enabled) {
  if (false == logging_initialized) {
    return TESA_EVENT_BUS_ERROR_CHANNEL_NOT_FOUND;
  }

  taskENTER_CRITICAL();
  logging_config.enable_timestamp = enabled;
  taskEXIT_CRITICAL();

  return TESA_EVENT_BUS_SUCCESS;
}

bool tesa_logging_get_timestamp_enabled(void) {
  bool enabled;

  if (false == logging_initialized) {
    return false;
  }

  taskENTER_CRITICAL();
  enabled = logging_config.enable_timestamp;
  taskEXIT_CRITICAL();

  return enabled;
}

tesa_event_bus_result_t
tesa_logging_set_timestamp_format(tesa_log_timestamp_format_t format) {
  if (3U <= format) {
    return TESA_EVENT_BUS_ERROR_INVALID_PARAM;
  }

  if (false == logging_initialized) {
    return TESA_EVENT_BUS_ERROR_CHANNEL_NOT_FOUND;
  }

  taskENTER_CRITICAL();
  logging_config.timestamp_format = format;
  taskEXIT_CRITICAL();

  return TESA_EVENT_BUS_SUCCESS;
}

tesa_log_timestamp_format_t tesa_logging_get_timestamp_format(void) {
  tesa_log_timestamp_format_t format;

  if (false == logging_initialized) {
    return TESA_LOG_TIMESTAMP_MS;
  }

  taskENTER_CRITICAL();
  format = logging_config.timestamp_format;
  taskEXIT_CRITICAL();

  return format;
}