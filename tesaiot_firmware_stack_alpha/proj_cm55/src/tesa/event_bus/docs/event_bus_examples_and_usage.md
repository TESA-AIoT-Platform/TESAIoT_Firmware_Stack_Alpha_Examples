# TESA Event Bus Examples and Usage Guide

## Introduction

The TESA Event Bus is a multi-channel event communication system designed for medical device firmware that requires deterministic, thread-safe, and memory-safe inter-module communication. This guide provides comprehensive examples and usage patterns for integrating the event bus into your applications.

### Key Features

- **Multi-channel architecture**: Support for up to 16 independent event channels
- **Thread-safe**: Safe for both FreeRTOS task and ISR contexts
- **Memory-safe**: Static memory pools only (no dynamic allocation)
- **Configurable queue policies**: DROP_NEWEST, DROP_OLDEST, NO_DROP, WAIT
- **Statistics tracking**: Monitor event delivery success and failures
- **Medical device ready**: Designed for IEC 62304 compliance

### Related Documentation

- [Memory Ownership Documentation](tesa_event_bus_memory.md) - Memory management and cleanup requirements
- [Safety Analysis](event_bus_safety_analysi.md) - Safety concerns and compliance considerations
- [API Reference](../tesa_event_bus.h) - Complete API documentation

### Document Structure

This guide covers:
1. Basic setup and initialization
2. Simple usage patterns
3. Queue policy selection
4. Task and ISR context usage
5. Error handling
6. Application-specific integrations (WiFi, Bluetooth, Serial, MQTT, LVGL, Sensors)
7. Complete system integration
8. Best practices

---

## 1. Basic Setup

### 1.1 Initialization

The event bus must be initialized once before any other operations:

```c
#include "tesa_event_bus.h"

void system_init(void) {
    tesa_event_bus_result_t result = tesa_event_bus_init();
    if (TESA_EVENT_BUS_SUCCESS != result) {
        // Handle initialization error
        // Note: This should never fail unless called multiple times
    }
}
```

**Important**: Call `tesa_event_bus_init()` once during system startup, before any channels are registered.

### 1.2 Channel Registration

Channels must be registered before events can be posted or subscriptions made.

#### Default Configuration

```c
#define CHANNEL_SENSOR_DATA    0x1001
#define CHANNEL_ALARM          0x1002

void register_channels(void) {
    tesa_event_bus_result_t result;
    
    // Register with default configuration (DROP_NEWEST policy)
    result = tesa_event_bus_register_channel(CHANNEL_SENSOR_DATA, "SensorData");
    if (TESA_EVENT_BUS_SUCCESS != result) {
        // Handle error
    }
    
    result = tesa_event_bus_register_channel(CHANNEL_ALARM, "Alarm");
    if (TESA_EVENT_BUS_SUCCESS != result) {
        // Handle error
    }
}
```

#### Custom Configuration

For channels requiring specific queue policies:

```c
void register_critical_channel(void) {
    tesa_event_bus_channel_config_t config = {
        .queue_policy = TESA_EVENT_BUS_QUEUE_NO_DROP,  // Guaranteed delivery
        .queue_timeout_ticks = pdMS_TO_TICKS(1000)      // 1 second timeout
    };
    
    tesa_event_bus_result_t result = tesa_event_bus_register_channel_with_config(
        CHANNEL_ALARM,
        "Alarm",
        &config
    );
    
    if (TESA_EVENT_BUS_SUCCESS != result) {
        // Handle error
    }
}
```

### 1.3 Creating Subscriber Queues

Each subscriber needs a FreeRTOS queue to receive events:

```c
#define QUEUE_LENGTH 10

QueueHandle_t create_subscriber_queue(void) {
    // Create queue that holds pointers to tesa_event_t
    QueueHandle_t queue = xQueueCreate(QUEUE_LENGTH, sizeof(tesa_event_t *));
    
    if (queue == NULL) {
        // Handle queue creation failure
        return NULL;
    }
    
    return queue;
}
```

**Note**: Queue length should be sized based on expected event rate and processing speed.

### 1.4 Subscribing to Channels

Subscribers register their queue to receive events from a channel:

```c
void subscribe_to_channel(QueueHandle_t my_queue) {
    tesa_event_bus_result_t result = tesa_event_bus_subscribe(
        CHANNEL_SENSOR_DATA,
        my_queue
    );
    
    if (TESA_EVENT_BUS_SUCCESS != result) {
        // Handle subscription error
    }
}
```

---

## 2. Basic Examples

### 2.1 Simple Event Posting (No Payload)

Posting an event without payload data:

```c
#define EVENT_SYSTEM_READY 0x0001

void post_system_ready_event(void) {
    tesa_event_bus_result_t result = tesa_event_bus_post(
        CHANNEL_SENSOR_DATA,
        EVENT_SYSTEM_READY,
        NULL,        // No payload
        0            // Payload size is zero
    );
    
    if (TESA_EVENT_BUS_SUCCESS != result) {
        // Handle error based on result code
        switch (result) {
            case TESA_EVENT_BUS_ERROR_CHANNEL_NOT_FOUND:
                // Channel not registered
                break;
            case TESA_EVENT_BUS_ERROR_MEMORY:
                // Event pool exhausted
                break;
            case TESA_EVENT_BUS_ERROR_QUEUE_FULL:
                // All subscriber queues full
                break;
            case TESA_EVENT_BUS_ERROR_PARTIAL_SUCCESS:
                // Some subscribers received, others didn't
                break;
            default:
                break;
        }
    }
}
```

### 2.2 Event Posting with Payload

Posting an event with payload data:

```c
typedef struct {
    uint16_t sensor_id;
    float temperature;
    float humidity;
} sensor_data_t;

#define EVENT_SENSOR_READING 0x0002

void post_sensor_reading(uint16_t id, float temp, float hum) {
    sensor_data_t data = {
        .sensor_id = id,
        .temperature = temp,
        .humidity = hum
    };
    
    tesa_event_bus_result_t result = tesa_event_bus_post(
        CHANNEL_SENSOR_DATA,
        EVENT_SENSOR_READING,
        &data,
        sizeof(sensor_data_t)
    );
    
    if (TESA_EVENT_BUS_SUCCESS != result) {
        // Handle error
    }
}
```

**Important**: Payload size must not exceed `TESA_EVENT_BUS_MAX_PAYLOAD_SIZE` (256 bytes).

### 2.3 Receiving and Processing Events

Subscriber task that receives and processes events:

```c
void subscriber_task(void *pvParameters) {
    QueueHandle_t my_queue = (QueueHandle_t)pvParameters;
    
    for (;;) {
        tesa_event_t *event = NULL;
        
        // Wait for event with timeout
        if (pdTRUE == xQueueReceive(my_queue, &event, pdMS_TO_TICKS(1000))) {
            // Process event based on type
            switch (event->event_type) {
                case EVENT_SYSTEM_READY:
                    handle_system_ready();
                    break;
                    
                case EVENT_SENSOR_READING:
                    if (event->payload != NULL && 
                        event->payload_size == sizeof(sensor_data_t)) {
                        sensor_data_t *data = (sensor_data_t *)event->payload;
                        process_sensor_data(data);
                    }
                    break;
                    
                default:
                    // Unknown event type
                    break;
            }
            
            // CRITICAL: Always free event after processing
            tesa_event_bus_free_event(event);
            event = NULL;
        }
    }
}
```

### 2.4 Complete Basic Example

Putting it all together:

```c
void basic_example(void) {
    // 1. Initialize event bus
    tesa_event_bus_init();
    
    // 2. Register channel
    tesa_event_bus_register_channel(CHANNEL_SENSOR_DATA, "SensorData");
    
    // 3. Create subscriber queue
    QueueHandle_t queue = xQueueCreate(10, sizeof(tesa_event_t *));
    
    // 4. Subscribe to channel
    tesa_event_bus_subscribe(CHANNEL_SENSOR_DATA, queue);
    
    // 5. Create subscriber task
    xTaskCreate(subscriber_task, "Subscriber", 1024, queue, 5, NULL);
    
    // 6. Post events
    post_system_ready_event();
    post_sensor_reading(1, 25.5f, 60.0f);
}
```

---

## 3. Queue Policy Examples

Different queue policies suit different use cases. Select based on your requirements:

### 3.1 DROP_NEWEST - Fire and Forget

**Use case**: High-frequency, non-critical events where latest data is preferred.

```c
void register_drop_newest_channel(void) {
    tesa_event_bus_channel_config_t config = {
        .queue_policy = TESA_EVENT_BUS_QUEUE_DROP_NEWEST,
        .queue_timeout_ticks = 0  // Not used for DROP_NEWEST
    };
    
    tesa_event_bus_register_channel_with_config(
        CHANNEL_SENSOR_DATA,
        "SensorData",
        &config
    );
}

// Posting events - if queue is full, this event is dropped
void post_sensor_data(void) {
    sensor_data_t data = { /* ... */ };
    tesa_event_bus_post(CHANNEL_SENSOR_DATA, EVENT_SENSOR_READING, 
                       &data, sizeof(data));
    // Always returns SUCCESS or error (never blocks)
}
```

**Characteristics**:
- Non-blocking
- Drops new events when queue is full
- Suitable for high-frequency sensor data
- Oldest data in queue is preserved

### 3.2 DROP_OLDEST - Latest Data Only

**Use case**: Always keep the most recent data, discard older samples.

```c
void register_drop_oldest_channel(void) {
    tesa_event_bus_channel_config_t config = {
        .queue_policy = TESA_EVENT_BUS_QUEUE_DROP_OLDEST,
        .queue_timeout_ticks = 0  // Not used for DROP_OLDEST
    };
    
    tesa_event_bus_register_channel_with_config(
        CHANNEL_SENSOR_DATA,
        "SensorData",
        &config
    );
}

// Posting - if queue is full, overwrites oldest event
void post_sensor_data(void) {
    sensor_data_t data = { /* ... */ };
    tesa_event_bus_post(CHANNEL_SENSOR_DATA, EVENT_SENSOR_READING,
                       &data, sizeof(data));
    // Always succeeds, overwrites oldest if queue full
}
```

**Characteristics**:
- Non-blocking
- Overwrites oldest events when queue is full
- Guarantees latest data is always available
- Suitable for status updates where old data is irrelevant

### 3.3 NO_DROP - Guaranteed Delivery

**Use case**: Critical events that must be delivered (alarms, safety events).

```c
void register_no_drop_channel(void) {
    tesa_event_bus_channel_config_t config = {
        .queue_policy = TESA_EVENT_BUS_QUEUE_NO_DROP,
        .queue_timeout_ticks = pdMS_TO_TICKS(5000)  // Wait up to 5 seconds
    };
    
    tesa_event_bus_register_channel_with_config(
        CHANNEL_ALARM,
        "Alarm",
        &config
    );
}

void post_critical_alarm(void) {
    alarm_data_t alarm = { /* ... */ };
    tesa_event_bus_result_t result = tesa_event_bus_post(
        CHANNEL_ALARM,
        EVENT_CRITICAL_ALARM,
        &alarm,
        sizeof(alarm)
    );
    
    if (TESA_EVENT_BUS_ERROR_QUEUE_FULL == result) {
        // Queue still full after timeout - critical failure!
        handle_critical_delivery_failure();
    }
}
```

**Characteristics**:
- Blocks until space available or timeout
- Guarantees delivery (if not timeout)
- Use for safety-critical events
- Monitor timeout to detect stuck subscribers

### 3.4 WAIT - Blocking with Timeout

**Use case**: Important events that should wait, but not indefinitely.

```c
void register_wait_channel(void) {
    tesa_event_bus_channel_config_t config = {
        .queue_policy = TESA_EVENT_BUS_QUEUE_WAIT,
        .queue_timeout_ticks = pdMS_TO_TICKS(1000)  // 1 second timeout
    };
    
    tesa_event_bus_register_channel_with_config(
        CHANNEL_COMMAND,
        "Command",
        &config
    );
}

void post_command(void) {
    command_t cmd = { /* ... */ };
    tesa_event_bus_result_t result = tesa_event_bus_post(
        CHANNEL_COMMAND,
        EVENT_COMMAND,
        &cmd,
        sizeof(cmd)
    );
    
    if (TESA_EVENT_BUS_ERROR_QUEUE_FULL == result) {
        // Subscriber not processing fast enough
        handle_subscriber_slow();
    }
}
```

**Characteristics**:
- Blocks with timeout
- Same behavior as NO_DROP (both use timeout)
- Allows graceful degradation on timeout
- Monitor for subscriber health

---

## 4. Context-Specific Examples

### 4.1 Task Context Posting

Posting from FreeRTOS task context:

```c
void sensor_processing_task(void *pvParameters) {
    for (;;) {
        // Read sensor
        sensor_data_t data = read_sensor();
        
        // Post event from task context
        tesa_event_bus_result_t result = tesa_event_bus_post(
            CHANNEL_SENSOR_DATA,
            EVENT_SENSOR_READING,
            &data,
            sizeof(data)
        );
        
        if (TESA_EVENT_BUS_SUCCESS != result) {
            // Handle error - but continue processing
            handle_post_error(result);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

### 4.2 ISR Context Posting

Posting from interrupt service routine:

```c
void GPIO_IRQ_Handler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t result;
    
    // Prepare event data
    button_data_t button_data = {
        .button_id = get_irq_button_id(),
        .pressed = true,
        .timestamp = tesa_event_bus_get_timestamp_ms()
    };
    
    // Post from ISR context
    result = tesa_event_bus_post_from_isr(
        CHANNEL_BUTTON,
        EVENT_BUTTON_PRESS,
        &button_data,
        sizeof(button_data),
        &xHigherPriorityTaskWoken
    );
    
    if (pdTRUE == result) {
        // Event posted successfully
        if (pdTRUE == xHigherPriorityTaskWoken) {
            // Higher priority task was woken - yield required
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    } else {
        // Event posting failed - handle in task context if needed
        // Note: Can't do heavy processing in ISR
    }
    
    // Clear interrupt flag
    clear_gpio_interrupt();
}
```

**Critical Rules for ISR Context**:
- Always use `tesa_event_bus_post_from_isr()`, never `tesa_event_bus_post()`
- Check `pxHigherPriorityTaskWoken` and yield if needed
- Keep ISR processing minimal
- Handle errors appropriately (may need deferred error handling)

### 4.3 Timer ISR Example

Posting from periodic timer interrupt:

```c
void timer_callback(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    // Read sensor in ISR (fast sensor with register interface)
    gyro_data_t gyro = read_gyroscope_registers();
    
    // Post sensor data event
    tesa_event_bus_post_from_isr(
        CHANNEL_GYRO,
        EVENT_GYRO_DATA_READY,
        &gyro,
        sizeof(gyro),
        &xHigherPriorityTaskWoken
    );
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
```

---

## 5. Multi-Subscriber Patterns

### 5.1 Broadcasting to Multiple Subscribers

One event posted to a channel reaches all subscribers:

```c
void broadcast_system_status(void) {
    system_status_t status = {
        .battery_level = get_battery_level(),
        .temperature = get_system_temp(),
        .uptime = get_uptime_seconds()
    };
    
    // Post once - all subscribers receive it
    tesa_event_bus_post(
        CHANNEL_SYSTEM_STATUS,
        EVENT_STATUS_UPDATE,
        &status,
        sizeof(status)
    );
    
    // Multiple subscribers each get their own copy of the event
    // - Display task updates GUI
    // - Logger task writes to flash
    // - Network task sends to server
    // - Health monitor checks thresholds
}

// Each subscriber has its own queue and receives independent event copy
void display_subscriber_task(void *pvParameters) {
    QueueHandle_t display_queue = (QueueHandle_t)pvParameters;
    
    for (;;) {
        tesa_event_t *event = NULL;
        if (xQueueReceive(display_queue, &event, portMAX_DELAY) == pdTRUE) {
            update_display(event);
            tesa_event_bus_free_event(event);
        }
    }
}
```

### 5.2 Channel Statistics per Subscriber

Monitor individual subscriber performance:

```c
void monitor_subscriber_health(QueueHandle_t subscriber_queue) {
    tesa_event_bus_subscriber_stats_t stats;
    
    tesa_event_bus_result_t result = tesa_event_bus_get_subscriber_stats(
        CHANNEL_SENSOR_DATA,
        subscriber_queue,
        &stats
    );
    
    if (TESA_EVENT_BUS_SUCCESS == result) {
        // Check for high drop rate
        uint32_t total = stats.posts_successful + stats.posts_dropped;
        if (total > 0) {
            float drop_rate = (float)stats.posts_dropped / total;
            
            if (drop_rate > 0.1f) {  // More than 10% dropped
                // Subscriber is not processing fast enough
                handle_subscriber_slowdown(subscriber_queue);
            }
        }
        
        // Check if drops are recent
        uint32_t current_time = tesa_event_bus_get_timestamp_ms();
        uint32_t time_since_drop = current_time - stats.last_drop_timestamp_ms;
        
        if (time_since_drop < 1000) {  // Drops within last second
            // Recent drops detected
            log_recent_drops(stats);
        }
    }
}
```

### 5.3 Aggregate Channel Statistics

Get overall channel performance:

```c
void monitor_channel_health(void) {
    tesa_event_bus_subscriber_stats_t total_stats;
    uint8_t subscriber_count;
    
    tesa_event_bus_result_t result = tesa_event_bus_get_channel_stats(
        CHANNEL_SENSOR_DATA,
        &total_stats,
        &subscriber_count
    );
    
    if (TESA_EVENT_BUS_SUCCESS == result) {
        printf("Channel has %d subscribers\n", subscriber_count);
        printf("Total successful: %lu\n", total_stats.posts_successful);
        printf("Total dropped: %lu\n", total_stats.posts_dropped);
        
        // Calculate overall drop rate
        uint32_t total = total_stats.posts_successful + total_stats.posts_dropped;
        if (total > 0) {
            float drop_rate = (float)total_stats.posts_dropped / total;
            printf("Drop rate: %.2f%%\n", drop_rate * 100.0f);
        }
    }
}
```

---

## 6. Error Handling

### 6.1 Checking Return Codes

Always check return codes and handle appropriately:

```c
void robust_event_posting(void) {
    sensor_data_t data = read_sensor();
    
    tesa_event_bus_result_t result = tesa_event_bus_post(
        CHANNEL_SENSOR_DATA,
        EVENT_SENSOR_READING,
        &data,
        sizeof(data)
    );
    
    switch (result) {
        case TESA_EVENT_BUS_SUCCESS:
            // Event posted successfully to all subscribers
            break;
            
        case TESA_EVENT_BUS_ERROR_CHANNEL_NOT_FOUND:
            // Channel not registered - configuration error
            log_error("Channel not registered");
            break;
            
        case TESA_EVENT_BUS_ERROR_MEMORY:
            // Event pool exhausted - system under stress
            log_warning("Event pool exhausted");
            handle_memory_pressure();
            break;
            
        case TESA_EVENT_BUS_ERROR_PAYLOAD_TOO_LARGE:
            // Payload exceeds maximum size
            log_error("Payload too large: %zu bytes", sizeof(data));
            break;
            
        case TESA_EVENT_BUS_ERROR_QUEUE_FULL:
            // All subscriber queues full (NO_DROP/WAIT policy timeout)
            log_critical("All subscriber queues full");
            handle_critical_delivery_failure();
            break;
            
        case TESA_EVENT_BUS_ERROR_PARTIAL_SUCCESS:
            // Some subscribers received, others didn't
            log_warning("Partial event delivery");
            handle_partial_delivery();
            break;
            
        default:
            log_error("Unknown error: %d", result);
            break;
    }
}
```

### 6.2 Handling Partial Success

When some subscribers receive the event but others don't:

```c
void handle_partial_success(tesa_event_bus_result_t result) {
    if (TESA_EVENT_BUS_ERROR_PARTIAL_SUCCESS == result) {
        // Check which subscribers are having issues
        tesa_event_bus_subscriber_stats_t total_stats;
        uint8_t subscriber_count;
        
        tesa_event_bus_get_channel_stats(
            CHANNEL_SENSOR_DATA,
            &total_stats,
            &subscriber_count
        );
        
        // Identify problematic subscribers
        for (uint8_t i = 0; i < subscriber_count; i++) {
            // In real implementation, need to track queue handles
            // and check individual stats
        }
        
        // May need to:
        // - Alert watchdog
        // - Log for analysis
        // - Trigger recovery actions
    }
}
```

### 6.3 Memory Exhaustion Scenarios

Handling event pool exhaustion:

```c
void handle_memory_pressure(void) {
    // Check if this is a persistent issue
    static uint32_t exhaustion_count = 0;
    static uint32_t last_exhaustion_time = 0;
    
    uint32_t current_time = tesa_event_bus_get_timestamp_ms();
    
    if (current_time - last_exhaustion_time < 1000) {
        exhaustion_count++;
    } else {
        exhaustion_count = 1;
    }
    
    last_exhaustion_time = current_time;
    
    if (exhaustion_count > 10) {
        // Persistent memory exhaustion
        // Possible causes:
        // 1. Subscribers not freeing events
        // 2. Event rate too high
        // 3. Subscriber queues too small
        
        trigger_emergency_recovery();
    }
}
```

### 6.4 Queue Full Scenarios

Handling when subscriber queues are full:

```c
void handle_queue_full(void) {
    // For critical channels, queue full is a serious issue
    if (is_critical_channel(CHANNEL_ALARM)) {
        // Critical events must be delivered
        // Possible actions:
        // 1. Alert watchdog
        // 2. Force subscriber task to higher priority
        // 3. Trigger emergency shutdown
        // 4. Log for post-mortem analysis
        
        trigger_emergency_shutdown();
    } else {
        // Non-critical: log and continue
        log_warning("Non-critical channel queue full");
        // May increase subscriber task priority temporarily
    }
}
```

---

## 7. Advanced Patterns

### 7.1 Event Type Enums for Structured Events

Organize event types by module:

```c
// Sensor events
typedef enum {
    EVENT_SENSOR_DATA_READY = 0x1000,
    EVENT_SENSOR_CALIBRATION_COMPLETE,
    EVENT_SENSOR_ERROR,
    EVENT_SENSOR_CONFIG_CHANGED
} sensor_event_type_t;

// Alarm events
typedef enum {
    EVENT_ALARM_TRIGGERED = 0x2000,
    EVENT_ALARM_CLEARED,
    EVENT_ALARM_ACKNOWLEDGED,
    EVENT_ALARM_TEST
} alarm_event_type_t;

// System events
typedef enum {
    EVENT_SYSTEM_READY = 0x3000,
    EVENT_SYSTEM_SHUTDOWN,
    EVENT_SYSTEM_ERROR,
    EVENT_SYSTEM_RESET
} system_event_type_t;

// Usage
void post_alarm(void) {
    alarm_data_t alarm = { /* ... */ };
    tesa_event_bus_post(
        CHANNEL_ALARM,
        EVENT_ALARM_TRIGGERED,  // Type-safe enum value
        &alarm,
        sizeof(alarm)
    );
}
```

### 7.2 Payload Structures

Define type-safe payload structures:

```c
// Sensor data payload
typedef struct {
    uint16_t sensor_id;
    float temperature;
    float humidity;
    uint32_t timestamp;
} sensor_data_t;

// Alarm payload
typedef struct {
    uint8_t alarm_id;
    uint8_t severity;
    char message[32];
    uint32_t timestamp;
} alarm_data_t;

// Command payload
typedef struct {
    uint16_t command_id;
    uint8_t parameters[64];
    uint8_t param_length;
} command_t;

// Usage with type checking
void process_event(tesa_event_t *event) {
    switch (event->event_type) {
        case EVENT_SENSOR_READING:
            if (event->payload_size == sizeof(sensor_data_t)) {
                sensor_data_t *data = (sensor_data_t *)event->payload;
                // Type-safe access
                process_sensor_data(data);
            }
            break;
            
        case EVENT_ALARM_TRIGGERED:
            if (event->payload_size == sizeof(alarm_data_t)) {
                alarm_data_t *alarm = (alarm_data_t *)event->payload;
                handle_alarm(alarm);
            }
            break;
    }
}
```

### 7.3 Timestamp Handling and Rollover Detection

Handle timestamp rollover (occurs after ~49.7 days):

```c
void process_event_with_timestamp(tesa_event_t *event) {
    uint32_t event_time = event->timestamp_ms;
    uint32_t current_time = tesa_event_bus_get_timestamp_ms();
    
    // Check for timestamp rollover
    if (tesa_event_bus_check_timestamp_rollover(current_time)) {
        // System has been running >49 days, timestamps may have rolled over
        // Need special handling for time calculations
        handle_timestamp_rollover(event_time, current_time);
    }
    
    // Calculate age of event
    uint32_t age_ms;
    if (current_time >= event_time) {
        // Normal case: no rollover
        age_ms = current_time - event_time;
    } else {
        // Rollover occurred between event and now
        age_ms = (TESA_EVENT_BUS_TIMESTAMP_ROLLOVER_MS - event_time) + current_time;
    }
    
    // Reject stale events (>1 second old)
    if (age_ms > 1000) {
        log_warning("Stale event detected: %lu ms old", age_ms);
        return;
    }
    
    process_fresh_event(event);
}
```

### 7.4 Statistics Monitoring for Health Checks

Periodic health monitoring task:

```c
void health_monitor_task(void *pvParameters) {
    const TickType_t check_interval = pdMS_TO_TICKS(5000);  // Every 5 seconds
    
    for (;;) {
        // Check all critical channels
        check_channel_health(CHANNEL_ALARM);
        check_channel_health(CHANNEL_SENSOR_DATA);
        check_channel_health(CHANNEL_COMMAND);
        
        vTaskDelay(check_interval);
    }
}

void check_channel_health(tesa_event_channel_id_t channel_id) {
    tesa_event_bus_subscriber_stats_t total_stats;
    uint8_t subscriber_count;
    
    tesa_event_bus_result_t result = tesa_event_bus_get_channel_stats(
        channel_id,
        &total_stats,
        &subscriber_count
    );
    
    if (TESA_EVENT_BUS_SUCCESS != result) {
        return;
    }
    
    // Calculate metrics
    uint32_t total_events = total_stats.posts_successful + total_stats.posts_dropped;
    
    if (total_events == 0) {
        // No events posted yet - normal for idle channels
        return;
    }
    
    float drop_rate = (float)total_stats.posts_dropped / total_events;
    
    // Alert on high drop rate
    if (drop_rate > 0.05f) {  // More than 5% dropped
        log_warning("High drop rate on channel 0x%04X: %.2f%%", 
                   channel_id, drop_rate * 100.0f);
        trigger_health_alert(channel_id, drop_rate);
    }
    
    // Alert on recent drops
    uint32_t current_time = tesa_event_bus_get_timestamp_ms();
    uint32_t time_since_drop = current_time - total_stats.last_drop_timestamp_ms;
    
    if (time_since_drop < 1000) {  // Drop within last second
        log_warning("Recent drops on channel 0x%04X", channel_id);
    }
}
```

---

## 8. Application-Specific Examples

This section provides complete examples for integrating the event bus with specific application modules.

### 8.1 WiFi Integration

WiFi modules generate various events that can be distributed through the event bus.

#### Channel and Event Definitions

```c
#define CHANNEL_WIFI         0x2001

// WiFi event types
typedef enum {
    EVENT_WIFI_CONNECTED = 0x2001,
    EVENT_WIFI_DISCONNECTED,
    EVENT_WIFI_DATA_RECEIVED,
    EVENT_WIFI_DATA_SENT,
    EVENT_WIFI_SCAN_RESULT,
    EVENT_WIFI_ERROR,
    EVENT_WIFI_IP_ASSIGNED
} wifi_event_type_t;

// WiFi connection status payload
typedef struct {
    uint8_t ssid[32];
    int8_t rssi;
    uint8_t channel;
    uint32_t ip_address;
} wifi_connected_data_t;

// WiFi data received payload
typedef struct {
    uint8_t *data;
    uint16_t length;
    uint16_t port;
    uint32_t remote_ip;
} wifi_data_rx_t;

// WiFi scan result payload
typedef struct {
    uint8_t ssid[32];
    int8_t rssi;
    uint8_t channel;
    uint8_t security;
} wifi_scan_result_t;
```

#### Channel Registration

```c
void wifi_init_channels(void) {
    // WiFi status channel - critical, use NO_DROP
    tesa_event_bus_channel_config_t status_config = {
        .queue_policy = TESA_EVENT_BUS_QUEUE_NO_DROP,
        .queue_timeout_ticks = pdMS_TO_TICKS(1000)
    };
    tesa_event_bus_register_channel_with_config(
        CHANNEL_WIFI,
        "WiFi",
        &status_config
    );
    
    // Data channel - high frequency, drop oldest
    tesa_event_bus_channel_config_t data_config = {
        .queue_policy = TESA_EVENT_BUS_QUEUE_DROP_OLDEST,
        .queue_timeout_ticks = 0
    };
    tesa_event_bus_register_channel_with_config(
        CHANNEL_WIFI_DATA,
        "WiFiData",
        &data_config
    );
}
```

#### Posting WiFi Connection Events

```c
void wifi_on_connected(const char *ssid, int8_t rssi, uint8_t channel, uint32_t ip) {
    wifi_connected_data_t data = {
        .rssi = rssi,
        .channel = channel,
        .ip_address = ip
    };
    strncpy((char *)data.ssid, ssid, sizeof(data.ssid) - 1);
    
    tesa_event_bus_post(
        CHANNEL_WIFI,
        EVENT_WIFI_CONNECTED,
        &data,
        sizeof(data)
    );
}

void wifi_on_disconnected(void) {
    tesa_event_bus_post(
        CHANNEL_WIFI,
        EVENT_WIFI_DISCONNECTED,
        NULL,
        0
    );
}
```

#### Posting WiFi Data Events from ISR

```c
void wifi_rx_isr_handler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    // Read data from WiFi module (assuming direct buffer access)
    static uint8_t rx_buffer[256];
    uint16_t length = wifi_read_rx_buffer(rx_buffer, sizeof(rx_buffer));
    
    if (length > 0 && length <= TESA_EVENT_BUS_MAX_PAYLOAD_SIZE) {
        wifi_data_rx_t rx_data;
        rx_data.length = length;
        rx_data.port = wifi_get_rx_port();
        rx_data.remote_ip = wifi_get_remote_ip();
        
        // Copy data (must fit in event payload)
        if (length <= 200) {  // Reserve space for metadata
            memcpy(rx_buffer, rx_data.data, length);
            rx_data.data = rx_buffer;  // Note: pointer in payload - see notes
            
            tesa_event_bus_post_from_isr(
                CHANNEL_WIFI_DATA,
                EVENT_WIFI_DATA_RECEIVED,
                &rx_data,
                sizeof(rx_data),
                &xHigherPriorityTaskWoken
            );
        }
    }
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
```

**Note**: For data payloads, copy the actual data into the payload structure rather than storing pointers, since pointers become invalid.

#### WiFi Event Subscriber Task

```c
void wifi_event_handler_task(void *pvParameters) {
    QueueHandle_t wifi_queue = (QueueHandle_t)pvParameters;
    
    for (;;) {
        tesa_event_t *event = NULL;
        
        if (xQueueReceive(wifi_queue, &event, portMAX_DELAY) == pdTRUE) {
            switch (event->event_type) {
                case EVENT_WIFI_CONNECTED: {
                    wifi_connected_data_t *data = (wifi_connected_data_t *)event->payload;
                    printf("WiFi connected: %s, IP: %lu.%lu.%lu.%lu\n",
                           data->ssid,
                           (data->ip_address >> 24) & 0xFF,
                           (data->ip_address >> 16) & 0xFF,
                           (data->ip_address >> 8) & 0xFF,
                           data->ip_address & 0xFF);
                    update_wifi_status_led(true);
                    break;
                }
                
                case EVENT_WIFI_DISCONNECTED:
                    printf("WiFi disconnected\n");
                    update_wifi_status_led(false);
                    trigger_reconnection();
                    break;
                
                case EVENT_WIFI_DATA_RECEIVED: {
                    wifi_data_rx_t *rx = (wifi_data_rx_t *)event->payload;
                    process_received_data(rx->data, rx->length, rx->port);
                    break;
                }
                
                case EVENT_WIFI_ERROR:
                    handle_wifi_error();
                    break;
            }
            
            tesa_event_bus_free_event(event);
        }
    }
}
```

### 8.2 Bluetooth Integration

Bluetooth/BLE modules generate discovery, connection, and data transfer events.

#### Channel and Event Definitions

```c
#define CHANNEL_BLUETOOTH    0x3001

typedef enum {
    EVENT_BT_DEVICE_DISCOVERED = 0x3001,
    EVENT_BT_CONNECTED,
    EVENT_BT_DISCONNECTED,
    EVENT_BT_DATA_RECEIVED,
    EVENT_BT_DATA_SENT,
    EVENT_BT_CHARACTERISTIC_NOTIFY,
    EVENT_BT_SCAN_COMPLETE,
    EVENT_BT_ERROR
} bluetooth_event_type_t;

typedef struct {
    uint8_t address[6];  // MAC address
    int8_t rssi;
    char name[32];
    uint16_t uuid;
} bt_device_info_t;

typedef struct {
    uint8_t address[6];
    uint16_t connection_handle;
    uint8_t bond_status;
} bt_connection_t;

typedef struct {
    uint16_t characteristic_uuid;
    uint8_t data[200];  // BLE characteristic value
    uint16_t length;
} bt_characteristic_data_t;
```

#### Posting Bluetooth Events

```c
void bt_on_device_discovered(const uint8_t *addr, int8_t rssi, const char *name) {
    bt_device_info_t device = {
        .rssi = rssi
    };
    memcpy(device.address, addr, 6);
    strncpy(device.name, name, sizeof(device.name) - 1);
    
    tesa_event_bus_post(
        CHANNEL_BLUETOOTH,
        EVENT_BT_DEVICE_DISCOVERED,
        &device,
        sizeof(device)
    );
}

void bt_on_connected(const uint8_t *addr, uint16_t handle) {
    bt_connection_t conn = {
        .connection_handle = handle,
        .bond_status = 0
    };
    memcpy(conn.address, addr, 6);
    
    tesa_event_bus_post(
        CHANNEL_BLUETOOTH,
        EVENT_BT_CONNECTED,
        &conn,
        sizeof(conn)
    );
}

// Post from BLE notification ISR
void ble_notification_isr(uint16_t uuid, const uint8_t *data, uint16_t length) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    bt_characteristic_data_t char_data;
    char_data.characteristic_uuid = uuid;
    char_data.length = (length > 200) ? 200 : length;
    memcpy(char_data.data, data, char_data.length);
    
    tesa_event_bus_post_from_isr(
        CHANNEL_BLUETOOTH,
        EVENT_BT_CHARACTERISTIC_NOTIFY,
        &char_data,
        sizeof(char_data),
        &xHigherPriorityTaskWoken
    );
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
```

#### Bluetooth Event Handler

```c
void bluetooth_handler_task(void *pvParameters) {
    QueueHandle_t bt_queue = (QueueHandle_t)pvParameters;
    
    for (;;) {
        tesa_event_t *event = NULL;
        
        if (xQueueReceive(bt_queue, &event, portMAX_DELAY) == pdTRUE) {
            switch (event->event_type) {
                case EVENT_BT_DEVICE_DISCOVERED: {
                    bt_device_info_t *device = (bt_device_info_t *)event->payload;
                    printf("Device found: %s, RSSI: %d\n", device->name, device->rssi);
                    add_to_discovery_list(device);
                    break;
                }
                
                case EVENT_BT_CONNECTED: {
                    bt_connection_t *conn = (bt_connection_t *)event->payload;
                    printf("Connected to device\n");
                    update_bt_status(true);
                    break;
                }
                
                case EVENT_BT_CHARACTERISTIC_NOTIFY: {
                    bt_characteristic_data_t *data = (bt_characteristic_data_t *)event->payload;
                    process_ble_characteristic(data->characteristic_uuid, 
                                              data->data, data->length);
                    break;
                }
            }
            
            tesa_event_bus_free_event(event);
        }
    }
}
```

### 8.3 Serial Port (UART) Integration

UART communication often uses ISR-based reception and task-based processing.

#### Channel and Event Definitions

```c
#define CHANNEL_UART         0x4001

typedef enum {
    EVENT_UART_DATA_RECEIVED = 0x4001,
    EVENT_UART_TX_COMPLETE,
    EVENT_UART_ERROR,
    EVENT_UART_FRAMING_ERROR,
    EVENT_UART_OVERRUN_ERROR
} uart_event_type_t;

typedef struct {
    uint8_t data[256];
    uint16_t length;
    uint8_t uart_id;
} uart_data_t;

typedef struct {
    uint8_t uart_id;
    uint8_t error_type;
} uart_error_t;
```

#### UART RX ISR Handler

```c
void uart_rx_isr_handler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    static uint8_t rx_buffer[TESA_EVENT_BUS_MAX_PAYLOAD_SIZE];
    static uint16_t buffer_index = 0;
    
    // Read byte from UART
    uint8_t byte = uart_read_byte();
    
    // Check for buffer overflow
    if (buffer_index < sizeof(rx_buffer) - 1) {
        rx_buffer[buffer_index++] = byte;
        
        // Check for end of message (e.g., newline or timeout)
        if (byte == '\n' || byte == '\r') {
            uart_data_t data;
            data.uart_id = UART_ID_MAIN;
            data.length = buffer_index;
            memcpy(data.data, rx_buffer, buffer_index);
            
            tesa_event_bus_post_from_isr(
                CHANNEL_UART,
                EVENT_UART_DATA_RECEIVED,
                &data,
                sizeof(data),
                &xHigherPriorityTaskWoken
            );
            
            buffer_index = 0;
        }
    } else {
        // Buffer overflow
        uart_error_t error = {
            .uart_id = UART_ID_MAIN,
            .error_type = UART_ERROR_OVERRUN
        };
        
        tesa_event_bus_post_from_isr(
            CHANNEL_UART,
            EVENT_UART_OVERRUN_ERROR,
            &error,
            sizeof(error),
            &xHigherPriorityTaskWoken
        );
        
        buffer_index = 0;
    }
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
```

#### UART Command Processor Task

```c
void uart_command_processor_task(void *pvParameters) {
    QueueHandle_t uart_queue = (QueueHandle_t)pvParameters;
    
    for (;;) {
        tesa_event_t *event = NULL;
        
        if (xQueueReceive(uart_queue, &event, portMAX_DELAY) == pdTRUE) {
            switch (event->event_type) {
                case EVENT_UART_DATA_RECEIVED: {
                    uart_data_t *data = (uart_data_t *)event->payload;
                    
                    // Parse command
                    if (parse_command(data->data, data->length) == 0) {
                        // Execute command
                        execute_command(data->data, data->length);
                    } else {
                        // Invalid command
                        uart_send_error("Invalid command");
                    }
                    break;
                }
                
                case EVENT_UART_ERROR:
                case EVENT_UART_FRAMING_ERROR:
                case EVENT_UART_OVERRUN_ERROR: {
                    uart_error_t *error = (uart_error_t *)event->payload;
                    log_uart_error(error->uart_id, error->error_type);
                    handle_uart_error(error);
                    break;
                }
            }
            
            tesa_event_bus_free_event(event);
        }
    }
}
```

### 8.4 MQTT Integration

MQTT protocol events for publish/subscribe messaging.

#### Channel and Event Definitions

```c
#define CHANNEL_MQTT         0x5001

typedef enum {
    EVENT_MQTT_CONNECTED = 0x5001,
    EVENT_MQTT_DISCONNECTED,
    EVENT_MQTT_MESSAGE_RECEIVED,
    EVENT_MQTT_MESSAGE_PUBLISHED,
    EVENT_MQTT_SUBSCRIBE_ACK,
    EVENT_MQTT_ERROR,
    EVENT_MQTT_PUBLISH_ACK
} mqtt_event_type_t;

typedef struct {
    char topic[64];
    uint8_t qos;
    uint16_t message_id;
} mqtt_connection_t;

typedef struct {
    char topic[64];
    uint8_t payload[200];
    uint16_t payload_length;
    uint8_t qos;
    uint16_t message_id;
} mqtt_message_t;

typedef struct {
    uint8_t error_code;
    char error_message[32];
} mqtt_error_t;
```

#### Posting MQTT Events

```c
void mqtt_on_connected(const char *broker, uint16_t port) {
    mqtt_connection_t conn;
    strncpy(conn.topic, broker, sizeof(conn.topic) - 1);
    conn.qos = 1;
    conn.message_id = 0;
    
    tesa_event_bus_post(
        CHANNEL_MQTT,
        EVENT_MQTT_CONNECTED,
        &conn,
        sizeof(conn)
    );
}

void mqtt_on_message_received(const char *topic, const uint8_t *payload, 
                               uint16_t length, uint8_t qos) {
    mqtt_message_t msg;
    strncpy(msg.topic, topic, sizeof(msg.topic) - 1);
    msg.qos = qos;
    msg.payload_length = (length > 200) ? 200 : length;
    memcpy(msg.payload, payload, msg.payload_length);
    
    tesa_event_bus_post(
        CHANNEL_MQTT,
        EVENT_MQTT_MESSAGE_RECEIVED,
        &msg,
        sizeof(msg)
    );
}

void mqtt_on_publish_complete(uint16_t message_id) {
    uint16_t msg_id = message_id;
    
    tesa_event_bus_post(
        CHANNEL_MQTT,
        EVENT_MQTT_PUBLISH_ACK,
        &msg_id,
        sizeof(msg_id)
    );
}
```

#### MQTT Event Handler Task

```c
void mqtt_handler_task(void *pvParameters) {
    QueueHandle_t mqtt_queue = (QueueHandle_t)pvParameters;
    
    for (;;) {
        tesa_event_t *event = NULL;
        
        if (xQueueReceive(mqtt_queue, &event, portMAX_DELAY) == pdTRUE) {
            switch (event->event_type) {
                case EVENT_MQTT_CONNECTED: {
                    mqtt_connection_t *conn = (mqtt_connection_t *)event->payload;
                    printf("MQTT connected to: %s\n", conn->topic);
                    
                    // Subscribe to topics
                    mqtt_subscribe("device/sensor/data", 1);
                    mqtt_subscribe("device/command", 1);
                    break;
                }
                
                case EVENT_MQTT_DISCONNECTED:
                    printf("MQTT disconnected\n");
                    trigger_reconnection();
                    break;
                
                case EVENT_MQTT_MESSAGE_RECEIVED: {
                    mqtt_message_t *msg = (mqtt_message_t *)event->payload;
                    
                    // Process message based on topic
                    if (strcmp(msg->topic, "device/command") == 0) {
                        process_device_command(msg->payload, msg->payload_length);
                    } else if (strcmp(msg->topic, "device/sensor/data") == 0) {
                        process_sensor_config(msg->payload, msg->payload_length);
                    }
                    break;
                }
                
                case EVENT_MQTT_PUBLISH_ACK: {
                    uint16_t *msg_id = (uint16_t *)event->payload;
                    handle_publish_ack(*msg_id);
                    break;
                }
                
                case EVENT_MQTT_ERROR: {
                    mqtt_error_t *error = (mqtt_error_t *)event->payload;
                    log_mqtt_error(error->error_code, error->error_message);
                    break;
                }
            }
            
            tesa_event_bus_free_event(event);
        }
    }
}
```

### 8.5 LVGL GUI Integration

LVGL (Light and Versatile Graphics Library) GUI events for user interface updates.

#### Channel and Event Definitions

```c
#define CHANNEL_LVGL         0x6001

typedef enum {
    EVENT_LVGL_BUTTON_PRESSED = 0x6001,
    EVENT_LVGL_BUTTON_RELEASED,
    EVENT_LVGL_TOUCH_DOWN,
    EVENT_LVGL_TOUCH_UP,
    EVENT_LVGL_WIDGET_VALUE_CHANGED,
    EVENT_LVGL_SCREEN_CHANGED,
    EVENT_LVGL_UPDATE_REQUEST
} lvgl_event_type_t;

typedef struct {
    uint16_t widget_id;
    uint16_t x;
    uint16_t y;
} lvgl_input_event_t;

typedef struct {
    uint16_t widget_id;
    int32_t value;
    int32_t old_value;
} lvgl_value_change_t;

typedef struct {
    uint16_t screen_id;
    uint16_t prev_screen_id;
} lvgl_screen_change_t;
```

#### Posting LVGL Events

```c
// Post from LVGL input ISR (touch screen interrupt)
void lvgl_touch_isr_handler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    uint16_t x, y;
    read_touch_coordinates(&x, &y);
    
    lvgl_input_event_t touch_event = {
        .widget_id = 0,
        .x = x,
        .y = y
    };
    
    tesa_event_bus_post_from_isr(
        CHANNEL_LVGL,
        EVENT_LVGL_TOUCH_DOWN,
        &touch_event,
        sizeof(touch_event),
        &xHigherPriorityTaskWoken
    );
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// Post from LVGL event callback
void lvgl_button_event_cb(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e);
    uint16_t widget_id = (uint16_t)(uintptr_t)lv_obj_get_user_data(btn);
    
    lvgl_input_event_t button_event = {
        .widget_id = widget_id,
        .x = 0,
        .y = 0
    };
    
    tesa_event_bus_post(
        CHANNEL_LVGL,
        EVENT_LVGL_BUTTON_PRESSED,
        &button_event,
        sizeof(button_event)
    );
}

// Post screen update request
void request_screen_update(uint16_t screen_id) {
    uint16_t screen = screen_id;
    
    tesa_event_bus_post(
        CHANNEL_LVGL,
        EVENT_LVGL_UPDATE_REQUEST,
        &screen,
        sizeof(screen)
    );
}
```

#### LVGL Event Handler Task

```c
void lvgl_event_handler_task(void *pvParameters) {
    QueueHandle_t lvgl_queue = (QueueHandle_t)pvParameters;
    
    for (;;) {
        tesa_event_t *event = NULL;
        
        if (xQueueReceive(lvgl_queue, &event, pdMS_TO_TICKS(100)) == pdTRUE) {
            switch (event->event_type) {
                case EVENT_LVGL_BUTTON_PRESSED: {
                    lvgl_input_event_t *btn = (lvgl_input_event_t *)event->payload;
                    handle_button_press(btn->widget_id);
                    break;
                }
                
                case EVENT_LVGL_TOUCH_DOWN: {
                    lvgl_input_event_t *touch = (lvgl_input_event_t *)event->payload;
                    lvgl_send_touch_event(touch->x, touch->y, LV_INDEV_STATE_PRESSED);
                    break;
                }
                
                case EVENT_LVGL_TOUCH_UP: {
                    lvgl_input_event_t *touch = (lvgl_input_event_t *)event->payload;
                    lvgl_send_touch_event(touch->x, touch->y, LV_INDEV_STATE_RELEASED);
                    break;
                }
                
                case EVENT_LVGL_WIDGET_VALUE_CHANGED: {
                    lvgl_value_change_t *change = (lvgl_value_change_t *)event->payload;
                    update_widget_value(change->widget_id, change->value);
                    break;
                }
                
                case EVENT_LVGL_UPDATE_REQUEST: {
                    uint16_t *screen_id = (uint16_t *)event->payload;
                    lvgl_refresh_screen(*screen_id);
                    break;
                }
                
                case EVENT_LVGL_SCREEN_CHANGED: {
                    lvgl_screen_change_t *change = (lvgl_screen_change_t *)event->payload;
                    handle_screen_transition(change->prev_screen_id, change->screen_id);
                    break;
                }
            }
            
            tesa_event_bus_free_event(event);
        }
        
        // Allow LVGL to process (called periodically)
        lv_timer_handler();
    }
}
```

### 8.6 Sensor Integration (Gyroscope, Accelerometer, Temperature)

Sensor modules generate data ready events, often from ISR contexts.

#### Channel and Event Definitions

```c
#define CHANNEL_GYRO         0x7001
#define CHANNEL_ACCEL        0x7002
#define CHANNEL_TEMP         0x7003

typedef enum {
    EVENT_GYRO_DATA_READY = 0x7001,
    EVENT_GYRO_CALIBRATION_COMPLETE,
    EVENT_GYRO_ERROR,
    EVENT_ACCEL_DATA_READY = 0x7101,
    EVENT_ACCEL_IMPACT_DETECTED,
    EVENT_ACCEL_ERROR,
    EVENT_TEMP_DATA_READY = 0x7201,
    EVENT_TEMP_THRESHOLD_EXCEEDED,
    EVENT_TEMP_ERROR
} sensor_event_type_t;

typedef struct {
    float x;
    float y;
    float z;
    uint32_t timestamp;
} gyro_data_t;

typedef struct {
    float x;
    float y;
    float z;
    float magnitude;
    uint32_t timestamp;
} accel_data_t;

typedef struct {
    float temperature;
    float humidity;  // If sensor supports it
    uint32_t timestamp;
} temp_data_t;

typedef struct {
    uint8_t sensor_id;
    uint8_t error_code;
} sensor_error_t;
```

#### Sensor ISR Handlers

```c
// Gyroscope data ready interrupt
void gyro_data_ready_isr(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    gyro_data_t gyro;
    read_gyroscope_xyz(&gyro.x, &gyro.y, &gyro.z);
    gyro.timestamp = tesa_event_bus_get_timestamp_ms();
    
    // Use DROP_OLDEST policy for high-frequency sensor data
    tesa_event_bus_post_from_isr(
        CHANNEL_GYRO,
        EVENT_GYRO_DATA_READY,
        &gyro,
        sizeof(gyro),
        &xHigherPriorityTaskWoken
    );
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// Accelerometer data ready interrupt
void accel_data_ready_isr(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    accel_data_t accel;
    read_accelerometer_xyz(&accel.x, &accel.y, &accel.z);
    accel.magnitude = sqrtf(accel.x * accel.x + 
                           accel.y * accel.y + 
                           accel.z * accel.z);
    accel.timestamp = tesa_event_bus_get_timestamp_ms();
    
    // Check for impact detection
    if (accel.magnitude > IMPACT_THRESHOLD) {
        tesa_event_bus_post_from_isr(
            CHANNEL_ACCEL,
            EVENT_ACCEL_IMPACT_DETECTED,  // Critical event
            &accel,
            sizeof(accel),
            &xHigherPriorityTaskWoken
        );
    } else {
        tesa_event_bus_post_from_isr(
            CHANNEL_ACCEL,
            EVENT_ACCEL_DATA_READY,
            &accel,
            sizeof(accel),
            &xHigherPriorityTaskWoken
        );
    }
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// Temperature sensor timer callback (periodic sampling)
void temp_timer_callback(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    temp_data_t temp;
    temp.temperature = read_temperature();
    temp.humidity = read_humidity();  // If available
    temp.timestamp = tesa_event_bus_get_timestamp_ms();
    
    tesa_event_bus_post_from_isr(
        CHANNEL_TEMP,
        EVENT_TEMP_DATA_READY,
        &temp,
        sizeof(temp),
        &xHigherPriorityTaskWoken
    );
    
    // Check threshold
    if (temp.temperature > TEMP_HIGH_THRESHOLD || 
        temp.temperature < TEMP_LOW_THRESHOLD) {
        tesa_event_bus_post_from_isr(
            CHANNEL_TEMP,
            EVENT_TEMP_THRESHOLD_EXCEEDED,  // Critical event
            &temp,
            sizeof(temp),
            &xHigherPriorityTaskWoken
        );
    }
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
```

#### Sensor Channel Registration

```c
void sensor_init_channels(void) {
    // Gyroscope - high frequency, drop oldest
    tesa_event_bus_channel_config_t gyro_config = {
        .queue_policy = TESA_EVENT_BUS_QUEUE_DROP_OLDEST,
        .queue_timeout_ticks = 0
    };
    tesa_event_bus_register_channel_with_config(
        CHANNEL_GYRO,
        "Gyroscope",
        &gyro_config
    );
    
    // Accelerometer - high frequency normal, but impact is critical
    tesa_event_bus_channel_config_t accel_config = {
        .queue_policy = TESA_EVENT_BUS_QUEUE_DROP_OLDEST,
        .queue_timeout_ticks = 0
    };
    tesa_event_bus_register_channel_with_config(
        CHANNEL_ACCEL,
        "Accelerometer",
        &accel_config
    );
    
    // Temperature - lower frequency, no drop for threshold events
    tesa_event_bus_channel_config_t temp_config = {
        .queue_policy = TESA_EVENT_BUS_QUEUE_DROP_OLDEST,
        .queue_timeout_ticks = 0
    };
    tesa_event_bus_register_channel_with_config(
        CHANNEL_TEMP,
        "Temperature",
        &temp_config
    );
}
```

#### Sensor Processing Task

```c
void sensor_processing_task(void *pvParameters) {
    QueueHandle_t sensor_queue = (QueueHandle_t)pvParameters;
    
    for (;;) {
        tesa_event_t *event = NULL;
        
        if (xQueueReceive(sensor_queue, &event, portMAX_DELAY) == pdTRUE) {
            switch (event->channel_id) {
                case CHANNEL_GYRO:
                    process_gyro_event(event);
                    break;
                    
                case CHANNEL_ACCEL:
                    process_accel_event(event);
                    break;
                    
                case CHANNEL_TEMP:
                    process_temp_event(event);
                    break;
            }
            
            tesa_event_bus_free_event(event);
        }
    }
}

void process_gyro_event(tesa_event_t *event) {
    if (event->event_type == EVENT_GYRO_DATA_READY) {
        gyro_data_t *gyro = (gyro_data_t *)event->payload;
        
        // Calculate orientation, rotation rate, etc.
        calculate_orientation(gyro);
        
        // Filter data
        apply_gyro_filter(gyro);
        
        // Store for fusion with accelerometer
        update_sensor_fusion_gyro(gyro);
    }
}

void process_accel_event(tesa_event_t *event) {
    switch (event->event_type) {
        case EVENT_ACCEL_DATA_READY: {
            accel_data_t *accel = (accel_data_t *)event->payload;
            update_sensor_fusion_accel(accel);
            break;
        }
        
        case EVENT_ACCEL_IMPACT_DETECTED: {
            accel_data_t *accel = (accel_data_t *)event->payload;
            // Critical event - must be handled
            trigger_impact_alarm(accel);
            log_impact_event(accel);
            break;
        }
        
        case EVENT_ACCEL_ERROR: {
            sensor_error_t *error = (sensor_error_t *)event->payload;
            handle_sensor_error(CHANNEL_ACCEL, error);
            break;
        }
    }
}

void process_temp_event(tesa_event_t *event) {
    switch (event->event_type) {
        case EVENT_TEMP_DATA_READY: {
            temp_data_t *temp = (temp_data_t *)event->payload;
            update_temperature_display(temp->temperature);
            break;
        }
        
        case EVENT_TEMP_THRESHOLD_EXCEEDED: {
            temp_data_t *temp = (temp_data_t *)event->payload;
            // Critical - trigger alarm
            trigger_temperature_alarm(temp->temperature);
            break;
        }
    }
}
```

---

## 9. Complete System Integration Example

This section demonstrates how all modules work together in a complete medical device system.

### 9.1 System Architecture

A typical medical device system integrates multiple modules:

```
Sensors (Gyro, Accel, Temp)
    ↓
Event Bus Channels
    ↓
Processing Tasks → Display (LVGL) + Network (WiFi/MQTT) + Storage (Serial/UART)
```

### 9.2 Channel Assignment Strategy

Organize channels by functional area:

```c
// Communication channels (0x2000-0x2FFF)
#define CHANNEL_WIFI         0x2001
#define CHANNEL_BLUETOOTH    0x2002
#define CHANNEL_SERIAL       0x2003
#define CHANNEL_MQTT         0x2004

// Sensor channels (0x7000-0x7FFF)
#define CHANNEL_GYRO         0x7001
#define CHANNEL_ACCEL        0x7002
#define CHANNEL_TEMP         0x7003

// User Interface channels (0x6000-0x6FFF)
#define CHANNEL_LVGL         0x6001

// System channels (0x1000-0x1FFF)
#define CHANNEL_SYSTEM       0x1001
#define CHANNEL_ALARM        0x1002
```

### 9.3 Complete Initialization

```c
void system_init_complete(void) {
    // 1. Initialize event bus first
    tesa_event_bus_init();
    
    // 2. Register all channels with appropriate policies
    register_all_channels();
    
    // 3. Create all subscriber queues
    create_all_queues();
    
    // 4. Subscribe modules to their channels
    subscribe_all_modules();
    
    // 5. Create processing tasks
    create_all_tasks();
    
    // 6. Initialize hardware modules
    init_hardware_modules();
}

void register_all_channels(void) {
    // Critical channels - guaranteed delivery
    tesa_event_bus_channel_config_t critical_config = {
        .queue_policy = TESA_EVENT_BUS_QUEUE_NO_DROP,
        .queue_timeout_ticks = pdMS_TO_TICKS(1000)
    };
    tesa_event_bus_register_channel_with_config(
        CHANNEL_ALARM,
        "Alarm",
        &critical_config
    );
    
    // High-frequency sensor channels - drop oldest
    tesa_event_bus_channel_config_t sensor_config = {
        .queue_policy = TESA_EVENT_BUS_QUEUE_DROP_OLDEST,
        .queue_timeout_ticks = 0
    };
    tesa_event_bus_register_channel_with_config(
        CHANNEL_GYRO,
        "Gyroscope",
        &sensor_config
    );
    tesa_event_bus_register_channel_with_config(
        CHANNEL_ACCEL,
        "Accelerometer",
        &sensor_config
    );
    
    // Network channels - drop oldest for data, no drop for status
    tesa_event_bus_register_channel_with_config(
        CHANNEL_WIFI,
        "WiFi",
        &critical_config  // Status events critical
    );
    
    tesa_event_bus_channel_config_t data_config = {
        .queue_policy = TESA_EVENT_BUS_QUEUE_DROP_OLDEST,
        .queue_timeout_ticks = 0
    };
    tesa_event_bus_register_channel_with_config(
        CHANNEL_MQTT,
        "MQTT",
        &data_config
    );
    
    // UI channel - normal priority
    tesa_event_bus_register_channel_with_config(
        CHANNEL_LVGL,
        "LVGL",
        &data_config
    );
}
```

### 9.4 Data Flow Example

Complete data flow from sensor to display and network:

```c
// Sensor data flows through the system:

// 1. Gyroscope ISR posts data
void gyro_data_ready_isr(void) {
    gyro_data_t gyro = read_gyroscope();
    tesa_event_bus_post_from_isr(CHANNEL_GYRO, EVENT_GYRO_DATA_READY,
                                 &gyro, sizeof(gyro), &xHigherPriorityTaskWoken);
}

// 2. Sensor fusion task processes multiple sensors
void sensor_fusion_task(void *pvParameters) {
    QueueHandle_t gyro_queue = get_gyro_queue();
    QueueHandle_t accel_queue = get_accel_queue();
    
    for (;;) {
        // Process gyro data
        tesa_event_t *gyro_event = NULL;
        if (xQueueReceive(gyro_queue, &gyro_event, pdMS_TO_TICKS(100)) == pdTRUE) {
            gyro_data_t *gyro = (gyro_data_t *)gyro_event->payload;
            update_fusion_gyro(gyro);
            tesa_event_bus_free_event(gyro_event);
        }
        
        // Process accel data
        tesa_event_t *accel_event = NULL;
        if (xQueueReceive(accel_queue, &accel_event, pdMS_TO_TICKS(100)) == pdTRUE) {
            accel_data_t *accel = (accel_data_t *)accel_event->payload;
            update_fusion_accel(accel);
            tesa_event_bus_free_event(accel_event);
        }
        
        // Perform sensor fusion
        orientation_t orientation = perform_sensor_fusion();
        
        // Broadcast fused data to multiple subscribers
        tesa_event_bus_post(CHANNEL_SYSTEM, EVENT_ORIENTATION_UPDATE,
                           &orientation, sizeof(orientation));
        // This reaches: GUI task, Network task, Logger task
    }
}

// 3. GUI task updates display
void gui_update_task(void *pvParameters) {
    QueueHandle_t system_queue = get_system_queue();
    
    for (;;) {
        tesa_event_t *event = NULL;
        if (xQueueReceive(system_queue, &event, portMAX_DELAY) == pdTRUE) {
            if (event->event_type == EVENT_ORIENTATION_UPDATE) {
                orientation_t *orient = (orientation_t *)event->payload;
                update_orientation_display(orient);
            }
            tesa_event_bus_free_event(event);
        }
    }
}

// 4. Network task sends to MQTT
void network_task(void *pvParameters) {
    QueueHandle_t system_queue = get_system_queue();
    
    for (;;) {
        tesa_event_t *event = NULL;
        if (xQueueReceive(system_queue, &event, portMAX_DELAY) == pdTRUE) {
            if (event->event_type == EVENT_ORIENTATION_UPDATE) {
                orientation_t *orient = (orientation_t *)event->payload;
                
                // Send to MQTT broker
                char json[256];
                snprintf(json, sizeof(json),
                        "{\"x\":%.2f,\"y\":%.2f,\"z\":%.2f}",
                        orient->x, orient->y, orient->z);
                
                mqtt_publish("device/orientation", json, strlen(json), 1);
            }
            tesa_event_bus_free_event(event);
        }
    }
}
```

---

## 10. Best Practices

### 10.1 Memory Management Do's and Don'ts

#### DO:
- Always call `tesa_event_bus_free_event()` immediately after processing
- Check event pointer before accessing payload
- Verify payload size matches expected structure size
- Use stack or static variables for event posting payloads

```c
// Correct usage
void correct_event_handling(tesa_event_t *event) {
    if (event != NULL && event->payload != NULL) {
        if (event->payload_size == sizeof(my_struct_t)) {
            my_struct_t *data = (my_struct_t *)event->payload;
            process_data(data);
        }
    }
    tesa_event_bus_free_event(event);  // Always free
}
```

#### DON'T:
- Never store event pointers for later use
- Never free payload memory directly
- Never access payload after freeing event
- Never cache events across task context switches

```c
// WRONG - storing event pointer
tesa_event_t *cached_event = NULL;

void wrong_event_handling(tesa_event_t *event) {
    cached_event = event;  // DON'T DO THIS
    // ... later ...
    process_event(cached_event);  // May be invalid!
}

// WRONG - manual memory free
void wrong_free(tesa_event_t *event) {
    free(event->payload);  // DON'T DO THIS
    free(event);           // DON'T DO THIS
}
```

### 10.2 Thread Safety Considerations

- Event posting is thread-safe (protected by critical sections)
- Multiple tasks can post to same channel simultaneously
- ISR and task contexts can post to same channel
- Always use `tesa_event_bus_post_from_isr()` in ISR context
- Always use `tesa_event_bus_post()` in task context

```c
// Safe: Multiple tasks posting
void task1(void *pv) {
    tesa_event_bus_post(CHANNEL_DATA, EVENT_DATA, &data1, sizeof(data1));
}

void task2(void *pv) {
    tesa_event_bus_post(CHANNEL_DATA, EVENT_DATA, &data2, sizeof(data2));
}

// Safe: ISR and task posting to same channel
void isr_handler(void) {
    tesa_event_bus_post_from_isr(CHANNEL_DATA, EVENT_DATA, &data, 
                                 sizeof(data), &xHigherPriorityTaskWoken);
}

void task_handler(void *pv) {
    tesa_event_bus_post(CHANNEL_DATA, EVENT_DATA, &data, sizeof(data));
}
```

### 10.3 Performance Optimization Tips

1. **Queue Sizing**: Size queues based on expected event rate and processing time
   ```c
   // High-frequency sensor: larger queue
   QueueHandle_t sensor_queue = xQueueCreate(20, sizeof(tesa_event_t *));
   
   // Low-frequency commands: smaller queue
   QueueHandle_t cmd_queue = xQueueCreate(5, sizeof(tesa_event_t *));
   ```

2. **Queue Policy Selection**: Match policy to use case
   - High-frequency data: `DROP_OLDEST`
   - Critical events: `NO_DROP`
   - Status updates: `DROP_NEWEST`

3. **Event Processing**: Keep subscriber tasks fast
   ```c
   // Good: Fast processing, defer heavy work
   void fast_subscriber(tesa_event_t *event) {
       sensor_data_t *data = (sensor_data_t *)event->payload;
       
       // Quick validation
       if (is_valid(data)) {
           // Defer to processing task via another channel
           tesa_event_bus_post(CHANNEL_PROCESSING, EVENT_PROCESS, 
                              data, sizeof(*data));
       }
       tesa_event_bus_free_event(event);
   }
   ```

4. **ISR Minimization**: Keep ISR handlers minimal
   ```c
   // Good: Read data, post event, exit quickly
   void minimal_isr(void) {
       data_t data = read_hardware_register();
       tesa_event_bus_post_from_isr(CHANNEL, EVENT, &data, sizeof(data), &x);
       portYIELD_FROM_ISR(x);
   }
   ```

### 10.4 Medical Device Specific Recommendations

1. **Critical Event Channels**: Use `NO_DROP` policy for safety-critical events
   ```c
   // Alarm channel - must be delivered
   tesa_event_bus_channel_config_t alarm_config = {
       .queue_policy = TESA_EVENT_BUS_QUEUE_NO_DROP,
       .queue_timeout_ticks = pdMS_TO_TICKS(1000)
   };
   ```

2. **Health Monitoring**: Implement periodic health checks
   ```c
   void health_monitor_task(void *pv) {
       for (;;) {
           check_all_critical_channels();
           vTaskDelay(pdMS_TO_TICKS(5000));
       }
   }
   ```

3. **Watchdog Integration**: Monitor event bus health
   ```c
   void check_event_bus_health(void) {
       tesa_event_bus_subscriber_stats_t stats;
       uint8_t count;
       
       tesa_event_bus_get_channel_stats(CHANNEL_ALARM, &stats, &count);
       
       if (stats.posts_dropped > 0) {
           // Critical: alarms are being dropped
           trigger_watchdog_alert();
       }
   }
   ```

4. **Deterministic Behavior**: All paths must be bounded
   - Event pool size is fixed (deterministic)
   - Queue operations have bounded execution time
   - No dynamic allocation (predictable memory usage)

5. **Audit Trail**: Log critical events
   ```c
   void post_alarm_with_logging(alarm_data_t *alarm) {
       tesa_event_bus_result_t result = tesa_event_bus_post(
           CHANNEL_ALARM, EVENT_ALARM, alarm, sizeof(*alarm));
       
       // Log all alarm posts for audit
       log_alarm_event(alarm, result);
       
       if (TESA_EVENT_BUS_SUCCESS != result) {
           log_critical("Alarm delivery failed: %d", result);
       }
   }
   ```

### 10.5 Channel ID Allocation Strategy

Organize channel IDs by functional area:

```c
// System channels: 0x1000-0x1FFF
#define CHANNEL_SYSTEM       0x1001
#define CHANNEL_ALARM        0x1002
#define CHANNEL_STATUS       0x1003

// Communication: 0x2000-0x2FFF
#define CHANNEL_WIFI         0x2001
#define CHANNEL_BLUETOOTH    0x2002
#define CHANNEL_SERIAL       0x2003
#define CHANNEL_MQTT         0x2004

// User Interface: 0x6000-0x6FFF
#define CHANNEL_LVGL         0x6001
#define CHANNEL_DISPLAY      0x6002

// Sensors: 0x7000-0x7FFF
#define CHANNEL_GYRO         0x7001
#define CHANNEL_ACCEL        0x7002
#define CHANNEL_TEMP         0x7003
```

### 10.6 Event Type Enumeration Organization

Use consistent event type numbering:

```c
// Module-specific event types
// Format: 0xMMMMNNNN where MMMM is module ID, NNNN is event number

// WiFi events: 0x2001-0x20FF
typedef enum {
    EVENT_WIFI_CONNECTED = 0x2001,
    EVENT_WIFI_DISCONNECTED = 0x2002,
    // ...
} wifi_event_type_t;

// Sensor events: 0x7001-0x70FF
typedef enum {
    EVENT_GYRO_DATA_READY = 0x7001,
    EVENT_GYRO_CALIBRATION = 0x7002,
    // ...
} sensor_event_type_t;
```

### 10.7 Coding Standards for Firmware

Follow firmware programming standards for safety and reliability:

#### Comparison Style (Yoda Conditions)

**Recommended**: Use Yoda conditions (constant first) for all equality comparisons:

```c
// ✅ CORRECT - Constant first (recommended for firmware)
if (TESA_EVENT_BUS_SUCCESS == result) {
    // Success handling
}

if (pdTRUE == xQueueReceive(queue, &event, timeout)) {
    // Process event
}

// ❌ AVOID - Variable first (can lead to accidental assignment)
if (result == TESA_EVENT_BUS_SUCCESS) {  // Typo: result = ... would compile
    // ...
}
```

**Rationale**:
- **Prevents accidental assignment**: If you mistype `=` instead of `==`, the compiler will error because you cannot assign to a constant
- **MISRA C compliant**: Recommended by MISRA C and other safety standards
- **Defensive programming**: Common practice in safety-critical firmware
- **Medical device standards**: Aligns with IEC 62304 and ISO 14971 requirements

**Example of the protection**:
```c
// Dangerous typo - compiles but assigns instead of compares
if (result = TESA_EVENT_BUS_SUCCESS) {  // Bug: assignment, always true!

// Safe - compiler error prevents the bug
if (TESA_EVENT_BUS_SUCCESS = result) {  // Error: cannot assign to constant
```

#### Return Code Checking

Always check return codes using Yoda conditions:

```c
// ✅ CORRECT
tesa_event_bus_result_t result = tesa_event_bus_post(...);
if (TESA_EVENT_BUS_SUCCESS != result) {
    handle_error(result);
}

// ✅ CORRECT - Switch statement (no Yoda needed)
switch (result) {
    case TESA_EVENT_BUS_SUCCESS:
        break;
    case TESA_EVENT_BUS_ERROR_MEMORY:
        handle_memory_error();
        break;
}
```

#### FreeRTOS Return Values

Apply Yoda conditions to FreeRTOS return values as well:

```c
// ✅ CORRECT
if (pdTRUE == xQueueReceive(queue, &event, timeout)) {
    process_event(event);
}

BaseType_t result = tesa_event_bus_post_from_isr(...);
if (pdTRUE == result) {
    // Success
}
```

### 10.8 Common Pitfalls to Avoid

1. **Forgetting to free events**: Always free after processing
2. **Accessing freed events**: Never use event after calling `tesa_event_bus_free_event()`
3. **Payload pointer issues**: Copy data, don't store pointers to external buffers
4. **Wrong ISR function**: Always use `_from_isr()` version in ISR context
5. **Ignoring return codes**: Always check and handle errors appropriately
6. **Incorrect queue policy**: Choose policy based on use case requirements
7. **Queue size too small**: Size queues based on event rate and processing time
8. **Incorrect comparison style**: Use Yoda conditions (constant first) for safety

---

## Conclusion

The TESA Event Bus provides a robust, deterministic, and memory-safe communication mechanism for medical device firmware. By following the patterns and best practices outlined in this guide, you can build reliable, maintainable systems that meet regulatory requirements.

For additional information, refer to:
- [Memory Ownership Documentation](tesa_event_bus_memory.md)
- [Safety Analysis](event_bus_safety_analysi.md)
- [API Reference](../tesa_event_bus.h)

---

**Document Version**: 1.0  
**Last Updated**: 2024  
**Target Platform**: FreeRTOS, Medical Device Firmware
