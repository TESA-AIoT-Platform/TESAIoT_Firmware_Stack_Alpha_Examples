/*******************************************************************************
 * File Name        : event_bus.c
 *
 * Description      : Multi-instance pub/sub event bus implementation.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM33 (non-secure)
 *
 *******************************************************************************/

#include "event_bus.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdlib.h>
#include <string.h>

/**
 * Subscriber node for the linked list.
 */
typedef struct subscriber_node
{
  event_callback_t callback;
  struct subscriber_node *next;
} subscriber_node_t;

/** Internal event bus context; holds per-event subscriber lists and mutex. */
struct event_bus_context
{
  subscriber_node_t **subscribers;  /* Per-event-ID head of linked list. */
  uint32_t max_events;              /* Maximum event ID (exclusive). */
  SemaphoreHandle_t mutex;          /* Protects subscribe/unsubscribe/publish. */
};

/**
 * Creates a new event bus instance.
 */
event_bus_t event_bus_create(uint32_t max_event_ids) {
  if (max_event_ids == 0) {
    return NULL;
  }

  struct event_bus_context *bus = pvPortMalloc(sizeof(struct event_bus_context));
  if (bus == NULL) {
    return NULL;
  }

  bus->subscribers = pvPortMalloc(sizeof(subscriber_node_t *) * max_event_ids);
  if (bus->subscribers == NULL) {
    vPortFree(bus);
    return NULL;
  }

  bus->mutex = xSemaphoreCreateMutex();
  if (bus->mutex == NULL) {
    vPortFree(bus->subscribers);
    vPortFree(bus);
    return NULL;
  }

  memset(bus->subscribers, 0, sizeof(subscriber_node_t *) * max_event_ids);
  bus->max_events = max_event_ids;

  return (event_bus_t)bus;
}

/**
 * Destroys an event bus instance.
 */
void event_bus_destroy(event_bus_t bus_handle) {
  struct event_bus_context *bus = (struct event_bus_context *)bus_handle;
  if (bus == NULL) {
    return;
  }

  /* Take mutex to ensure no one is using it while we destroy */
  if (xSemaphoreTake(bus->mutex, portMAX_DELAY) == pdTRUE) {
    for (uint32_t i = 0; i < bus->max_events; i++) {
      subscriber_node_t *current = bus->subscribers[i];
      while (current != NULL) {
        subscriber_node_t *next = current->next;
        vPortFree(current);
        current = next;
      }
    }
    vPortFree(bus->subscribers);
    xSemaphoreGive(bus->mutex);
  }

  vSemaphoreDelete(bus->mutex);
  vPortFree(bus);
}

/**
 * Subscribes a callback to a specific event ID.
 */
bool event_bus_subscribe(event_bus_t bus_handle, uint32_t event_id, event_callback_t callback) {
  struct event_bus_context *bus = (struct event_bus_context *)bus_handle;
  if (bus == NULL || event_id >= bus->max_events || callback == NULL) {
    return false;
  }

  if (xSemaphoreTake(bus->mutex, portMAX_DELAY) != pdTRUE)
  {
    return false;
  }

  /* Check if already subscribed to avoid duplicates. */
  subscriber_node_t *current = bus->subscribers[event_id];
  while (current != NULL) {
    if (current->callback == callback) {
      xSemaphoreGive(bus->mutex);
      return true;
    }
    current = current->next;
  }

  /* Create new subscriber node and prepend to list. */
  subscriber_node_t *new_node = pvPortMalloc(sizeof(subscriber_node_t));
  if (new_node == NULL) {
    xSemaphoreGive(bus->mutex);
    return false;
  }

  new_node->callback = callback;
  new_node->next = bus->subscribers[event_id];
  bus->subscribers[event_id] = new_node;

  xSemaphoreGive(bus->mutex);
  return true;
}

/**
 * Unsubscribes a callback from a specific event ID.
 */
bool event_bus_unsubscribe(event_bus_t bus_handle, uint32_t event_id, event_callback_t callback) {
  struct event_bus_context *bus = (struct event_bus_context *)bus_handle;
  if (bus == NULL || event_id >= bus->max_events || callback == NULL) {
    return false;
  }

  if (xSemaphoreTake(bus->mutex, portMAX_DELAY) != pdTRUE) {
    return false;
  }

  subscriber_node_t *current = bus->subscribers[event_id];
  subscriber_node_t *prev = NULL;

  while (current != NULL) {
    if (current->callback == callback) {
      if (prev == NULL) {
        bus->subscribers[event_id] = current->next;
      } else {
        prev->next = current->next;
      }
      vPortFree(current);
      xSemaphoreGive(bus->mutex);
      return true;
    }
    prev = current;
    current = current->next;
  }

  xSemaphoreGive(bus->mutex);
  return false;
}

/**
 * Publishes an event to all subscribers.
 */
bool event_bus_publish(event_bus_t bus_handle, uint32_t event_id, void *event_data) {
  struct event_bus_context *bus = (struct event_bus_context *)bus_handle;
  if (bus == NULL || event_id >= bus->max_events) {
    return false;
  }

  if (xSemaphoreTake(bus->mutex, portMAX_DELAY) != pdTRUE) {
    return false;
  }

  subscriber_node_t *current = bus->subscribers[event_id];
  while (current != NULL) {
    current->callback(event_id, event_data);
    current = current->next;
  }

  xSemaphoreGive(bus->mutex);
  return true;
}
