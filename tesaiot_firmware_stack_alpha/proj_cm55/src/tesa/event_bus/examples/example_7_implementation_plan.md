# Example 7 Implementation Plan: Advanced Patterns

## Overview

Example 7 demonstrates advanced patterns including unsubscribe, channel unregistration, timestamp handling, and dynamic subscription management.

## Objectives

1. Demonstrate unsubscribe operations
2. Show channel unregistration
3. Demonstrate timestamp handling and rollover detection
4. Show dynamic subscription patterns
5. Demonstrate channel lifecycle management

## Implementation Details

### Channel Configuration

Create channels for dynamic management:

- **Channel 1**: Dynamic subscription test
- **Channel 2**: Timestamp handling test

### Header File Structure (`tesa_event_bus_example_7.h`)

```c
#define EXAMPLE_7_CHANNEL_DYNAMIC  0x0701U
#define EXAMPLE_7_CHANNEL_TIMESTAMP 0x0702U

#define EXAMPLE_7_EVENT_DATA  0x0701U
#define EXAMPLE_7_EVENT_TIMESTAMP_TEST 0x0702U

typedef struct {
    uint32_t sequence;
    uint32_t timestamp_ms;
} example_7_data_t;

void tesa_event_bus_example_7(void);
```

### Source File Structure (`tesa_event_bus_example_7.c`)

#### Constants

```c
#define EXAMPLE_7_QUEUE_LENGTH 10U
#define EXAMPLE_7_TASK_STACK_SIZE 1024U
#define EXAMPLE_7_TASK_PRIORITY 5U
#define EXAMPLE_7_SUBSCRIBE_INTERVAL_MS 5000U
#define EXAMPLE_7_UNSUBSCRIBE_INTERVAL_MS 10000U
```

#### Static Variables

```c
static QueueHandle_t example_7_queue_subscriber1 = NULL;
static QueueHandle_t example_7_queue_subscriber2 = NULL;
static bool example_7_subscriber1_active = false;
static bool example_7_subscriber2_active = false;
```

#### Main Function Flow

1. Initialize event bus
2. Register channels
3. Create subscriber queues
4. Create publisher task
5. Create subscriber tasks
6. Create dynamic management task (subscribe/unsubscribe)
7. Create timestamp test task

#### Dynamic Subscription Management

**Subscribe/Unsubscribe Task**:
```c
static void example_7_dynamic_management_task(void *pvParameters) {
    (void)pvParameters;
    tesa_event_bus_result_t result;
    uint32_t cycle_count = 0U;
    
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(EXAMPLE_7_SUBSCRIBE_INTERVAL_MS));
        cycle_count++;
        
        if (0U == (cycle_count % 2U)) {
            if (false == example_7_subscriber1_active) {
                result = tesa_event_bus_subscribe(
                    EXAMPLE_7_CHANNEL_DYNAMIC,
                    example_7_queue_subscriber1);
                
                if (TESA_EVENT_BUS_SUCCESS == result) {
                    example_7_subscriber1_active = true;
                    TESA_LOG_INFO("Subscriber 1 subscribed");
                }
            } else {
                result = tesa_event_bus_unsubscribe(
                    EXAMPLE_7_CHANNEL_DYNAMIC,
                    example_7_queue_subscriber1);
                
                if (TESA_EVENT_BUS_SUCCESS == result) {
                    example_7_subscriber1_active = false;
                    TESA_LOG_INFO("Subscriber 1 unsubscribed");
                }
            }
        }
        
        if (0U == (cycle_count % 3U)) {
            if (false == example_7_subscriber2_active) {
                result = tesa_event_bus_subscribe(
                    EXAMPLE_7_CHANNEL_DYNAMIC,
                    example_7_queue_subscriber2);
                
                if (TESA_EVENT_BUS_SUCCESS == result) {
                    example_7_subscriber2_active = true;
                    TESA_LOG_INFO("Subscriber 2 subscribed");
                }
            } else {
                result = tesa_event_bus_unsubscribe(
                    EXAMPLE_7_CHANNEL_DYNAMIC,
                    example_7_queue_subscriber2);
                
                if (TESA_EVENT_BUS_SUCCESS == result) {
                    example_7_subscriber2_active = false;
                    TESA_LOG_INFO("Subscriber 2 unsubscribed");
                }
            }
        }
    }
}
```

#### Channel Unregistration

**Unregister Test**:
```c
static void test_channel_unregistration(void) {
    tesa_event_bus_result_t result;
    QueueHandle_t queue;
    
    TESA_LOG_INFO("Testing channel unregistration...");
    
    result = tesa_event_bus_register_channel(0x0703U, "UnregisterTest");
    if (TESA_EVENT_BUS_SUCCESS != result) {
        TESA_LOG_ERROR("Failed to register test channel");
        return;
    }
    
    queue = xQueueCreate(5, sizeof(tesa_event_t *));
    result = tesa_event_bus_subscribe(0x0703U, queue);
    
    if (TESA_EVENT_BUS_SUCCESS == result) {
        result = tesa_event_bus_unregister_channel(0x0703U);
        if (TESA_EVENT_BUS_ERROR_HAS_SUBSCRIBERS == result) {
            TESA_LOG_INFO("✓ Correctly rejected unregister with active subscribers");
        }
        
        result = tesa_event_bus_unsubscribe(0x0703U, queue);
        if (TESA_EVENT_BUS_SUCCESS == result) {
            result = tesa_event_bus_unregister_channel(0x0703U);
            if (TESA_EVENT_BUS_SUCCESS == result) {
                TESA_LOG_INFO("✓ Successfully unregistered channel after unsubscribe");
            }
        }
        
        vQueueDelete(queue);
    }
}
```

#### Timestamp Handling

**Timestamp Test Task**:
```c
static void example_7_timestamp_task(void *pvParameters) {
    (void)pvParameters;
    example_7_data_t data;
    uint32_t current_time;
    uint32_t rollover_check;
    
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000U));
        
        current_time = tesa_event_bus_get_timestamp_ms();
        rollover_check = tesa_event_bus_check_timestamp_rollover(current_time);
        
        TESA_LOG_DEBUG("Current timestamp: %lu ms, Rollover: %lu",
                      (unsigned long)current_time,
                      (unsigned long)rollover_check);
        
        if (1U == rollover_check) {
            TESA_LOG_WARNING("Timestamp rollover detected!");
        }
        
        data.sequence = 0U;
        data.timestamp_ms = current_time;
        
        tesa_event_bus_post(EXAMPLE_7_CHANNEL_TIMESTAMP,
                           EXAMPLE_7_EVENT_TIMESTAMP_TEST,
                           &data,
                           sizeof(data));
    }
}
```

#### Event Age Calculation

**Subscriber with Age Check**:
```c
static void example_7_subscriber_with_age_check(QueueHandle_t queue) {
    tesa_event_t *event = NULL;
    uint32_t current_time;
    uint32_t event_age;
    
    for (;;) {
        if (pdTRUE == xQueueReceive(queue, &event, pdMS_TO_TICKS(1000U))) {
            if (NULL != event) {
                current_time = tesa_event_bus_get_timestamp_ms();
                
                if (current_time >= event->timestamp_ms) {
                    event_age = current_time - event->timestamp_ms;
                } else {
                    event_age = (TESA_EVENT_BUS_TIMESTAMP_ROLLOVER_MS - 
                                event->timestamp_ms) + current_time;
                }
                
                if (event_age > 1000U) {
                    TESA_LOG_WARNING("Stale event detected: %lu ms old",
                                   (unsigned long)event_age);
                } else {
                    TESA_LOG_DEBUG("Event age: %lu ms", (unsigned long)event_age);
                }
                
                tesa_event_bus_free_event(event);
                event = NULL;
            }
        }
    }
}
```

### Key Demonstration Points

1. **Unsubscribe**: Remove subscribers dynamically
2. **Unregister**: Remove channels when no subscribers
3. **Timestamp Retrieval**: Get current system timestamp
4. **Rollover Detection**: Detect 32-bit timestamp rollover (~49.7 days)
5. **Event Age Calculation**: Calculate how old an event is
6. **Dynamic Management**: Add/remove subscribers at runtime

### Expected Behavior

- Subscribers subscribe and unsubscribe dynamically
- Channel can be unregistered only when no subscribers
- Timestamps tracked and logged
- Rollover detected and logged
- Event age calculated correctly (handling rollover)

### Timestamp Rollover

The timestamp is a 32-bit value that rolls over after:
- 2^32 milliseconds = 4,294,967,296 ms
- ≈ 49.7 days

When calculating event age, must handle rollover case:
```c
if (current_time >= event->timestamp_ms) {
    age = current_time - event->timestamp_ms;
} else {
    age = (MAX_TIMESTAMP - event->timestamp_ms) + current_time;
}
```

### Use Cases

1. **Dynamic Reconfiguration**: Add/remove subscribers based on system state
2. **Resource Management**: Unregister channels when not needed
3. **Stale Event Detection**: Reject events that are too old
4. **Performance Monitoring**: Track event processing delays
5. **System Uptime**: Monitor system uptime via timestamps
