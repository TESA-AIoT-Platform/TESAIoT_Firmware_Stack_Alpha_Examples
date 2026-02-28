# Example 5 Implementation Plan: Statistics and Monitoring

## Overview

Example 5 demonstrates how to retrieve and use event bus statistics for health monitoring and system diagnostics.

## Objectives

1. Demonstrate subscriber statistics retrieval
2. Show channel-level aggregate statistics
3. Implement health monitoring patterns
4. Calculate and analyze drop rates
5. Make decisions based on statistics

## Implementation Details

### Channel Configuration

Reuse Example 4 setup with multiple subscribers:

- **Channel 1**: System status (multiple subscribers)

### Header File Structure (`tesa_event_bus_example_5.h`)

```c
#define EXAMPLE_5_CHANNEL_STATUS  0x0501U

#define EXAMPLE_5_EVENT_STATUS_UPDATE  0x0501U

typedef struct {
    uint32_t sequence;
    uint8_t battery_level;
    int16_t temperature;
} example_5_status_data_t;

void tesa_event_bus_example_5(void);
```

### Source File Structure (`tesa_event_bus_example_5.c`)

#### Constants

```c
#define EXAMPLE_5_QUEUE_LENGTH 5U
#define EXAMPLE_5_TASK_STACK_SIZE 1024U
#define EXAMPLE_5_PUBLISHER_PRIORITY 5U
#define EXAMPLE_5_SUBSCRIBER_PRIORITY 4U
#define EXAMPLE_5_MONITOR_PRIORITY 6U
#define EXAMPLE_5_PUBLISHER_DELAY_MS 50U
#define EXAMPLE_5_MONITOR_INTERVAL_MS 5000U
```

#### Static Variables

```c
static QueueHandle_t example_5_queue_subscriber1 = NULL;
static QueueHandle_t example_5_queue_subscriber2 = NULL;
static QueueHandle_t example_5_queue_subscriber3 = NULL;
```

#### Main Function Flow

1. Initialize event bus
2. Register channel
3. Create 3 subscriber queues
4. Subscribe all queues to channel
5. Create publisher task (posts rapidly)
6. Create 3 subscriber tasks (one slow to cause drops)
7. Create health monitor task (checks statistics periodically)

#### Publisher Task

- Posts events rapidly (faster than slow subscriber can process)
- Logs posting results
- Continues posting regardless of drops

#### Subscriber Tasks

- Subscriber 1: Fast processing
- Subscriber 2: Fast processing
- Subscriber 3: Slow processing (causes drops)

#### Health Monitor Task

Periodically (every 5 seconds):
1. Get statistics for each subscriber
2. Get channel aggregate statistics
3. Calculate drop rates
4. Log statistics
5. Alert if drop rate exceeds threshold (>10%)
6. Alert if recent drops detected

```c
static void example_5_monitor_task(void *pvParameters) {
    (void)pvParameters;
    
    for (;;) {
        tesa_event_bus_subscriber_stats_t stats1, stats2, stats3;
        tesa_event_bus_subscriber_stats_t channel_stats;
        uint8_t subscriber_count;
        tesa_event_bus_result_t result;
        
        vTaskDelay(pdMS_TO_TICKS(EXAMPLE_5_MONITOR_INTERVAL_MS));
        
        result = tesa_event_bus_get_subscriber_stats(
            EXAMPLE_5_CHANNEL_STATUS,
            example_5_queue_subscriber1,
            &stats1
        );
        if (TESA_EVENT_BUS_SUCCESS == result) {
            calculate_and_log_drop_rate("Subscriber1", &stats1);
        }
        
        result = tesa_event_bus_get_subscriber_stats(
            EXAMPLE_5_CHANNEL_STATUS,
            example_5_queue_subscriber2,
            &stats2
        );
        if (TESA_EVENT_BUS_SUCCESS == result) {
            calculate_and_log_drop_rate("Subscriber2", &stats2);
        }
        
        result = tesa_event_bus_get_subscriber_stats(
            EXAMPLE_5_CHANNEL_STATUS,
            example_5_queue_subscriber3,
            &stats3
        );
        if (TESA_EVENT_BUS_SUCCESS == result) {
            calculate_and_log_drop_rate("Subscriber3", &stats3);
        }
        
        result = tesa_event_bus_get_channel_stats(
            EXAMPLE_5_CHANNEL_STATUS,
            &channel_stats,
            &subscriber_count
        );
        if (TESA_EVENT_BUS_SUCCESS == result) {
            log_channel_stats(&channel_stats, subscriber_count);
        }
    }
}
```

#### Helper Functions

**Calculate Drop Rate**:
```c
static void calculate_and_log_drop_rate(const char *name, 
                                       tesa_event_bus_subscriber_stats_t *stats) {
    uint32_t total = stats->posts_successful + stats->posts_dropped;
    
    if (total > 0U) {
        float drop_rate = (float)stats->posts_dropped / (float)total;
        
        TESA_LOG_INFO("[%s] Success: %lu, Dropped: %lu, Rate: %.2f%%",
                     name,
                     (unsigned long)stats->posts_successful,
                     (unsigned long)stats->posts_dropped,
                     drop_rate * 100.0f);
        
        if (drop_rate > 0.1f) {
            TESA_LOG_WARNING("[%s] High drop rate detected!", name);
        }
    }
}
```

**Check Recent Drops**:
```c
static void check_recent_drops(tesa_event_bus_subscriber_stats_t *stats) {
    uint32_t current_time = tesa_event_bus_get_timestamp_ms();
    uint32_t time_since_drop;
    
    if (stats->last_drop_timestamp_ms > 0U) {
        if (current_time >= stats->last_drop_timestamp_ms) {
            time_since_drop = current_time - stats->last_drop_timestamp_ms;
        } else {
            time_since_drop = (TESA_EVENT_BUS_TIMESTAMP_ROLLOVER_MS - 
                             stats->last_drop_timestamp_ms) + current_time;
        }
        
        if (time_since_drop < 1000U) {
            TESA_LOG_WARNING("Recent drop detected: %lu ms ago", 
                           (unsigned long)time_since_drop);
        }
    }
}
```

### Key Demonstration Points

1. **Subscriber Statistics**: Individual subscriber performance
2. **Channel Statistics**: Aggregate performance across all subscribers
3. **Drop Rate Calculation**: Percentage of dropped events
4. **Recent Drop Detection**: Timestamp-based drop detection
5. **Health Monitoring**: Periodic health checks
6. **Alerting**: Alert on high drop rates or recent drops

### Expected Behavior

- Publisher posts events rapidly
- Fast subscribers: Low drop rate
- Slow subscriber: High drop rate
- Monitor task logs statistics every 5 seconds
- Alerts generated when drop rate > 10%
- Alerts generated for recent drops

### Statistics Fields

- `posts_successful`: Number of successfully delivered events
- `posts_dropped`: Number of dropped events
- `last_drop_timestamp_ms`: Timestamp of last drop (0 if never dropped)

### Use Cases

1. **Health Monitoring**: Periodic checks of system health
2. **Performance Analysis**: Identify slow subscribers
3. **Debugging**: Understand why events are being dropped
4. **Alerting**: Trigger alerts on high drop rates
5. **Optimization**: Identify bottlenecks
