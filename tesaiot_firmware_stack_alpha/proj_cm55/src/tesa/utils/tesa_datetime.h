#ifndef TESA_DATETIME_H
#define TESA_DATETIME_H

#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include "mtb_hal_rtc.h"

typedef enum {
  DATETIME_FORMAT_FULL = 0,
  DATETIME_FORMAT_DATE = 1,
  DATETIME_FORMAT_TIME = 2
} datetime_format_t;

void tesa_datetime_init(mtb_hal_rtc_t *rtc_obj);
char *tesa_get_current_datetime(datetime_format_t format, char *buffer, size_t buffer_size);
void tesa_print_current_datetime(datetime_format_t format);
void tesa_print_datetime(datetime_format_t format);
void tesa_set_rtc_time(int hour, int minute, int second, int day, int month, int year);
void tesa_set_rtc_to_compile_time(void);

#endif
