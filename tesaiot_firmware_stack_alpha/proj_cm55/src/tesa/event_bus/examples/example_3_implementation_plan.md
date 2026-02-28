# Example 3 Implementation Plan: ISR Context Posting

## Overview

Example 3 demonstrates posting events from ISR (Interrupt Service Routine) context. This is critical for real-time systems where hardware interrupts need to communicate with tasks.

## Objectives

1. Demonstrate `tesa_event_bus_post_from_isr()` usage
2. Show proper yield handling with `pxHigherPriorityTaskWoken`
3. Demonstrate timer ISR pattern
4. Demonstrate GPIO/button ISR pattern
5. Compare ISR vs task posting

## Implementation Details

### Channel Configuration

Create channels for different ISR sources:

- **Channel 1**: Timer ISR events (periodic sensor sampling)
- **Channel 2**: GPIO ISR events (button presses)

### Header File Structure (`tesa_event_bus_example_3.h`)

```c
#define EXAMPLE_3_CHANNEL_TIMER  0x0301U
#define EXAMPLE_3_CHANNEL_GPIO   0x0302U

#define EXAMPLE_3_EVENT_TIMER_TICK    0x0301U
#define EXAMPLE_3_EVENT_BUTTON_PRESS 0x0302U
#define EXAMPLE_3_EVENT_BUTTON_RELEASE 0x0303U

typedef struct {
    uint32_t tick_count;
    uint32_t timestamp_ms;
} example_3_timer_data_t;

typedef struct {
    uint8_t button_id;
    uint32_t timestamp_ms;
} example_3_button_data_t;

void tesa_event_bus_example_3(void);
```

### Source File Structure (`tesa_event_bus_example_3.c`)

#### Constants

```c
#define EXAMPLE_3_QUEUE_LENGTH 10U
#define EXAMPLE_3_TASK_STACK_SIZE 1024U
#define EXAMPLE_3_TASK_PRIORITY 5U
#define EXAMPLE_3_TIMER_PERIOD_MS 100U
```

#### Static Variables

```c
static QueueHandle_t example_3_queue_timer = NULL;
static QueueHandle_t example_3_queue_gpio = NULL;
static uint32_t example_3_tick_counter = 0U;
static TimerHandle_t example_3_timer_handle = NULL;
```

#### Main Function Flow

1. Initialize event bus
2. Register timer and GPIO channels (default config)
3. Create subscriber queues
4. Subscribe queues to channels
5. Create timer (FreeRTOS software timer) that triggers ISR callback
6. Create subscriber tasks
7. Simulate GPIO interrupt (can use software timer or actual GPIO if available)

#### Timer ISR Callback

```c
static void example_3_timer_callback(TimerHandle_t xTimer) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    example_3_timer_data_t timer_data;
    
    example_3_tick_counter++;
    timer_data.tick_count = example_3_tick_counter;
    timer_data.timestamp_ms = tesa_event_bus_get_timestamp_ms();
    
    tesa_event_bus_post_from_isr(
        EXAMPLE_3_CHANNEL_TIMER,
        EXAMPLE_3_EVENT_TIMER_TICK,
        &timer_data,
        sizeof(example_3_timer_data_t),
        &xHigherPriorityTaskWoken
    );
    
    if (pdTRUE == xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
```

#### GPIO ISR Simulation

Since actual GPIO ISR may not be available, simulate with:
- Software timer that triggers at irregular intervals
- Or use a task that simulates button presses

#### Subscriber Tasks

- Timer subscriber: Processes periodic timer events
- GPIO subscriber: Processes button press/release events
- Both log events with timestamps

### Key Demonstration Points

1. **ISR Safety**: Using `_from_isr()` version is mandatory in ISR context
2. **Yield Handling**: Always check and yield if `pxHigherPriorityTaskWoken` is true
3. **Timestamp in ISR**: Use ISR-safe timestamp (handled internally)
4. **No Blocking**: ISR posting never blocks (uses DROP_NEWEST by default or configured policy)

### Expected Behavior

- Timer ISR fires periodically, posts events
- GPIO ISR fires on button events, posts events
- Subscriber tasks receive and process events
- System remains responsive (ISR is non-blocking)

### Important Notes

- Never use `tesa_event_bus_post()` in ISR context
- Always use `tesa_event_bus_post_from_isr()` in ISR context
- Always check `pxHigherPriorityTaskWoken` and yield if needed
- Keep ISR processing minimal (read data, post event, exit)
