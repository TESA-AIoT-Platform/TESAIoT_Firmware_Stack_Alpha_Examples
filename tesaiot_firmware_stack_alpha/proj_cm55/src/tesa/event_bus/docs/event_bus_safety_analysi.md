# Medical Device Firmware Standards - Event Bus Safety Analysis

## Overview

This document identifies critical safety concerns and compliance issues for using the event bus in medical device firmware that must comply with IEC 62304, ISO 14971, and related medical device software standards.

## Document Status

**Last Updated**: Reflects current implementation as of latest code review  
**Implementation Status**: 6/10 critical concerns resolved, 4/10 require external integration or documentation  
**Code Base**: `proj_cm55/src/tesa/event_bus/tesa_event_bus.c` and `tesa_event_bus.h`

## Quick Status Summary

- ✅ **6 Critical Items Resolved**: Memory safety, statistics protection, deterministic allocation, input validation, partial success reporting, stack safety
- ⚠️ **4 Items Requiring Action**: Timestamp rollover handling (app-level), queue validation discipline (app-level), watchdog integration (external), MISRA analysis (pending)

## Critical Safety Concerns

### 1. Memory Safety & Use-After-Free Risk

**Status**: ✅ **RESOLVED**

**Original Issue**: `free_payload()` may free memory that's still referenced by events in queues.

**Location**: Lines 681-692 in `tesa_event_bus.c`

**Original Problem**: 
- When subscribers receive events, they may hold pointers to payloads
- If they don't call `tesa_event_bus_free_event()` immediately, and the payload was dynamically allocated, freeing can cause use-after-free
- This violates memory safety requirements for medical devices

**Implementation**:
- ✅ **Dynamic allocation eliminated**: All `pvPortMalloc()` and `vPortFree()` calls removed (lines 643-692)
- ✅ **Static pools only**: All payloads use static buffer pools with bounded allocation
- ✅ **Memory ownership documented**: `tesa_event_bus_memory.md` created with lifecycle documentation
- ✅ **Cleanup helper provided**: `tesa_event_bus_free_event()` function available (lines 513-520)
- ✅ **Deterministic behavior**: Allocation always succeeds or fails immediately, no heap fragmentation concerns

**Residual Risk**: Low - Subscribers must still call `tesa_event_bus_free_event()` immediately after processing, but memory cannot be freed incorrectly since no dynamic allocation exists.

### 2. Race Condition in Statistics Update

**Status**: ✅ **RESOLVED**

**Original Issue**: Statistics counters can overflow without protection.

**Location**: Lines 597-616 in `tesa_event_bus.c`

**Original Problem**:
- `posts_successful++` and `posts_dropped++` are NOT atomic operations
- Concurrent updates from multiple tasks/ISRs can cause lost increments or corruption
- No overflow protection for `uint32_t` counters

**Implementation**:
- ✅ **Critical sections**: All statistics updates protected by critical sections (lines 345-346, 353-354, 454-455, 482-483, 489-490)
- ✅ **Overflow protection**: Counters saturate at `TESA_EVENT_BUS_STATS_MAX_VALUE` (UINT32_MAX) instead of wrapping (lines 605-612)
- ✅ **ISR-safe updates**: `update_subscriber_stats_with_time()` used in ISR context with proper critical section macros
- ✅ **Constant defined**: `TESA_EVENT_BUS_STATS_MAX_VALUE` defined in header

**Residual Risk**: Low - Statistics are protected and overflow is handled gracefully. Note: Critical sections protect against race conditions but add small latency. For very high-frequency updates, consider atomic operations if available.

### 3. Non-Deterministic Failure Modes

**Status**: ✅ **RESOLVED**

**Original Issue**: `pvPortMalloc()` failure handling is non-deterministic.

**Location**: Lines 643-692 in `tesa_event_bus.c`

**Original Problem**:
- Malloc failures depend on heap fragmentation state
- Non-deterministic behavior violates requirements for safety-critical systems (IEC 62304 Class C)
- Dynamic allocation is discouraged in real-time embedded systems

**Implementation**:
- ✅ **Dynamic allocation removed**: All `pvPortMalloc()` and `vPortFree()` calls eliminated
- ✅ **Static pools only**: `allocate_payload()` and `allocate_payload_from_isr()` use only static buffers (lines 643-692)
- ✅ **Bounded allocation**: Maximum payload size is compile-time constant (`TESA_EVENT_BUS_MAX_PAYLOAD_SIZE`)
- ✅ **Fail-fast behavior**: Allocation fails immediately if static pools exhausted (predictable, deterministic)
- ✅ **ISR-safe**: `allocate_payload_from_isr()` never uses malloc, fails if payload too large or pools exhausted

**Residual Risk**: None - All allocation is deterministic with bounded execution time.

### 4. Missing Input Validation

**Status**: ✅ **RESOLVED**

**Original Issues**:
- No validation of `payload_size` bounds (could cause buffer overflows)
- No validation that `channel_name` pointer remains valid during use
- No validation of queue handle validity after subscription

**Location**: Multiple functions

**Implementation**:
- ✅ **Payload size validation**: Bounds checking added in `tesa_event_bus_post()` (lines 238-244) and `tesa_event_bus_post_from_isr()` (lines 385-392)
- ✅ **NULL pointer validation**: Payload NULL check when `payload_size > 0` (lines 242-244)
- ✅ **Constant defined**: `TESA_EVENT_BUS_MAX_PAYLOAD_SIZE` defined in header (line 17-18)
- ✅ **Error code**: `TESA_EVENT_BUS_ERROR_PAYLOAD_TOO_LARGE` added for explicit error reporting
- ✅ **Queue handle validation**: NULL checks before queue operations (lines 310-319, 451-460)
- ⚠️ **Channel name validation**: Pointer validity cannot be validated at runtime (documented requirement: must remain valid)

**Residual Risk**: Low - All critical inputs validated. Channel name pointer lifetime is application responsibility (documented).

### 5. Timestamp Accuracy & Rollover

**Status**: ⚠️ **PARTIALLY RESOLVED**

**Original Issue**: `uint32_t` timestamp will rollover after ~49 days.

**Location**: Lines 704-717 in `tesa_event_bus.c`

**Original Problem**:
- Rollover can cause incorrect timestamp comparisons
- Medical devices often require continuous operation >49 days
- Event ordering could be incorrect after rollover

**Implementation**:
- ✅ **Rollover constant**: `TESA_EVENT_BUS_TIMESTAMP_ROLLOVER_MS` defined (2^32 - 1 ms = ~49.7 days)
- ✅ **Rollover detection function**: `tesa_event_bus_check_timestamp_rollover()` implemented (lines 712-717)
- ✅ **Documentation**: Rollover behavior documented in this analysis
- ⚠️ **32-bit timestamps**: Still using `uint32_t` timestamps (rollover detection available but not automatic)
- ⚠️ **64-bit timestamps**: Not implemented (would require structure change)

**Residual Risk**: Medium - Rollover detection function provided but rollover handling must be implemented at application level. For devices requiring >49 days continuous operation, consider:
- Using 64-bit timestamps if available
- Implementing application-level rollover handling using `tesa_event_bus_check_timestamp_rollover()`
- Using external monotonic time source that doesn't roll over

### 6. Queue Handle Invalidation Risk

**Status**: ⚠️ **PARTIALLY RESOLVED**

**Original Issue**: No protection against queue deletion while events are being posted.

**Location**: Lines 310-319, 451-460 in `tesa_event_bus.c`

**Original Problem**:
- If a queue is deleted (by FreeRTOS) between subscription and posting, `xQueueSend()` will access invalid memory
- No validation that queue handle is still valid before use

**Implementation**:
- ✅ **NULL checks**: Queue handle NULL validation added before use (lines 310-319, 451-460)
- ✅ **Error handling**: Invalid queue handles tracked in statistics
- ✅ **Documentation**: Requirement documented that queues MUST NOT be deleted while subscribed
- ⚠️ **Deep validation**: FreeRTOS doesn't provide API to validate queue handle validity after creation
- ⚠️ **Reference counting**: Not implemented (requires application-level coordination)

**Residual Risk**: Medium - NULL checks prevent crashes from NULL handles, but cannot detect if queue was deleted externally. This requires application-level discipline:
- Queues must NOT be deleted while subscribed
- Unsubscribe before deleting queues
- Consider reference counting at application level if dynamic queue management is needed

### 7. Partial Success Ambiguity

**Status**: ✅ **RESOLVED**

**Original Issue**: Event posting can partially succeed (some subscribers get event, others don't).

**Location**: Lines 305-371 in `tesa_event_bus.c`

**Original Problem**:
- For critical events (e.g., alarm conditions), partial delivery may be unacceptable
- API doesn't distinguish between "all failed" vs "some succeeded, some failed"
- Error handling cannot determine which subscribers received event

**Implementation**:
- ✅ **Error code added**: `TESA_EVENT_BUS_ERROR_PARTIAL_SUCCESS` distinguishes partial from complete failure (line 366)
- ✅ **Logic updated**: Post functions track `all_queues_ok` and `any_queues_ok` to determine return code (lines 305-369)
- ✅ **Return values**:
  - `TESA_EVENT_BUS_SUCCESS`: All subscribers received event
  - `TESA_EVENT_BUS_ERROR_PARTIAL_SUCCESS`: Some subscribers received, some didn't
  - `TESA_EVENT_BUS_ERROR_QUEUE_FULL`: No subscribers received event
- ✅ **Statistics tracking**: Per-subscriber success/failure tracking already available via statistics functions

**Residual Risk**: Low - Application can now determine delivery status. For critical channels requiring guaranteed delivery, use `TESA_EVENT_BUS_QUEUE_NO_DROP` or `TESA_EVENT_BUS_QUEUE_WAIT` policy and handle `ERROR_PARTIAL_SUCCESS` appropriately.

### 8. Stack Overflow Potential

**Status**: ✅ **RESOLVED**

**Original Issue**: Static buffers could be large if constants are increased.

**Location**: Lines 15-17 in `tesa_event_bus.c`

**Current Status**: ✅ **Safe (static allocation in global scope)**

**Implementation**:
- ✅ **Global static arrays**: `post_events_buffer` and `post_payloads_buffer` are static globals, not stack variables (lines 15-17)
- ✅ **No stack growth**: Arrays allocated at global scope, constant memory usage regardless of function calls
- ✅ **Current configuration**: Safe with current limits (8 subscribers max)

**Risk Assessment**:
- **Current**: No stack risk - arrays are global static
- **Future**: If `TESA_EVENT_BUS_MAX_SUBSCRIBERS_PER_CHANNEL` increased significantly, only global memory increases (not stack)

**Residual Risk**: None - Current implementation uses global static arrays. Stack usage is constant regardless of subscriber count.

### 9. Missing Watchdog/Stuck Detection

**Status**: ⚠️ **REQUIRES EXTERNAL INTEGRATION**

**Original Issue**: No protection against tasks blocking indefinitely in WAIT/NO_DROP policies.

**Location**: Lines 330-336 in `tesa_event_bus.c`

**Original Problem**:
- If subscriber queue never has space, posting task blocks until timeout
- For safety-critical systems, this may require watchdog monitoring

**Implementation**:
- ✅ **Bounded blocking time**: Maximum blocking = `channel->config.queue_timeout_ticks` (configurable per channel)
- ✅ **Default timeout**: `TESA_EVENT_BUS_DEFAULT_QUEUE_TIMEOUT_MS` (100ms) provides reasonable default
- ⚠️ **Watchdog integration**: Not implemented in event bus (requires application-level integration)
- ⚠️ **Timeout monitoring**: No automatic monitoring or alerts

**Documentation**:
- Maximum blocking time is bounded by configured timeout
- For `TESA_EVENT_BUS_QUEUE_DROP_NEWEST`: No blocking (0 timeout)
- For `TESA_EVENT_BUS_QUEUE_NO_DROP` and `TESA_EVENT_BUS_QUEUE_WAIT`: Blocks up to timeout value

**Residual Risk**: Medium - Blocking time is bounded and configurable, but requires:
- Application-level watchdog monitoring for safety-critical systems
- Appropriate timeout values configured per channel based on safety requirements
- Monitoring of drop statistics to detect stuck queues

### 10. MISRA C Violations

**Status**: ⚠️ **REQUIRES STATIC ANALYSIS**

**Issues**:
- Use of `void*` for payloads (MISRA C Rule 11.1-11.5: no casts from void pointer)
- Missing const correctness in some places
- Function-like macros (`taskENTER_CRITICAL()`) may violate MISRA C Rule 19.7

**Current Implementation**:
- `void*` payload: Used for generic payload support (line 50 in header)
- FreeRTOS macros: `taskENTER_CRITICAL()` and `taskENTER_CRITICAL_FROM_ISR()` are required for FreeRTOS integration
- Const correctness: Some function parameters could be const-qualified

**Recommendation**:
- ⚠️ **Run static analysis**: Use PC-lint or similar with MISRA rules enabled
- ⚠️ **Document deviations**: Create MISRA deviation log with justifications:
  - `void*` payload: Justified for generic event system supporting various payload types
  - FreeRTOS macros: Required for FreeRTOS API compatibility
- ⚠️ **Consider alternatives**: Type-safe wrapper structures could replace `void*` but would reduce flexibility
- ⚠️ **Const correctness**: Review and add `const` qualifiers where appropriate

**Residual Risk**: Low - Most violations are justified by design requirements, but formal analysis and deviation documentation is required for compliance.

## Compliance Concerns

### IEC 62304 Requirements

1. **Software Safety Classification**: 
   - Determine classification (A/B/C) for event bus component
   - Class C (highest) requires most stringent controls
   - Classify each channel based on safety impact

2. **Verification & Validation**:
   - Requires comprehensive unit testing (target >90% code coverage)
   - Integration testing with all subscriber types
   - Stress testing for memory exhaustion scenarios
   - Worst-case execution time (WCET) analysis

3. **Software Risk Management**:
   - Each identified risk above needs mitigation plan
   - Risk control measures must be implemented and verified
   - Residual risk must be documented

4. **Traceability**:
   - Document requirements → design → code mapping
   - All design decisions must be traceable to requirements
   - Changes must be tracked with impact analysis

### ISO 14971 (Risk Management)

1. **Hazard Analysis**:
   - Dropped events could cause patient harm (e.g., missed alarm)
   - Lost events could cause incorrect therapy delivery
   - Delayed events could cause timeout issues

2. **Risk Controls**:
   - Implement monitoring/alerts for dropped events
   - Set thresholds for acceptable drop rates
   - Implement graceful degradation modes

3. **Residual Risk**:
   - Document acceptable drop rates per channel type
   - Justify why partial delivery is acceptable (if applicable)
   - Implement fail-safe modes for critical channels

## Recommendations for Medical Device Use

### ✅ Completed Fixes

1. **✅ Remove Dynamic Allocation**: **COMPLETED**
   - ✅ `pvPortMalloc()` usage entirely eliminated
   - ✅ Only static pools with bounded allocation
   - ✅ Fail fast if static pools exhausted

2. **✅ Add Atomic Statistics**: **COMPLETED**
   - ✅ All counter updates protected by critical sections
   - ✅ Overflow detection and saturation implemented
   - ✅ Critical sections used for all statistics updates

3. **✅ Validate All Inputs**: **COMPLETED**
   - ✅ Bounds checking for `payload_size` added
   - ✅ All pointer parameters validated
   - ✅ NULL pointer checks implemented

4. **✅ Document Memory Ownership**: **COMPLETED**
   - ✅ Clear contract: Memory ownership documented in `tesa_event_bus_memory.md`
   - ✅ Cleanup helper: `tesa_event_bus_free_event()` provided
   - ✅ Lifecycle: Event lifecycle documented with examples

### ⚠️ Remaining Actions Required

5. **⚠️ Add Static Analysis**: **PENDING**
   - [ ] Run PC-lint or similar with MISRA rules
   - [ ] Address all high-severity warnings
   - [ ] Document and justify unavoidable violations
   - **Priority**: High (required for compliance)

6. **⚠️ Implement Comprehensive Testing**: **PENDING**
   - [ ] Unit tests with >90% code coverage
   - [ ] Boundary value testing
   - [ ] Stress testing (memory exhaustion, queue full scenarios)
   - [ ] Concurrency testing (race conditions)
   - **Priority**: Critical (required for IEC 62304 compliance)

### Architecture Improvements

1. **Safety Classification**:
   - Classify each channel by safety level
   - Apply different policies based on classification
   - Critical channels: NO_DROP or guaranteed delivery

2. **Event Priority**:
   - Add priority levels for critical vs non-critical events
   - Priority inversion protection
   - Preemptive delivery of critical events

3. **Watchdog Integration**:
   - Monitor event bus health
   - Alert on excessive drop rates
   - Implement recovery procedures

4. **Audit Logging**:
   - Log all dropped events for post-incident analysis
   - Log queue full conditions
   - Timestamp all operations for forensics

5. **Graceful Degradation**:
   - Define behavior when resources exhausted
   - Implement fail-safe modes for critical channels
   - Provide degraded operation modes

6. **Deterministic Behavior**:
   - Remove all non-deterministic operations
   - Bounded execution times for all functions
   - Worst-case analysis for all paths

## Testing Requirements

### Unit Testing

- [ ] Test all error paths (>90% coverage)
- [ ] Test memory allocation failures
- [ ] Test queue full scenarios
- [ ] Test concurrent operations
- [ ] Test ISR vs task context interactions
- [ ] Test statistics accuracy
- [ ] Test timestamp rollover scenarios

### Integration Testing

- [ ] Test with multiple subscribers
- [ ] Test channel registration/unregistration
- [ ] Test subscription/unsubscription
- [ ] Test all queue policies
- [ ] Test timeout scenarios
- [ ] Test memory exhaustion

### Stress Testing

- [ ] Maximum channel count
- [ ] Maximum subscribers per channel
- [ ] Maximum event rate
- [ ] Maximum payload size
- [ ] Continuous operation >49 days

### Safety Testing

- [ ] Worst-case execution time analysis
- [ ] Stack usage analysis
- [ ] Memory usage analysis
- [ ] Concurrency stress testing
- [ ] Failure mode analysis

## Documentation Requirements

- [ ] Software requirements specification (SRS)
- [ ] Software design specification (SDS)
- [ ] Interface control document (ICD)
- [ ] Software verification plan (SVP)
- [ ] Software verification report (SVR)
- [ ] Risk management file updates
- [ ] Traceability matrix (requirements → code)
- [ ] API documentation with usage examples
- [ ] Memory ownership documentation
- [ ] Performance characteristics document

## Risk Summary

| Risk ID                      | Severity | Likelihood | Mitigation Priority | Status     |
| ---------------------------- | -------- | ---------- | ------------------- | ---------- |
| Memory Use-After-Free        | High     | Medium     | Critical            | ✅ Resolved |
| Statistics Race Condition    | Medium   | High       | High                | ✅ Resolved |
| Non-Deterministic Allocation | High     | Low        | High                | ✅ Resolved |
| Missing Input Validation     | High     | Medium     | Critical            | ✅ Resolved |
| Timestamp Rollover           | Medium   | Low        | Medium              | ⚠️ Partial  |
| Queue Handle Invalidation    | High     | Low        | High                | ⚠️ Partial  |
| Partial Success Ambiguity    | Medium   | High       | Medium              | ✅ Resolved |
| Stack Overflow               | Low      | Low        | Low                 | ✅ Resolved |
| Missing Watchdog             | Medium   | Low        | Medium              | ⚠️ External |
| MISRA Violations             | Low      | N/A        | Low                 | ⚠️ Analysis |

**Status Legend**:
- ✅ Resolved: Implementation completed, risk mitigated
- ⚠️ Partial: Partial mitigation implemented, some residual risk
- ⚠️ External: Requires application-level or external integration
- ⚠️ Analysis: Requires static analysis and deviation documentation

## Implementation Summary

### Completed Improvements (6/10 Critical Concerns)

1. ✅ **Memory Safety**: Dynamic allocation eliminated, static pools only
2. ✅ **Statistics Protection**: Critical sections and overflow saturation
3. ✅ **Deterministic Behavior**: All allocation is static and bounded
4. ✅ **Input Validation**: Comprehensive bounds and NULL checks
5. ✅ **Partial Success Reporting**: Enhanced error codes and tracking
6. ✅ **Stack Safety**: Global static arrays prevent stack issues

### Partial/External Items (4/10 Concerns)

7. ⚠️ **Timestamp Rollover**: Detection function provided, application-level handling needed
8. ⚠️ **Queue Validation**: NULL checks added, deep validation requires application discipline
9. ⚠️ **Watchdog Integration**: Bounded blocking time, requires external watchdog
10. ⚠️ **MISRA Compliance**: Requires static analysis and deviation documentation

### Remaining Work

**High Priority**:
- Static analysis (MISRA compliance check)
- Comprehensive testing suite (>90% coverage)
- Documentation (SRS, SDS, ICD, SVP, SVR)

**Medium Priority**:
- Watchdog integration (application-level)
- Timestamp rollover handling (application-level)
- Performance analysis (WCET, stack usage)

**Low Priority**:
- Enhanced API documentation comments
- Const correctness review

## Notes

- All critical safety fixes have been implemented
- Code is ready for static analysis and testing
- Remaining work focuses on verification, documentation, and external integration
- Validation evidence must be maintained for regulatory submission
- Trade-offs must be documented in risk management file
