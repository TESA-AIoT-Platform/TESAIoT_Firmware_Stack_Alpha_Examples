# Example 2 Implementation Plan: Queue Policies and Channel Configuration

## Overview

Example 2 demonstrates all four queue policies and custom channel configuration. This example shows how different queue policies behave under various conditions, particularly when queues become full.

## Objectives

1. Register multiple channels with different queue policies
2. Demonstrate behavior of each policy when queues fill up
3. Show custom timeout configuration
4. Illustrate when to use each policy

## Implementation Details

### Channel Configuration

Create 4 channels, one for each queue policy:

- **Channel 1**: DROP_NEWEST - High-frequency sensor data
- **Channel 2**: DROP_OLDEST - Status updates (keep latest)
- **Channel 3**: NO_DROP - Critical alarms (guaranteed delivery)
- **Channel 4**: WAIT - Commands (wait with timeout)

### Header File Structure (`tesa_event_bus_example_2.h`)

```c
#define EXAMPLE_2_CHANNEL_DROP_NEWEST  0x0201U
#define EXAMPLE_2_CHANNEL_DROP_OLDEST  0x0202U
#define EXAMPLE_2_CHANNEL_NO_DROP     0x0203U
#define EXAMPLE_2_CHANNEL_WAIT         0x0204U

#define EXAMPLE_2_EVENT_DATA           0x0201U
#define EXAMPLE_2_EVENT_STATUS         0x0202U
#define EXAMPLE_2_EVENT_ALARM          0x0203U
#define EXAMPLE_2_EVENT_COMMAND        0x0204U

typedef struct {
    uint32_t sequence;
    uint32_t value;
} example_2_data_t;

void tesa_event_bus_example_2(void);
```

### Source File Structure (`tesa_event_bus_example_2.c`)

#### Constants

```c
#define EXAMPLE_2_QUEUE_LENGTH 5U
#define EXAMPLE_2_TASK_STACK_SIZE 1024U
#define EXAMPLE_2_PUBLISHER_PRIORITY 5U
#define EXAMPLE_2_SUBSCRIBER_PRIORITY 4U
#define EXAMPLE_2_PUBLISHER_DELAY_MS 100U
#define EXAMPLE_2_NO_DROP_TIMEOUT_MS 1000U
#define EXAMPLE_2_WAIT_TIMEOUT_MS 500U
```

#### Static Variables

```c
static QueueHandle_t example_2_queue_drop_newest = NULL;
static QueueHandle_t example_2_queue_drop_oldest = NULL;
static QueueHandle_t example_2_queue_no_drop = NULL;
static QueueHandle_t example_2_queue_wait = NULL;
```

#### Main Function Flow

1. Initialize event bus
2. Register 4 channels with different configurations:
   - DROP_NEWEST: default config (no timeout needed)
   - DROP_OLDEST: custom config
   - NO_DROP: custom config with 1 second timeout
   - WAIT: custom config with 500ms timeout
3. Create 4 subscriber queues
4. Subscribe each queue to its respective channel
5. Create publisher task (posts to all channels rapidly)
6. Create 4 subscriber tasks (one per channel)

#### Publisher Task

- Posts events rapidly to all 4 channels
- Uses small delay to fill queues quickly
- Posts sequence numbers to track dropped events
- Logs when events are dropped (for DROP_NEWEST/OLDEST)
- Logs when timeout occurs (for NO_DROP/WAIT)

#### Subscriber Tasks

Each subscriber task:
- Receives events from its queue
- Logs received sequence numbers
- For DROP_NEWEST: Shows that new events are dropped when queue full
- For DROP_OLDEST: Shows that oldest events are overwritten
- For NO_DROP: Shows blocking behavior (may timeout)
- For WAIT: Shows timeout behavior

### Key Demonstration Points

1. **DROP_NEWEST**: When queue is full, new events are silently dropped, oldest events remain
2. **DROP_OLDEST**: When queue is full, oldest events are overwritten, newest events remain
3. **NO_DROP**: Blocks until space available or timeout, then returns error
4. **WAIT**: Blocks with timeout, returns error on timeout

### Expected Behavior

- Publisher posts events faster than subscribers can process
- DROP_NEWEST channel: Subscriber sees gaps in sequence numbers (dropped events)
- DROP_OLDEST channel: Subscriber sees continuous sequence numbers but may miss older ones
- NO_DROP channel: Publisher may timeout if subscriber is slow
- WAIT channel: Publisher may timeout, but waits up to 500ms

### Testing Strategy

1. Start with slow subscriber (demonstrates queue filling)
2. Show statistics after running for a period
3. Demonstrate recovery when subscriber speeds up
