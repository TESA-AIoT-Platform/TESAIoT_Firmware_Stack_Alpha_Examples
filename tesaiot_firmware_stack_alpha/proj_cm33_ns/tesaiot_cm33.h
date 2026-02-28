#ifndef TESAIOT_CM33_H
#define TESAIOT_CM33_H

#include "error_handler.h"

#define TESAIOT_ERROR_CHECK_ENABLE
#ifdef TESAIOT_ERROR_CHECK_ENABLE
#define TESAIOT_ERROR_CHECK(expr) \
  if (!(expr))                    \
  {                               \
    handle_error(#expr);          \
  }
#else
#define TESAIOT_ERROR_CHECK(expr) (void)0
#endif

#endif /* TESAIOT_CM33_H */
