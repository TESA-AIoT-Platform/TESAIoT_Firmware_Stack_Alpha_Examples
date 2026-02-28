/*
Usage:

  #include "tesa/event_bus/examples.h"

  void main(void) {
      // Initialize FreeRTOS scheduler first
      // ...

      // Run example - sets up everything and returns
      tesa_event_bus_example_1();

      // Start scheduler (example tasks run independently)
      vTaskStartScheduler();
  }
*/

#include "tesa_event_bus_example_1.h"
#include "../../logging/tesa_logging.h"
#include <stdio.h>
#include <string.h>

#define EXAMPLE_1_QUEUE_LENGTH 10U
#define EXAMPLE_1_TASK_STACK_SIZE 1024U
#define EXAMPLE_1_TASK_PRIORITY 5U
#define EXAMPLE_1_PUBLISHER_DELAY_MS 2000U

static QueueHandle_t example_1_subscriber_queue = NULL;

static void example_1_publisher_task(void *pvParameters);
static void example_1_subscriber_task(void *pvParameters);

void tesa_event_bus_example_1(void) {
  tesa_event_bus_result_t result;
  BaseType_t task_result;

  result = tesa_event_bus_init();
  if (TESA_EVENT_BUS_SUCCESS != result) {
    return;
  }

  result = tesa_logging_init();
  if (TESA_EVENT_BUS_SUCCESS != result) {
    (void)printf("[EXAMPLE1] ERROR: Logging init failed: %d\r\n", result);
    (void)fflush(stdout);
    return;
  }

  (void)tesa_logging_set_level(TESA_LOG_DEBUG);

  TESA_LOG_INFO("Example1", "Event Bus Example 1: Logging system initialized");
  TESA_LOG_DEBUG("Example1", "Debug messages enabled");

  result = tesa_event_bus_register_channel(EXAMPLE_1_CHANNEL_ID, "Example1");
  if (TESA_EVENT_BUS_SUCCESS != result) {
    return;
  }

  example_1_subscriber_queue =
      xQueueCreate(EXAMPLE_1_QUEUE_LENGTH, sizeof(tesa_event_t *));
  if (NULL == example_1_subscriber_queue) {
    return;
  }

  result = tesa_event_bus_subscribe(EXAMPLE_1_CHANNEL_ID,
                                    example_1_subscriber_queue);
  if (TESA_EVENT_BUS_SUCCESS != result) {
    vQueueDelete(example_1_subscriber_queue);
    example_1_subscriber_queue = NULL;
    return;
  }

  task_result = xTaskCreate(example_1_publisher_task, "Example1Pub",
                            EXAMPLE_1_TASK_STACK_SIZE, NULL,
                            EXAMPLE_1_TASK_PRIORITY, NULL);
  if (pdPASS != task_result) {
    return;
  }

  task_result = xTaskCreate(
      example_1_subscriber_task, "Example1Sub", EXAMPLE_1_TASK_STACK_SIZE,
      example_1_subscriber_queue, EXAMPLE_1_TASK_PRIORITY, NULL);
  if (pdPASS != task_result) {
    return;
  }
}

static void example_1_publisher_task(void *pvParameters) {
  (void)pvParameters;
  uint32_t counter = 0U;
  tesa_event_bus_result_t result;
  example_1_data_t data;

  for (;;) {
    result = tesa_event_bus_post(EXAMPLE_1_CHANNEL_ID, EXAMPLE_1_EVENT_HELLO,
                                 NULL, 0U);
    if (TESA_EVENT_BUS_SUCCESS == result) {
      TESA_LOG_DEBUG("Publisher", "Posted HELLO event");
    }

    counter = counter + 1U;
    data.counter = counter;
    data.value = counter * 10U;

    result = tesa_event_bus_post(EXAMPLE_1_CHANNEL_ID, EXAMPLE_1_EVENT_DATA,
                                 &data, sizeof(example_1_data_t));
    if (TESA_EVENT_BUS_SUCCESS == result) {
      TESA_LOG_DEBUG("Publisher", "Posted DATA event: counter=%lu, value=%lu",
                     (unsigned long)data.counter, (unsigned long)data.value);
    }

    vTaskDelay(pdMS_TO_TICKS(EXAMPLE_1_PUBLISHER_DELAY_MS));
  }
}

static void example_1_subscriber_task(void *pvParameters) {
  QueueHandle_t queue = (QueueHandle_t)pvParameters;
  tesa_event_t *event = NULL;

  for (;;) {
    if (pdTRUE == xQueueReceive(queue, &event, pdMS_TO_TICKS(1000U))) {
      if (NULL != event) {
        switch (event->event_type) {
        case EXAMPLE_1_EVENT_HELLO:
          TESA_LOG_INFO("Subscriber", "Received HELLO event");
          break;

        case EXAMPLE_1_EVENT_DATA: {
          if ((NULL != event->payload) &&
              (sizeof(example_1_data_t) == event->payload_size)) {
            example_1_data_t *data = (example_1_data_t *)event->payload;
            TESA_LOG_INFO(
                "Subscriber", "Received DATA event: counter=%lu, value=%lu",
                (unsigned long)data->counter, (unsigned long)data->value);
          }
          break;
        }

        default:
          break;
        }

        tesa_event_bus_free_event(event);
        event = NULL;
      }
    }
  }
}