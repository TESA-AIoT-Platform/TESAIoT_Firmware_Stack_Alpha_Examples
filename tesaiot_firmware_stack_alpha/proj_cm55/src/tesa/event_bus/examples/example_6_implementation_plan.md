# Example 6 Implementation Plan: Error Handling and Edge Cases

## Overview

Example 6 demonstrates comprehensive error handling for all possible error conditions and edge cases in the event bus system.

## Objectives

1. Demonstrate all error return codes
2. Show proper error handling patterns
3. Demonstrate edge cases (memory exhaustion, invalid parameters)
4. Show recovery strategies
5. Illustrate error logging patterns

## Implementation Details

### Channel Configuration

Create channels for testing different error scenarios:

- **Channel 1**: Normal operation (for comparison)
- **Channel 2**: Stress testing (memory pressure)

### Header File Structure (`tesa_event_bus_example_6.h`)

```c
#define EXAMPLE_6_CHANNEL_NORMAL  0x0601U
#define EXAMPLE_6_CHANNEL_STRESS  0x0602U

#define EXAMPLE_6_EVENT_TEST  0x0601U

typedef struct {
    uint32_t sequence;
    uint8_t data[200];
} example_6_test_data_t;

void tesa_event_bus_example_6(void);
```

### Source File Structure (`tesa_event_bus_example_6.c`)

#### Constants

```c
#define EXAMPLE_6_QUEUE_LENGTH 3U
#define EXAMPLE_6_TASK_STACK_SIZE 1024U
#define EXAMPLE_6_TASK_PRIORITY 5U
```

#### Error Test Functions

**1. Test Invalid Parameters**:
```c
static void test_invalid_parameters(void) {
    tesa_event_bus_result_t result;
    
    TESA_LOG_INFO("Testing invalid parameters...");
    
    result = tesa_event_bus_register_channel(0U, "Invalid");
    if (TESA_EVENT_BUS_ERROR_INVALID_PARAM == result) {
        TESA_LOG_INFO("✓ Correctly rejected channel ID 0");
    }
    
    result = tesa_event_bus_register_channel(0x0601U, NULL);
    if (TESA_EVENT_BUS_ERROR_INVALID_PARAM == result) {
        TESA_LOG_INFO("✓ Correctly rejected NULL channel name");
    }
    
    result = tesa_event_bus_subscribe(0x0601U, NULL);
    if (TESA_EVENT_BUS_ERROR_INVALID_PARAM == result) {
        TESA_LOG_INFO("✓ Correctly rejected NULL queue handle");
    }
    
    result = tesa_event_bus_post(0U, 0x0601U, NULL, 0U);
    if (TESA_EVENT_BUS_ERROR_INVALID_PARAM == result) {
        TESA_LOG_INFO("✓ Correctly rejected invalid channel ID in post");
    }
}
```

**2. Test Channel Not Found**:
```c
static void test_channel_not_found(void) {
    tesa_event_bus_result_t result;
    QueueHandle_t queue = xQueueCreate(5, sizeof(tesa_event_t *));
    
    TESA_LOG_INFO("Testing channel not found...");
    
    result = tesa_event_bus_subscribe(0x9999U, queue);
    if (TESA_EVENT_BUS_ERROR_CHANNEL_NOT_FOUND == result) {
        TESA_LOG_INFO("✓ Correctly rejected subscription to non-existent channel");
    }
    
    result = tesa_event_bus_post(0x9999U, 0x0601U, NULL, 0U);
    if (TESA_EVENT_BUS_ERROR_CHANNEL_NOT_FOUND == result) {
        TESA_LOG_INFO("✓ Correctly rejected post to non-existent channel");
    }
    
    vQueueDelete(queue);
}
```

**3. Test Payload Too Large**:
```c
static void test_payload_too_large(void) {
    tesa_event_bus_result_t result;
    uint8_t large_payload[300];
    
    TESA_LOG_INFO("Testing payload too large...");
    
    result = tesa_event_bus_post(EXAMPLE_6_CHANNEL_NORMAL, 
                                 EXAMPLE_6_EVENT_TEST,
                                 large_payload,
                                 sizeof(large_payload));
    if (TESA_EVENT_BUS_ERROR_PAYLOAD_TOO_LARGE == result) {
        TESA_LOG_INFO("✓ Correctly rejected payload > 256 bytes");
    }
}
```

**4. Test Memory Exhaustion**:
```c
static void test_memory_exhaustion(void) {
    tesa_event_bus_result_t result;
    example_6_test_data_t data;
    uint32_t post_count = 0U;
    uint32_t success_count = 0U;
    uint32_t memory_error_count = 0U;
    
    TESA_LOG_INFO("Testing memory exhaustion...");
    TESA_LOG_INFO("Posting events rapidly to exhaust event pool...");
    
    for (uint32_t i = 0U; i < 100U; i++) {
        data.sequence = i;
        result = tesa_event_bus_post(EXAMPLE_6_CHANNEL_STRESS,
                                    EXAMPLE_6_EVENT_TEST,
                                    &data,
                                    sizeof(data));
        post_count++;
        
        if (TESA_EVENT_BUS_SUCCESS == result) {
            success_count++;
        } else if (TESA_EVENT_BUS_ERROR_MEMORY == result) {
            memory_error_count++;
            TESA_LOG_WARNING("Memory error at post #%lu", (unsigned long)i);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10U));
    }
    
    TESA_LOG_INFO("Memory test results: Posts=%lu, Success=%lu, MemoryErrors=%lu",
                 (unsigned long)post_count,
                 (unsigned long)success_count,
                 (unsigned long)memory_error_count);
}
```

**5. Test Queue Full (NO_DROP policy)**:
```c
static void test_queue_full_no_drop(void) {
    tesa_event_bus_result_t result;
    tesa_event_bus_channel_config_t config = {
        .queue_policy = TESA_EVENT_BUS_QUEUE_NO_DROP,
        .queue_timeout_ticks = pdMS_TO_TICKS(1000U)
    };
    
    TESA_LOG_INFO("Testing queue full with NO_DROP policy...");
    
    result = tesa_event_bus_register_channel_with_config(
        0x0603U, "NoDropTest", &config);
    
    QueueHandle_t queue = xQueueCreate(2, sizeof(tesa_event_t *));
    tesa_event_bus_subscribe(0x0603U, queue);
    
    example_6_test_data_t data;
    for (uint32_t i = 0U; i < 10U; i++) {
        data.sequence = i;
        result = tesa_event_bus_post(0x0603U, EXAMPLE_6_EVENT_TEST,
                                    &data, sizeof(data));
        
        if (TESA_EVENT_BUS_ERROR_QUEUE_FULL == result) {
            TESA_LOG_WARNING("Queue full detected at post #%lu", (unsigned long)i);
        }
    }
    
    vQueueDelete(queue);
}
```

**6. Test Partial Success**:
```c
static void test_partial_success(void) {
    tesa_event_bus_result_t result;
    example_6_test_data_t data;
    
    TESA_LOG_INFO("Testing partial success scenario...");
    TESA_LOG_INFO("Creating subscribers with different queue sizes...");
    
    QueueHandle_t queue1 = xQueueCreate(10, sizeof(tesa_event_t *));
    QueueHandle_t queue2 = xQueueCreate(2, sizeof(tesa_event_t *));
    
    tesa_event_bus_subscribe(EXAMPLE_6_CHANNEL_NORMAL, queue1);
    tesa_event_bus_subscribe(EXAMPLE_6_CHANNEL_NORMAL, queue2);
    
    for (uint32_t i = 0U; i < 5U; i++) {
        data.sequence = i;
        result = tesa_event_bus_post(EXAMPLE_6_CHANNEL_NORMAL,
                                    EXAMPLE_6_EVENT_TEST,
                                    &data,
                                    sizeof(data));
        
        if (TESA_EVENT_BUS_ERROR_PARTIAL_SUCCESS == result) {
            TESA_LOG_WARNING("Partial success at post #%lu", (unsigned long)i);
        }
    }
    
    vQueueDelete(queue1);
    vQueueDelete(queue2);
}
```

**7. Test Channel Full**:
```c
static void test_channel_full(void) {
    tesa_event_bus_result_t result;
    
    TESA_LOG_INFO("Testing channel registry full...");
    TESA_LOG_INFO("Registering channels until registry is full...");
    
    for (uint16_t i = 0x1000U; i < 0x2000U; i++) {
        char name[16];
        (void)snprintf(name, sizeof(name), "Chan%04X", i);
        
        result = tesa_event_bus_register_channel(i, name);
        
        if (TESA_EVENT_BUS_ERROR_CHANNEL_FULL == result) {
            TESA_LOG_INFO("✓ Channel registry full at channel 0x%04X", i);
            break;
        }
    }
}
```

**8. Test Subscriber Full**:
```c
static void test_subscriber_full(void) {
    tesa_event_bus_result_t result;
    
    TESA_LOG_INFO("Testing subscriber limit per channel...");
    
    tesa_event_bus_register_channel(0x0604U, "SubscriberTest");
    
    for (uint8_t i = 0U; i < 10U; i++) {
        QueueHandle_t queue = xQueueCreate(5, sizeof(tesa_event_t *));
        result = tesa_event_bus_subscribe(0x0604U, queue);
        
        if (TESA_EVENT_BUS_ERROR_SUBSCRIBER_FULL == result) {
            TESA_LOG_INFO("✓ Subscriber limit reached at subscriber %u", i);
            vQueueDelete(queue);
            break;
        }
    }
}
```

**9. Test Invalid Queue**:
```c
static void test_invalid_queue(void) {
    tesa_event_bus_result_t result;
    QueueHandle_t invalid_queue = (QueueHandle_t)0xDEADBEEF;
    
    TESA_LOG_INFO("Testing invalid queue handle...");
    
    result = tesa_event_bus_get_subscriber_stats(
        EXAMPLE_6_CHANNEL_NORMAL,
        invalid_queue,
        NULL);
    
    if (TESA_EVENT_BUS_ERROR_INVALID_PARAM == result) {
        TESA_LOG_INFO("✓ Correctly rejected invalid queue handle");
    }
}
```

### Main Function Flow

1. Initialize event bus
2. Register test channels
3. Run all error test functions sequentially
4. Log results
5. Demonstrate recovery strategies

### Key Demonstration Points

1. **All Error Codes**: Every possible error condition tested
2. **Error Recovery**: Strategies for handling each error
3. **Edge Cases**: Memory exhaustion, queue full, etc.
4. **Error Logging**: Proper logging of errors
5. **Graceful Degradation**: System continues operating despite errors

### Expected Behavior

- Each error test logs its results
- System continues operating after errors
- Recovery strategies demonstrated
- Comprehensive error coverage

### Error Codes Covered

- `TESA_EVENT_BUS_ERROR_INVALID_PARAM`
- `TESA_EVENT_BUS_ERROR_CHANNEL_NOT_FOUND`
- `TESA_EVENT_BUS_ERROR_CHANNEL_FULL`
- `TESA_EVENT_BUS_ERROR_SUBSCRIBER_FULL`
- `TESA_EVENT_BUS_ERROR_MEMORY`
- `TESA_EVENT_BUS_ERROR_QUEUE_FULL`
- `TESA_EVENT_BUS_ERROR_PAYLOAD_TOO_LARGE`
- `TESA_EVENT_BUS_ERROR_PARTIAL_SUCCESS`
- `TESA_EVENT_BUS_ERROR_INVALID_QUEUE`
