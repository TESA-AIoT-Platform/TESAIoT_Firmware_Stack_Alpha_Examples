# Event Bus Module – User Manual

**Author:** Asst. Prof. Santi Nuratch, Ph.D  
**Organization:** Thailand Embedded Systems Association (TESA)

---

## 1. Overview

The event bus module is a lightweight, multi-instance publisher/subscriber library for the PSoC Edge CM33 (non-secure) application. Each bus instance has a configurable maximum number of event IDs. Publishers emit events by ID; subscribers register callbacks per event ID and are invoked with optional payload data. Multiple independent buses can coexist (e.g. one for UI, one for sensors). All state is held in handles; there is no global state.

---

## 2. Features

- **Multi-instance** – Create separate buses for different domains (e.g. UI, sensors, buttons).
- **Thread-safe** – Each instance uses a FreeRTOS mutex around subscribe/unsubscribe/publish.
- **Dynamic configuration** – Maximum number of event IDs is set at create time; event IDs must be in `[0, max_event_ids - 1]`.
- **Subscribe / unsubscribe** – Callbacks can be added or removed per event ID.
- **Opaque handles** – No global state; all data is encapsulated in `event_bus_t`.

---

## 3. Dependencies

- **FreeRTOS** – Mutex and heap (`pvPortMalloc`) for bus context and subscriber nodes.
- **stdint / stdbool** – For `uint32_t` and `bool`.

---

## 4. Integration

### 4.1 Makefile

Paths below are relative to the CM33 project directory (the directory containing the Makefile).

- **INCLUDES** – Add the module directory so the compiler finds `event_bus.h`:
  ```makefile
  INCLUDES += modules/event_bus
  ```
- **SOURCES** – Many build systems (e.g. ModusToolbox) auto-discover `.c` files under the project tree. If yours does not, add the implementation explicitly:
  ```makefile
  SOURCES += modules/event_bus/event_bus.c
  ```

### 4.2 Initialization

Define an application-specific enum for event IDs and create a bus after early init (e.g. in `main` before starting tasks):

```c
#include "event_bus.h"

typedef enum {
  MY_EVENT_A,
  MY_EVENT_B,
  MY_EVENT_MAX
} my_events_t;

event_bus_t app_bus;

int main(void) {
  app_bus = event_bus_create(MY_EVENT_MAX);
  if (app_bus == NULL) {
    handle_error("event_bus_create failed");
  }
  event_bus_subscribe(app_bus, MY_EVENT_A, my_handler);
  vTaskStartScheduler();
}
```

---

## 5. API Reference

### 5.1 Lifecycle

| Function | Description |
|----------|-------------|
| `event_bus_create(max_event_ids)` | Allocates a new bus supporting event IDs in `[0, max_event_ids - 1]`. Returns handle or NULL. |
| `event_bus_destroy(bus)` | Frees the bus and all subscriber nodes. No effect if `bus` is NULL. |

### 5.2 Subscription

| Function | Description |
|----------|-------------|
| `event_bus_subscribe(bus, event_id, callback)` | Registers `callback` for `event_id` on the given bus. Returns true on success, false otherwise. |
| `event_bus_unsubscribe(bus, event_id, callback)` | Removes the given callback for `event_id`. Returns true on success, false otherwise. |

### 5.3 Publishing

| Function | Description |
|----------|-------------|
| `event_bus_publish(bus, event_id, event_data)` | Calls all callbacks registered for `event_id` with `(event_id, event_data)`. Returns true on success. |

---

## 6. Types

### 6.1 event_bus_t

Opaque handle to an event bus instance. Created by `event_bus_create()` and passed to subscribe, unsubscribe, publish, and destroy.

### 6.2 event_callback_t

Callback type used when subscribing:

```c
typedef void (*event_callback_t)(uint32_t event_id, void *event_data);
```

- **event_id** – The event ID that was published.
- **event_data** – The pointer passed to `event_bus_publish()` (may be NULL). Valid only for the duration of the callback.

---

## 7. Usage Examples

**Subscribe and handle one event:**

```c
void my_handler(uint32_t event_id, void *data) {
  if (event_id == MY_EVENT_A) {
    printf("Event A received!\n");
  }
}
event_bus_subscribe(app_bus, MY_EVENT_A, my_handler);
```

**Publish with no data:**

```c
event_bus_publish(app_bus, MY_EVENT_A, NULL);
```

**Publish with payload:**

```c
some_struct_t payload = { .value = 42 };
event_bus_publish(app_bus, MY_EVENT_B, &payload);
```

**Unsubscribe:**

```c
event_bus_unsubscribe(app_bus, MY_EVENT_A, my_handler);
```

**Tear down:**

```c
event_bus_destroy(app_bus);
```

---

## 8. Limits and Notes

- **Event ID range** – Event IDs must be in `[0, max_event_ids - 1]` for the given bus. Passing an out-of-range ID can lead to undefined behavior.
- **Memory** – Bus context and per-subscriber nodes are allocated from the FreeRTOS heap. Ensure sufficient heap and destroy buses when no longer needed.
- **Callback context** – Callbacks run in the context of the caller of `event_bus_publish()`. Keep them short; do not block or call publish recursively on the same bus if it could cause deadlock.
- **event_data lifetime** – The pointer passed to subscribers is valid only during the callback; copy data if needed after return.
