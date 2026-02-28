/*******************************************************************************
 * File Name        : event_bus.h
 *
 * Description      : Multi-instance publisher/subscriber event bus library.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM33 (non-secure)
 *
 *******************************************************************************/

#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Handle for an event bus instance.
 */
typedef struct event_bus_context *event_bus_t;

/**
 * Event callback function type. Invoked when an event is published.
 * event_id is the published ID; event_data is the payload (may be NULL).
 */
typedef void (*event_callback_t)(uint32_t event_id, void *event_data);

/**
 * Creates a new event bus instance supporting event IDs in [0, max_event_ids-1].
 * Returns handle or NULL on failure.
 */
event_bus_t event_bus_create(uint32_t max_event_ids);

/**
 * Destroys an event bus instance and frees all associated memory.
 * No effect if bus is NULL.
 */
void event_bus_destroy(event_bus_t bus);

/**
 * Subscribes a callback to a specific event ID. Idempotent if already subscribed.
 * Returns true on success, false on invalid args or mutex failure.
 */
bool event_bus_subscribe(event_bus_t bus, uint32_t event_id, event_callback_t callback);

/**
 * Unsubscribes a callback from a specific event ID.
 * Returns true if removed, false if not found or invalid args.
 */
bool event_bus_unsubscribe(event_bus_t bus, uint32_t event_id, event_callback_t callback);

/**
 * Publishes an event to all subscribers of the given event ID.
 * Callbacks run in caller context. Returns true on success.
 */
bool event_bus_publish(event_bus_t bus, uint32_t event_id, void *event_data);

#endif /* EVENT_BUS_H */
