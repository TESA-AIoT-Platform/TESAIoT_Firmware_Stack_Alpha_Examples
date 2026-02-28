/*******************************************************************************
 * File Name        : date_time.h
 *
 * Description      : API and types for the date_time module (RTC get/print/set,
 *                    format enum for full, date-only, time-only).
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM33 (non-secure)
 *
 *******************************************************************************/

#ifndef DATE_TIME_H
#define DATE_TIME_H

#include <stddef.h>
#include <time.h>
#include "mtb_hal_rtc.h"

typedef enum
{
  DATE_TIME_FORMAT_FULL = 0,   /* YYYY-MM-DD HH:MM:SS */
  DATE_TIME_FORMAT_DATE = 1,   /* YYYY-MM-DD only */
  DATE_TIME_FORMAT_TIME = 2    /* HH:MM:SS only */
} date_time_format_t;

/**
 * Stores the RTC driver pointer for later use. Call once (e.g. from main) before any other
 * date_time API. Pass the same mtb_hal_rtc_t* used for system RTC.
 */
void date_time_init(mtb_hal_rtc_t *rtc_obj);

/**
 * Fills buffer with the current date/time from the RTC (via time/localtime). Format selects
 * full, date-only, or time-only. Returns buffer on success, NULL if buffer/size invalid or
 * localtime fails. Buffer is always null-terminated on return.
 */
char *date_time_get_current_datetime(date_time_format_t format, char *buffer, size_t buffer_size);

/**
 * Gets current date/time in the given format and prints it to stdout with a label (e.g. "Current date: ...").
 * No-op if date_time_get_current_datetime fails.
 */
void date_time_print_current_datetime(date_time_format_t format);

/**
 * Alias for date_time_print_current_datetime; prints current date/time in the given format.
 */
void date_time_print_datetime(date_time_format_t format);

/**
 * Sets the RTC to the given date and time (24h). No-op if init was not called or mtb_hal_rtc_write
 * fails; prints success or failure to stdout.
 */
void date_time_set_rtc_time(int hour, int minute, int second, int day, int month, int year);

/**
 * Sets the RTC to the firmware build date and time (__DATE__, __TIME__). No-op if init was not
 * called or write fails; prints result to stdout.
 */
void date_time_set_rtc_to_compile_time(void);

#endif
