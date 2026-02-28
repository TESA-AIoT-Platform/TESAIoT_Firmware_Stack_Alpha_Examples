# Example 4 Implementation Plan: Multiple Subscribers and Broadcasting

## Overview

Example 4 demonstrates one-to-many event broadcasting where a single publisher posts events that are received by multiple subscribers on the same channel.

## Objectives

1. Demonstrate multiple subscribers on same channel
2. Show independent event copies per subscriber
3. Demonstrate different subscriber processing speeds
4. Show partial success scenarios
5. Illustrate broadcasting pattern

## Implementation Details

### Channel Configuration

Create a single channel for system status broadcasting:

- **Channel 1**: System status (broadcast to multiple subscribers)

### Header File Structure (`tesa_event_bus_example_4.h`)

```c
#define EXAMPLE_4_CHANNEL_STATUS  0x0401U

#define EXAMPLE_4_EVENT_STATUS_UPDATE  0x0401U
#define EXAMPLE_4_EVENT_ALERT          0x0402U

typedef struct {
    uint32_t sequence;
    uint8_t battery_level;
    int16_t temperature;
    uint32_t uptime_seconds;
} example_4_status_data_t;

typedef struct {
    uint8_t alert_type;
    char message[32];
} example_4_alert_data_t;

void tesa_event_bus_example_4(void);
```

### Source File Structure (`tesa_event_bus_example_4.c`)

#### Constants

```c
#define EXAMPLE_4_QUEUE_LENGTH 10U
#define EXAMPLE_4_TASK_STACK_SIZE 1024U
#define EXAMPLE_4_PUBLISHER_PRIORITY 5U
#define EXAMPLE_4_SUBSCRIBER_PRIORITY_DISPLAY 4U
#define EXAMPLE_4_SUBSCRIBER_PRIORITY_LOGGER 3U
#define EXAMPLE_4_SUBSCRIBER_PRIORITY_NETWORK 3U
#define EXAMPLE_4_PUBLISHER_DELAY_MS 1000U
```

#### Static Variables

```c
static QueueHandle_t example_4_queue_display = NULL;
static QueueHandle_t example_4_queue_logger = NULL;
static QueueHandle_t example_4_queue_network = NULL;
```

#### Main Function Flow

1. Initialize event bus
2. Register system status channel (default config)
3. Create 3 subscriber queues (display, logger, network)
4. Subscribe all 3 queues to the same channel
5. Create publisher task
6. Create 3 subscriber tasks with different priorities/processing speeds:
   - Display task: Fast processing (high priority)
   - Logger task: Medium processing (writes to log)
   - Network task: Slow processing (simulates network delay)

#### Publisher Task

- Posts system status updates periodically
- Posts alerts occasionally
- Logs when posting succeeds/fails
- Handles partial success (some subscribers receive, others don't)

#### Subscriber Tasks

**Display Task** (Fast):
- Processes events quickly
- Updates display (simulated)
- Logs received events

**Logger Task** (Medium):
- Processes events at medium speed
- Writes to log (simulated with delay)
- Logs received events

**Network Task** (Slow):
- Processes events slowly (simulates network transmission)
- May cause queue to fill up
- Demonstrates partial success when queue full

### Key Demonstration Points

1. **Broadcasting**: One post reaches all subscribers
2. **Independent Copies**: Each subscriber gets its own event copy
3. **Independent Queues**: Each subscriber has its own queue
4. **Partial Success**: If one subscriber's queue is full, others still receive
5. **Different Speeds**: Fast subscribers keep up, slow subscribers may drop events

### Expected Behavior

- Publisher posts status update
- All 3 subscribers receive the event (if queues not full)
- Display task processes quickly
- Logger task processes at medium speed
- Network task processes slowly, may drop events if queue fills
- Publisher may see `TESA_EVENT_BUS_ERROR_PARTIAL_SUCCESS` if network queue is full

### Testing Strategy

1. Start with all subscribers processing normally
2. Slow down network subscriber to demonstrate queue filling
3. Show partial success scenario
4. Speed up network subscriber to show recovery

### Statistics Demonstration

This example sets up for Example 5, which will show statistics:
- Display subscriber: High success rate
- Logger subscriber: High success rate
- Network subscriber: May have drops if slow
