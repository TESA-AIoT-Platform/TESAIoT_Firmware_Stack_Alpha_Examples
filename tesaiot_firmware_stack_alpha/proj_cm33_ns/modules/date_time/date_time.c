/*******************************************************************************
 * File Name        : date_time.c
 *
 * Description      : RTC-backed date/time helpers. Uses mtb_hal_rtc for set/sync;
 *                    time/localtime for reading. Init with system RTC; get/print/set
 *                    APIs then apply to it.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM33 (non-secure)
 *
 *******************************************************************************/

#include "date_time.h"
#include "cy_result.h"
#include "mtb_hal_rtc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static mtb_hal_rtc_t *s_rtc_ptr = NULL; /* RTC driver pointer set by date_time_init. */

/**
 * Stores the RTC driver pointer for later use. Call once (e.g. from main) before
 * any other date_time API. Pass the same mtb_hal_rtc_t* used for system RTC.
 */
void date_time_init(mtb_hal_rtc_t *rtc_obj)
{
  s_rtc_ptr = rtc_obj;
}

/**
 * Fills buffer with the current date/time from the RTC (via time/localtime). Format
 * selects full, date-only, or time-only. Returns buffer on success, NULL if buffer/size
 * invalid or localtime fails. Buffer is always null-terminated on return.
 */
char *date_time_get_current_datetime(date_time_format_t format, char *buffer, size_t buffer_size)
{
  time_t now;
  struct tm *timeinfo;

  if ((NULL == buffer) || (0U == buffer_size))
  {
    return NULL;
  }

  (void)time(&now);
  timeinfo = localtime(&now);
  if (NULL == timeinfo)
  {
    buffer[0U] = '\0';
    return NULL;
  }

  /* Format into buffer according to format; unknown format yields empty string. */
  if (DATE_TIME_FORMAT_FULL == format)
  {
    (void)snprintf(buffer, buffer_size, "%d-%02d-%02d %02d:%02d:%02d",
                   timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
                   timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  }
  else if (DATE_TIME_FORMAT_DATE == format)
  {
    (void)snprintf(buffer, buffer_size, "%d-%02d-%02d",
                   timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
  }
  else if (DATE_TIME_FORMAT_TIME == format)
  {
    (void)snprintf(buffer, buffer_size, "%02d:%02d:%02d",
                   timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  }
  else
  {
    buffer[0U] = '\0';
  }

  return buffer;
}

/**
 * Gets current date/time in the given format and prints it to stdout with a label
 * ("Current time" or "Current date"). No-op if date_time_get_current_datetime fails.
 */
void date_time_print_current_datetime(date_time_format_t format)
{
  char buffer[32];
  const char *label;

  if (NULL == date_time_get_current_datetime(format, buffer, sizeof(buffer)))
  {
    return;
  }

  /* Use "Current time" for time-only, "Current date" for full or date-only. */
  if (DATE_TIME_FORMAT_TIME == format)
  {
    label = "Current time";
  }
  else
  {
    label = "Current date";
  }

  (void)printf("%s: %s\r\n", label, buffer);
}

/**
 * Alias for date_time_print_current_datetime; prints current date/time in the given format.
 */
void date_time_print_datetime(date_time_format_t format)
{
  date_time_print_current_datetime(format);
}

/**
 * Sets the RTC to the given date and time (24h). No-op if init was not called or
 * mtb_hal_rtc_write fails; prints success or failure to stdout.
 */
void date_time_set_rtc_time(int hour, int minute, int second, int day, int month, int year)
{
  struct tm time_set;
  cy_rslt_t result;

  if (NULL == s_rtc_ptr)
  {
    return;
  }

  /* Build struct tm (month 0-based, year years since 1900) and write to RTC. */
  (void)memset(&time_set, 0, sizeof(time_set));
  time_set.tm_sec = second;
  time_set.tm_min = minute;
  time_set.tm_hour = hour;
  time_set.tm_mday = day;
  time_set.tm_mon = month - 1;
  time_set.tm_year = year - 1900;
  time_set.tm_wday = 0;
  time_set.tm_yday = 0;
  time_set.tm_isdst = -1;

  result = mtb_hal_rtc_write(s_rtc_ptr, &time_set);
  if (CY_RSLT_SUCCESS != result)
  {
    (void)printf("Failed to set RTC time: 0x%08X\r\n", (unsigned int)result);
  }
  else
  {
    (void)printf("RTC time set successfully\r\n");
  }
}

/**
 * Sets the RTC to the firmware build date and time (__DATE__, __TIME__). No-op if init
 * was not called or write fails; prints result to stdout.
 */
void date_time_set_rtc_to_compile_time(void)
{
  struct tm time_set;
  cy_rslt_t result;
  const char *month_names[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                               "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  char month_str[4];
  int month;
  int day;
  int year;
  int hour;
  int minute;
  int second;

  if (NULL == s_rtc_ptr)
  {
    return;
  }

  (void)memset(&time_set, 0, sizeof(time_set));

  /* Parse __DATE__ (e.g. "Feb 17 2026") and __TIME__ (e.g. "12:34:56"). */
  (void)sscanf(__DATE__, "%3s %d %d", month_str, &day, &year);
  (void)sscanf(__TIME__, "%d:%d:%d", &hour, &minute, &second);

  /* Resolve month name to 0-based index. */
  for (month = 0; month < 12; month++)
  {
    if (0 == strcmp(month_str, month_names[month]))
    {
      break;
    }
  }

  /* Fill struct tm and write to RTC. */
  time_set.tm_sec = second;
  time_set.tm_min = minute;
  time_set.tm_hour = hour;
  time_set.tm_mday = day;
  time_set.tm_mon = month;
  time_set.tm_year = year - 1900;
  time_set.tm_wday = 0;
  time_set.tm_yday = 0;
  time_set.tm_isdst = -1;

  result = mtb_hal_rtc_write(s_rtc_ptr, &time_set);
  if (CY_RSLT_SUCCESS != result)
  {
    (void)printf("Failed to set RTC time\r\n");
  }
  else
  {
    (void)printf("RTC set to compile time: %s %s\r\n", __DATE__, __TIME__);
  }
}
