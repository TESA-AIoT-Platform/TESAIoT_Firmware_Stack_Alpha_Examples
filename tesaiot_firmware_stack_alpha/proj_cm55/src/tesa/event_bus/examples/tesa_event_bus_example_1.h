#ifndef TESA_EVENT_BUS_EXAMPLE_1_H
#define TESA_EVENT_BUS_EXAMPLE_1_H

#include "../tesa_event_bus.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include <stdint.h>

#define EXAMPLE_1_CHANNEL_ID 0x0001U

#define EXAMPLE_1_EVENT_HELLO 0x0001U
#define EXAMPLE_1_EVENT_DATA 0x0002U

typedef struct {
  uint32_t counter;
  uint32_t value;
} example_1_data_t;

void tesa_event_bus_example_1(void);

#endif
