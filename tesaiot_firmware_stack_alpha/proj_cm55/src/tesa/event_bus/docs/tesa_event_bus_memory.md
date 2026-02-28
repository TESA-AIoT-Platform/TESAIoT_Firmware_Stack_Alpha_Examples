# Event Bus Memory Ownership Documentation

## Overview

This document describes memory ownership, lifecycle, and cleanup requirements for the event bus system to ensure safe operation in medical device firmware.

## Memory Ownership Model

### Event Structures

**Ownership**: Event bus owns event structures until `tesa_event_bus_free_event()` is called.

**Lifecycle**:
1. Event is allocated from static pool when posting
2. Event is sent to subscriber queue (pointer copied)
3. Subscriber receives event from queue
4. Subscriber MUST call `tesa_event_bus_free_event()` after processing
5. Event structure and payload memory returned to pool

**Critical Requirement**: Subscribers MUST free events immediately after processing. Events MUST NOT be stored or used after processing.

### Payload Memory

**Ownership**: Event bus owns payload memory until `tesa_event_bus_free_event()` is called.

**Allocation**:
- All payloads use static buffer pools (no dynamic allocation)
- Maximum payload size: `TESA_EVENT_BUS_MAX_PAYLOAD_SIZE` (256 bytes)
- Payloads larger than maximum will cause posting to fail

**Memory Location**:
- Payloads are copied into event bus managed memory
- Original payload pointer provided by caller is NOT used after posting
- Subscriber receives a pointer to the copied payload

**Critical Requirement**: Subscribers MUST NOT free payload memory directly. Use `tesa_event_bus_free_event()` which handles all cleanup.

## Cleanup Requirements

### Required Cleanup

**After receiving event from queue**:
```c
tesa_event_t *event;
if (xQueueReceive(my_queue, &event, timeout) == pdTRUE) {
    // Process event
    process_event(event);
    
    // REQUIRED: Free event immediately after processing
    tesa_event_bus_free_event(event);
}
```

**Never do this**:
```c
// WRONG: Storing event pointer for later use
tesa_event_t *stored_event;
xQueueReceive(my_queue, &stored_event, timeout);
// ... later ...
process_event(stored_event);  // May use freed memory!

// WRONG: Manual payload freeing
free(event->payload);  // Never free payload directly!
free(event);           // Never free event structure directly!
```

### Automatic Cleanup

If an event fails to post to any subscriber:
- Event bus automatically frees the event and payload
- No subscriber action required

## Memory Limits

### Static Pools

- **Event Pool Size**: `TESA_EVENT_BUS_EVENT_POOL_SIZE` (32 events)
- **Payload Buffer Size**: `TESA_EVENT_BUS_PAYLOAD_BUFFER_SIZE` (256 bytes)
- **Payload Pool Size**: `TESA_EVENT_BUS_EVENT_POOL_SIZE` (32 buffers)

### Behavior on Exhaustion

**Event Pool Exhausted**:
- `tesa_event_bus_post()` returns `TESA_EVENT_BUS_ERROR_MEMORY`
- No event is created
- Original payload is not modified

**Payload Pool Exhausted**:
- `tesa_event_bus_post()` returns `TESA_EVENT_BUS_ERROR_MEMORY`
- Event structure is freed
- Original payload is not modified

**Payload Too Large**:
- `tesa_event_bus_post()` returns `TESA_EVENT_BUS_ERROR_PAYLOAD_TOO_LARGE`
- No memory allocation attempted
- Original payload is not modified

## Thread Safety

### Allocation/Deallocation

- Allocations are protected by critical sections
- `tesa_event_bus_free_event()` is thread-safe
- Can be called from any task context
- CANNOT be called from ISR context (use task context only)

### Memory Safety

- Event structures are copied for each subscriber
- Each subscriber gets independent event and payload copies
- Freeing one subscriber's event does not affect others

## Best Practices

1. **Always free immediately**: Free events as soon as processing completes
2. **Never store events**: Do not cache event pointers
3. **Use provided API**: Always use `tesa_event_bus_free_event()`
4. **Check return codes**: Handle `TESA_EVENT_BUS_ERROR_MEMORY` appropriately
5. **Respect payload limits**: Ensure payloads fit within maximum size

## Example: Proper Usage

```c
void my_subscriber_task(void *pvParameters) {
    QueueHandle_t my_queue = xQueueCreate(10, sizeof(tesa_event_t *));
    
    // Subscribe to channel
    tesa_event_bus_subscribe(MY_CHANNEL_ID, my_queue);
    
    for (;;) {
        tesa_event_t *event;
        
        // Receive event
        if (xQueueReceive(my_queue, &event, portMAX_DELAY) == pdTRUE) {
            // Process event
            if (event->payload != NULL) {
                my_process_payload(event->payload, event->payload_size);
            }
            
            // REQUIRED: Free event immediately
            tesa_event_bus_free_event(event);
        }
    }
}
```

## Medical Device Safety Considerations

- **Deterministic Behavior**: All memory allocation uses static pools (no heap)
- **Bounded Execution**: Allocation/deallocation is O(n) with fixed bounds
- **No Fragmentation**: Static pools eliminate heap fragmentation
- **Fail-Fast**: Memory exhaustion causes immediate, predictable failure
- **Audit Trail**: Statistics track successful/failed allocations

## Troubleshooting

**Memory Exhaustion**:
- Check that all subscribers are freeing events properly
- Monitor statistics for high drop rates
- Consider increasing pool sizes if needed

**Use-After-Free**:
- Ensure events are freed immediately after processing
- Never cache event pointers
- Use static analysis tools to detect violations
