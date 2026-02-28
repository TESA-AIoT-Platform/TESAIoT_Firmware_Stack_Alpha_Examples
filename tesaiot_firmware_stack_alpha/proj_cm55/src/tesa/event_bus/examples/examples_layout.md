# Event Bus Examples Layout Design

## Overview

This document defines the structure and objectives for all TESA Event Bus examples. The examples are organized to progressively demonstrate all features and use cases of the event bus system.

## Example Organization Strategy

Examples are organized by complexity and feature coverage:

- **Example 1**: Basic usage (already exists, may need refinement)
- **Example 2**: Queue policies and channel configuration
- **Example 3**: ISR context posting
- **Example 4**: Multiple subscribers and broadcasting
- **Example 5**: Statistics and monitoring
- **Example 6**: Error handling and edge cases
- **Example 7**: Advanced patterns (unsubscribe, unregister, timestamp handling)

## Detailed Example Specifications

### Example 1: Basic Usage (Current - May Need Refinement)

**File**: `tesa_event_bus_example_1.c` / `tesa_event_bus_example_1.h`

**Objectives**:
- Demonstrate basic initialization sequence
- Show channel registration with default configuration
- Demonstrate single publisher, single subscriber pattern
- Show event posting with and without payload
- Demonstrate event reception and processing
- Show proper event cleanup (free_event)

**Key Features Covered**:
- `tesa_event_bus_init()`
- `tesa_event_bus_register_channel()` (default config)
- `tesa_event_bus_subscribe()`
- `tesa_event_bus_post()` (task context)
- `tesa_event_bus_free_event()`
- Basic event structure handling

**Current Status**: Exists, may need refinement based on final layout

---

### Example 2: Queue Policies and Channel Configuration

**File**: `tesa_event_bus_example_2.c` / `tesa_event_bus_example_2.h`

**Objectives**:
- Demonstrate all four queue policies (DROP_NEWEST, DROP_OLDEST, NO_DROP, WAIT)
- Show channel registration with custom configuration
- Demonstrate behavior differences between policies
- Show when to use each policy

**Key Features Covered**:
- `tesa_event_bus_register_channel_with_config()`
- Queue policy: `TESA_EVENT_BUS_QUEUE_DROP_NEWEST`
- Queue policy: `TESA_EVENT_BUS_QUEUE_DROP_OLDEST`
- Queue policy: `TESA_EVENT_BUS_QUEUE_NO_DROP`
- Queue policy: `TESA_EVENT_BUS_QUEUE_WAIT`
- Custom timeout configuration
- Behavior under queue full conditions

**Scenario**: Multiple channels with different policies, showing how events are handled when queues fill up

---

### Example 3: ISR Context Posting

**File**: `tesa_event_bus_example_3.c` / `tesa_event_bus_example_3.h`

**Objectives**:
- Demonstrate posting events from ISR context
- Show proper ISR-safe API usage
- Demonstrate yield handling (`pxHigherPriorityTaskWoken`)
- Show timer ISR and GPIO ISR patterns
- Compare task vs ISR posting

**Key Features Covered**:
- `tesa_event_bus_post_from_isr()`
- `portYIELD_FROM_ISR()` usage
- ISR-safe timestamp handling
- Multiple ISR sources posting to same channel
- ISR to task communication pattern

**Scenario**: Simulated timer ISR and GPIO ISR posting sensor data and button events

---

### Example 4: Multiple Subscribers and Broadcasting

**File**: `tesa_event_bus_example_4.c` / `tesa_event_bus_example_4.h`

**Objectives**:
- Demonstrate one-to-many event broadcasting
- Show multiple subscribers on same channel
- Demonstrate independent event copies per subscriber
- Show different subscriber processing speeds
- Demonstrate partial success scenarios

**Key Features Covered**:
- Multiple `tesa_event_bus_subscribe()` calls on same channel
- Broadcasting to multiple subscribers
- Independent subscriber queues
- `TESA_EVENT_BUS_ERROR_PARTIAL_SUCCESS` handling
- Different subscriber task priorities

**Scenario**: One publisher broadcasting system status to multiple subscribers (display, logger, network)

---

### Example 5: Statistics and Monitoring

**File**: `tesa_event_bus_example_5.c` / `tesa_event_bus_example_5.h`

**Objectives**:
- Demonstrate subscriber statistics retrieval
- Show channel-level aggregate statistics
- Demonstrate health monitoring patterns
- Show drop rate calculation
- Demonstrate statistics-based decision making

**Key Features Covered**:
- `tesa_event_bus_get_subscriber_stats()`
- `tesa_event_bus_get_channel_stats()`
- Statistics structure fields (posts_successful, posts_dropped, last_drop_timestamp_ms)
- Health monitoring task pattern
- Drop rate analysis

**Scenario**: Health monitoring task periodically checking statistics and alerting on high drop rates

---

### Example 6: Error Handling and Edge Cases

**File**: `tesa_event_bus_example_6.c` / `tesa_event_bus_example_6.h`

**Objectives**:
- Demonstrate comprehensive error handling
- Show all error return codes and their meanings
- Demonstrate edge cases (memory exhaustion, invalid parameters)
- Show recovery strategies
- Demonstrate error logging patterns

**Key Features Covered**:
- All `tesa_event_bus_result_t` error codes
- `TESA_EVENT_BUS_ERROR_MEMORY` handling
- `TESA_EVENT_BUS_ERROR_QUEUE_FULL` handling
- `TESA_EVENT_BUS_ERROR_PAYLOAD_TOO_LARGE` handling
- `TESA_EVENT_BUS_ERROR_CHANNEL_NOT_FOUND` handling
- Invalid parameter handling
- Error recovery strategies

**Scenario**: Stress testing with high event rates, memory pressure, and error injection

---

### Example 7: Advanced Patterns

**File**: `tesa_event_bus_example_7.c` / `tesa_event_bus_example_7.h`

**Objectives**:
- Demonstrate unsubscribe operations
- Show channel unregistration
- Demonstrate timestamp handling and rollover detection
- Show dynamic subscription patterns
- Demonstrate channel lifecycle management

**Key Features Covered**:
- `tesa_event_bus_unsubscribe()`
- `tesa_event_bus_unregister_channel()`
- `tesa_event_bus_get_timestamp_ms()`
- `tesa_event_bus_check_timestamp_rollover()`
- Timestamp-based event age calculation
- Dynamic subscription management

**Scenario**: Dynamic system reconfiguration, subscriber management, and timestamp-based event filtering

---

## Example File Structure

Each example should follow this structure:

```
tesa_event_bus_example_N.h:
  - Channel ID definitions
  - Event type definitions
  - Payload structure definitions
  - Function declaration: void tesa_event_bus_example_N(void);

tesa_event_bus_example_N.c:
  - Usage comment block
  - Includes
  - Constants (queue lengths, stack sizes, priorities, delays)
  - Static variables (queues, task handles)
  - Static function declarations
  - Main example function
  - Publisher task function
  - Subscriber task function(s)
  - Helper functions
```

## Example Dependencies

- **Example 1**: No dependencies (basic)
- **Example 2**: Depends on Example 1 concepts
- **Example 3**: Depends on Example 1, introduces ISR concepts
- **Example 4**: Depends on Example 1, introduces multi-subscriber
- **Example 5**: Depends on Example 4 (needs multiple subscribers for meaningful stats)
- **Example 6**: Depends on Examples 1-3 (error scenarios)
- **Example 7**: Depends on all previous examples (advanced features)

## Integration with examples.h

The `examples.h` file should include all example headers:

```c
#ifndef TESA_EVENT_BUS_EXAMPLES_H
#define TESA_EVENT_BUS_EXAMPLES_H

#include "examples/tesa_event_bus_example_1.h"
#include "examples/tesa_event_bus_example_2.h"
#include "examples/tesa_event_bus_example_3.h"
#include "examples/tesa_event_bus_example_4.h"
#include "examples/tesa_event_bus_example_5.h"
#include "examples/tesa_event_bus_example_6.h"
#include "examples/tesa_event_bus_example_7.h"

void tesa_event_bus_example_1(void);
void tesa_event_bus_example_2(void);
void tesa_event_bus_example_3(void);
void tesa_event_bus_example_4(void);
void tesa_event_bus_example_5(void);
void tesa_event_bus_example_6(void);
void tesa_event_bus_example_7(void);

#endif
```

## Coverage Matrix

| Feature                       | Ex1   | Ex2   | Ex3   | Ex4   | Ex5 | Ex6 | Ex7 |
| ----------------------------- | ----- | ----- | ----- | ----- | --- | --- | --- |
| Basic init/register/subscribe | ✓     | ✓     | ✓     | ✓     | ✓   | ✓   | ✓   |
| Default channel config        | ✓     | -     | ✓     | ✓     | ✓   | ✓   | ✓   |
| Custom channel config         | -     | ✓     | -     | -     | -   | -   | -   |
| All queue policies            | -     | ✓     | -     | -     | -   | -   | -   |
| Task context posting          | ✓     | ✓     | -     | ✓     | ✓   | ✓   | ✓   |
| ISR context posting           | -     | -     | ✓     | -     | -   | -   | -   |
| Single subscriber             | ✓     | ✓     | ✓     | -     | -   | ✓   | ✓   |
| Multiple subscribers          | -     | -     | -     | ✓     | ✓   | -   | -   |
| Statistics                    | -     | -     | -     | -     | ✓   | -   | -   |
| Error handling                | Basic | Basic | Basic | Basic | -   | ✓   | -   |
| Unsubscribe/unregister        | -     | -     | -     | -     | -   | -   | ✓   |
| Timestamp handling            | -     | -     | -     | -     | -   | -   | ✓   |

## Best Practices for Examples

1. **Consistency**: All examples should follow the same structure and coding style
2. **Completeness**: Each example should be self-contained and runnable
3. **Documentation**: Each example should have clear usage comments
4. **MISRA C Compliance**: Follow Yoda conditions and other MISRA C patterns
5. **Error Handling**: Show proper error checking (even if simplified)
6. **Memory Safety**: Always demonstrate proper event freeing
7. **Realistic Scenarios**: Use realistic use cases (sensors, UI, network, etc.)

## Example Naming Convention

- Header files: `tesa_event_bus_example_N.h`
- Source files: `tesa_event_bus_example_N.c`
- Function names: `tesa_event_bus_example_N()`
- Channel IDs: `EXAMPLE_N_CHANNEL_*`
- Event types: `EXAMPLE_N_EVENT_*`
- Data types: `example_N_data_t`

## Future Considerations

- Example 8: Real-world integration (WiFi, Bluetooth, LVGL, Sensors) - could be separate integration examples
- Example 9: Performance benchmarking
- Example 10: Safety-critical patterns (watchdog integration, fault injection)

## Documentation Integration

The examples should complement the existing documentation:
- `event_bus_examples_and_usage.md` - references these examples
- Each example should be referenced in the main documentation
- Examples serve as executable documentation

## Summary

**Total Examples Needed**: 7 core examples

**Coverage**:
- All API functions covered
- All queue policies demonstrated
- Both task and ISR contexts
- Single and multiple subscriber patterns
- Statistics and monitoring
- Error handling
- Advanced lifecycle management

**Example 1 Status**: Exists, may need refinement to align with this layout

**Example 2 Status**: Empty, needs implementation

**Examples 3-7 Status**: Need to be created

This layout ensures comprehensive coverage of all event bus functionalities while maintaining a logical progression from basic to advanced usage patterns.
