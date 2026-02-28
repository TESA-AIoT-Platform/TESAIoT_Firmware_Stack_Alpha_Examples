#ifndef TESA_LOGGING_H
#define TESA_LOGGING_H

#include "../event_bus/tesa_event_bus.h"
#include "FreeRTOS.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "tesa_logging_config.h"

typedef enum {
  TESA_LOG_VERBOSE = 0U,
  TESA_LOG_DEBUG = 1U,
  TESA_LOG_INFO = 2U,
  TESA_LOG_WARNING = 3U,
  TESA_LOG_ERROR = 4U,
  TESA_LOG_CRITICAL = 5U,
  TESA_LOG_LEVEL_COUNT
} tesa_log_level_t;

typedef enum {
  TESA_LOG_TIMESTAMP_MS = 0U,
  TESA_LOG_TIMESTAMP_FULL_DATETIME = 1U,
  TESA_LOG_TIMESTAMP_TIME_ONLY = 2U
} tesa_log_timestamp_format_t;

typedef struct {
  tesa_log_level_t min_level;
  bool enable_colors;
  bool enable_timestamp;
  tesa_log_timestamp_format_t timestamp_format;
} tesa_logging_config_t;

tesa_event_bus_result_t tesa_logging_init(void);

tesa_event_bus_result_t
tesa_logging_init_with_config(const tesa_logging_config_t *config);

tesa_event_bus_result_t tesa_logging_set_level(tesa_log_level_t min_level);

tesa_log_level_t tesa_logging_get_level(void);

tesa_event_bus_result_t tesa_logging_set_colors_enabled(bool enabled);

bool tesa_logging_get_colors_enabled(void);

tesa_event_bus_result_t tesa_logging_set_timestamp_enabled(bool enabled);

bool tesa_logging_get_timestamp_enabled(void);

tesa_event_bus_result_t
tesa_logging_set_timestamp_format(tesa_log_timestamp_format_t format);

tesa_log_timestamp_format_t tesa_logging_get_timestamp_format(void);

tesa_event_bus_result_t tesa_log_verbose(const char *owner, const char *format,
                                         ...);

tesa_event_bus_result_t tesa_log_debug(const char *owner, const char *format,
                                       ...);

tesa_event_bus_result_t tesa_log_info(const char *owner, const char *format,
                                      ...);

tesa_event_bus_result_t tesa_log_warning(const char *owner, const char *format,
                                         ...);

tesa_event_bus_result_t tesa_log_error(const char *owner, const char *format,
                                       ...);

tesa_event_bus_result_t tesa_log_critical(const char *owner, const char *format,
                                          ...);

#define TESA_LOG_VERBOSE(owner, ...) tesa_log_verbose(owner, __VA_ARGS__)
#define TESA_LOG_DEBUG(owner, ...) tesa_log_debug(owner, __VA_ARGS__)
#define TESA_LOG_INFO(owner, ...) tesa_log_info(owner, __VA_ARGS__)
#define TESA_LOG_WARNING(owner, ...) tesa_log_warning(owner, __VA_ARGS__)
#define TESA_LOG_ERROR(owner, ...) tesa_log_error(owner, __VA_ARGS__)
#define TESA_LOG_CRITICAL(owner, ...) tesa_log_critical(owner, __VA_ARGS__)

#endif