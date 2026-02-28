#include "tesa_datetime.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "cy_result.h"
#include "mtb_hal_rtc.h"
#include "task.h"
#include <stdlib.h>
#include <string.h>

static mtb_hal_rtc_t *rtc_obj_ptr = NULL;

void tesa_datetime_init(mtb_hal_rtc_t *rtc_obj) { rtc_obj_ptr = rtc_obj; }

char *tesa_get_current_datetime(datetime_format_t format, char *buffer,
                                size_t buffer_size) {
  time_t now;
  struct tm *timeinfo;

  if (buffer == NULL || buffer_size == 0) {
    return NULL;
  }

  time(&now);
  timeinfo = localtime(&now);

  if (format == DATETIME_FORMAT_FULL) {
    snprintf(buffer, buffer_size, "%d-%02d-%02d %02d:%02d:%02d",
             timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
             timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  } else if (format == DATETIME_FORMAT_DATE) {
    snprintf(buffer, buffer_size, "%d-%02d-%02d", timeinfo->tm_year + 1900,
             timeinfo->tm_mon + 1, timeinfo->tm_mday);
  } else if (format == DATETIME_FORMAT_TIME) {
    snprintf(buffer, buffer_size, "%02d:%02d:%02d", timeinfo->tm_hour,
             timeinfo->tm_min, timeinfo->tm_sec);
  } else {
    buffer[0] = '\0';
  }

  return buffer;
}

void tesa_print_current_datetime(datetime_format_t format) {
  char buffer[32];
  const char *label;

  if (tesa_get_current_datetime(format, buffer, sizeof(buffer)) == NULL) {
    return;
  }

  if (format == DATETIME_FORMAT_TIME) {
    label = "Current time";
  } else {
    label = "Current date";
  }

  printf("%s: %s\r\n", label, buffer);
}

static void time_task(void *arg) {
  // datetime_format_t format =
  //     (arg != NULL) ? *(datetime_format_t *)arg : DATETIME_FORMAT_FULL;

  for (;;) {
    // tesa_print_current_datetime(format);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void tesa_print_datetime(datetime_format_t format) {
  static datetime_format_t dtf;
  dtf = format;
  xTaskCreate(time_task, "DateTime Task", configMINIMAL_STACK_SIZE * 1, &dtf,
              1U, NULL);
}

void tesa_set_rtc_time(int hour, int minute, int second, int day, int month,
                       int year) {
  struct tm time_set;
  cy_rslt_t result;

  if (rtc_obj_ptr == NULL) {
    return;
  }

  time_set.tm_sec = second;
  time_set.tm_min = minute;
  time_set.tm_hour = hour;
  time_set.tm_mday = day;
  time_set.tm_mon = month - 1;
  time_set.tm_year = year - 1900;
  time_set.tm_wday = 0;
  time_set.tm_yday = 0;
  time_set.tm_isdst = -1;

  result = mtb_hal_rtc_write(rtc_obj_ptr, &time_set);
  if (CY_RSLT_SUCCESS != result) {
    printf("Failed to set RTC time: 0x%08X\r\n", (unsigned int)result);
  } else {
    printf("RTC time set successfully\r\n");
  }
}

void tesa_set_rtc_to_compile_time(void) {
  struct tm time_set;
  cy_rslt_t result;
  const char *month_names[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                               "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  char month_str[4];
  int month, day, year;
  int hour, minute, second;

  if (rtc_obj_ptr == NULL) {
    return;
  }

  memset(&time_set, 0, sizeof(struct tm));

  sscanf(__DATE__, "%3s %d %d", month_str, &day, &year);
  sscanf(__TIME__, "%d:%d:%d", &hour, &minute, &second);

  for (month = 0; month < 12; month++) {
    if (strcmp(month_str, month_names[month]) == 0) {
      break;
    }
  }

  time_set.tm_sec = second;
  time_set.tm_min = minute;
  time_set.tm_hour = hour;
  time_set.tm_mday = day;
  time_set.tm_mon = month;
  time_set.tm_year = year - 1900;
  time_set.tm_wday = 0;
  time_set.tm_yday = 0;
  time_set.tm_isdst = -1;

  result = mtb_hal_rtc_write(rtc_obj_ptr, &time_set);
  if (CY_RSLT_SUCCESS != result) {
    printf("Failed to set RTC time\r\n");
  } else {
    printf("RTC set to compile time: %s %s\r\n", __DATE__, __TIME__);
  }
}
