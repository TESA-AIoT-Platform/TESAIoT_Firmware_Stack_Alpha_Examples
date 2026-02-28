# TESAIoT Library

**Hardware-Secured IoT Device Provisioning for PSoC Edge + OPTIGA Trust M**

Version: 2.8.0
Last Updated: 2026-02-02

## Overview

TESAIoT Library provides secure device provisioning and certificate management for IoT devices using Infineon PSoC Edge and OPTIGA Trust M secure element.

### Features

- **License Key System** - ECDSA signature-based device licensing
- **CSR Workflow** - Generate Certificate Signing Requests using OPTIGA Trust M
- **MQTT with mTLS** - Secure MQTT communication with mutual TLS authentication
- **OPTIGA Integration** - Full integration with OPTIGA Trust M secure element
- **Protected Update** - Secure firmware/certificate update workflow
- **SNTP Client** - Time synchronization for certificate validation

---

## Header Structure (v2.8.0)

The library uses a consolidated 8-header structure:

```ini
tesaiot/include/
├── tesaiot.h                  # Main umbrella (includes all + license API)
├── tesaiot_config.h           # Configuration (debug, OID, labels)
├── tesaiot_csr.h              # CSR workflow
├── tesaiot_license.h          # License API (NEW in v2.8.0)
├── tesaiot_license_config.h   # Customer editable (UID + key)
├── tesaiot_optiga.h           # OPTIGA integration
├── tesaiot_optiga_core.h      # OPTIGA manager
├── tesaiot_platform.h         # Platform services (MQTT + SNTP)
└── tesaiot_protected_update.h # Protected Update workflow
```

### Usage

```c
// Option 1: Single include (recommended)
#include "tesaiot.h"

// Option 2: Domain-specific includes
#include "tesaiot_platform.h"  // MQTT, SNTP
#include "tesaiot_csr.h"       // CSR workflow
#include "tesaiot_optiga.h"    // OPTIGA Trust M
```

---

## License Key System (v2.8.0)

### 3-Layer Architecture

**Key Innovation: Link-time Binding + IP Protection**

```ini
┌─────────────────────────────────────────────────────────────────┐
│  Layer 1: Configuration (Customer Edits)                         │
│  tesaiot/include/tesaiot_license_config.h                        │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │ #define TESAIOT_DEVICE_UID   "CD163393..."                │  │
│  │ #define TESAIOT_LICENSE_KEY  "MEUCIEJU..."                │  │
│  └───────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                            ↓ (include)
┌─────────────────────────────────────────────────────────────────┐
│  Layer 2: Data Binding (Customer Compiles)                       │
│  tesaiot/src/tesaiot_license_data.c                              │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │ const char* tesaiot_device_uid = TESAIOT_DEVICE_UID;      │  │
│  │ const char* tesaiot_license_key = TESAIOT_LICENSE_KEY;    │  │
│  └───────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                            ↓ (link time)
┌─────────────────────────────────────────────────────────────────┐
│  Layer 3: Verification Logic (IP-Protected - Library Only)       │
│  tesaiot/src/tesaiot_license.c → libtesaiot.a                   │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │ extern const char* tesaiot_device_uid;                    │  │
│  │ extern const char* tesaiot_license_key;                   │  │
│  │                                                            │  │
│  │ // EMBEDDED PUBLIC KEY (IP-Protected)                     │  │
│  │ #define TESAIOT_LICENSE_PUBLIC_KEY_PEM "-----BEGIN..."    │  │
│  │                                                            │  │
│  │ tesaiot_license_status_t tesaiot_license_init(void) {     │  │
│  │   // 1. Read OPTIGA UID from OID 0xE0C2                   │  │
│  │   // 2. Compare with customer UID                         │  │
│  │   // 3. Verify ECDSA signature                            │  │
│  │ }                                                          │  │
│  └───────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

### How It Works

1. **Device UID**: Read from OPTIGA Trust M (27 bytes, factory-programmed)
2. **License Key**: ECDSA signature of SHA-256(UID), signed by TESAIoT Server
3. **Link-time Binding**: Customer compiles `tesaiot_license_data.c` with their values
4. **Verification**: mbedTLS in library verifies signature using embedded public key

### User Configuration (v2.8.0)

**NEW: Config location moved to `tesaiot/include/`**

Users provide 2 values in `tesaiot/include/tesaiot_license_config.h`:

```c
// tesaiot/include/tesaiot_license_config.h - NEW LOCATION

#define TESAIOT_DEVICE_UID      "CD16339301001C000500000A01BB820003004000AE801010712440"
#define TESAIOT_LICENSE_KEY     "MEUCIEjUX7JFWXDpOTXbKLxiNpj5X22d+FFhIGqsZe5UMsZ0AiEA..."
```

### User Workflow (v2.8.0)

```ini
1. Build and flash project (first time)
2. Run Menu 1 → Get OPTIGA Trust M UID
3. Register on TESAIoT Server with UID
4. Download tesaiot_license_config.h
5. Place in tesaiot/include/ folder (moved from proj_cm33_ns/)
6. Rebuild and flash → Licensed!
```

### Security Benefits (v2.8.0)

| Aspect | Old (v2.6) | New (v2.8) |
|--------|-----------|-----------|
| **Config Location** | `proj_cm33_ns/` | `tesaiot/include/` |
| **Data Binding** | ❌ None | ✅ Link-time binding |
| **Public Key** | ⚠️ In header? | ✅ Embedded in .c (IP-protected) |
| **Verification Logic** | ⚠️ Distributed | ✅ Compiled in library only |
| **Customer Sees** | Source code | Header + pre-compiled lib |
| **IP Protection** | Partial | ✅ Complete |

---

## API Reference

### License Functions (from tesaiot.h)

| Function | Description |
|----------|-------------|
| `tesaiot_license_init()` | Initialize and verify license |
| `tesaiot_is_licensed()` | Check if library is licensed |
| `tesaiot_get_device_uid()` | Get device OPTIGA UID |
| `tesaiot_print_device_uid()` | Print UID for registration |
| `tesaiot_license_status_str()` | Get human-readable status |

### Platform Functions (from tesaiot_platform.h)

| Function | Description |
|----------|-------------|
| `tesaiot_mqtt_connect()` | Connect to MQTT broker |
| `tesaiot_mqtt_is_connected()` | Check MQTT connection status |
| `tesaiot_sntp_sync_time()` | Synchronize time via NTP |
| `tesaiot_sntp_get_time()` | Get current Unix timestamp |
| `tesaiot_sntp_is_time_synced()` | Check if time is synced |

### CSR Functions (from tesaiot_csr.h)

| Function | Description |
|----------|-------------|
| `tesaiot_csr_workflow_init()` | Initialize CSR workflow |
| `tesaiot_csr_workflow_start()` | Start CSR workflow |
| `tesaiot_csr_workflow_run()` | Run one iteration |
| `tesaiot_csr_workflow_get_state()` | Get current state |

### OPTIGA Functions (from tesaiot_optiga.h)

| Function | Description |
|----------|-------------|
| `tesaiot_optiga_generate_keypair()` | Generate keypair |
| `tesaiot_optiga_generate_csr()` | Generate CSR |
| `tesaiot_optiga_write_cert()` | Write certificate |
| `tesaiot_optiga_get_cert_oid()` | Get current cert OID |

---

## Usage Example

### Basic License Check

```c
#include "tesaiot.h"

void app_init(void) {
    // 1. Initialize OPTIGA first
    optiga_manager_init();

    // 2. Verify license
    tesaiot_license_status_t status = tesaiot_license_init();
    if (status != TESAIOT_LICENSE_OK) {
        printf("License error: %s\n", tesaiot_license_status_str(status));
        return;
    }

    printf("License verified!\n");

    // 3. Use library functions
    tesaiot_mqtt_connect();
}
```

---

## Directory Structure

### Distribution Structure

```ini
TESAIoT_Distribution/
├── libtesaiot/
│   ├── include/                     # Public headers (9 files in v2.8.0)
│   │   ├── tesaiot.h
│   │   ├── tesaiot_config.h
│   │   ├── tesaiot_csr.h
│   │   ├── tesaiot_license.h         # NEW in v2.8.0
│   │   ├── tesaiot_license_config.h
│   │   ├── tesaiot_optiga.h
│   │   ├── tesaiot_optiga_core.h
│   │   ├── tesaiot_platform.h
│   │   └── tesaiot_protected_update.h
│   ├── lib/
│   │   └── libtesaiot.a              # Universal library
│   ├── src/
│   │   └── tesaiot_license_data.c    # NEW: Customer compiles (link-time binding)
│   └── README.md
│
└── pse84_tesaiot_template/           # Template project for customers
    ├── tesaiot/
    │   ├── include/
    │   │   └── tesaiot_license_config.h  # Customer edits (moved from proj_cm33_ns/)
    │   └── src/
    │       └── tesaiot_license_data.c    # Customer compiles
    └── proj_cm33_ns/
        └── mqtt_device_config_data.c     # NEW: MQTT config binding
```

---

## Building

### Prerequisites

1. ModusToolbox 3.6+
2. ARM GCC Toolchain (14.2.1)
3. PSoC Edge E84 BSP

### Build Command

```bash
cd proj_cm33_ns && make build -j8
```

### Deploy

```bash
./deploy.sh           # Full deploy
./deploy.sh --skip-build  # Skip build, just sync files
```

---

## Platform Support

| Component | Specification |
|-----------|---------------|
| MCU | Infineon PSoC Edge E84 (Cortex-M33) |
| Secure Element | OPTIGA Trust M |
| RTOS | FreeRTOS |
| Network | lwIP |
| TLS | mbedTLS |
| Build System | ModusToolbox |

---

## Security Architecture

**Security Level: Very High** - Hardware-backed security with OPTIGA Trust M secure element.

### Why "Very High" Security?

| Security Aspect | Protection | Mechanism |
|-----------------|------------|-----------|
| **Device Identity (UID)** | Hardware-bound | Factory-programmed in OPTIGA OID 0xE0C2, read-only |
| **Private Keys** | Never extractable | Generated and stored inside OPTIGA, signing happens on-chip |
| **mTLS Authentication** | Hardware-backed | Private key never leaves OPTIGA during TLS handshake |
| **License Verification** | Cryptographically signed | ECDSA P-256 signature verified with public key |

### Key Protection Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                    mTLS Handshake (Hardware-Secured)            │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  OPTIGA Trust M             PSoC Edge            MQTT Broker    │
│  ───────────────            ─────────            ───────────    │
│  │                          │                    │              │
│  │ Certificate ────────────▶│ Send to broker ───▶│              │
│  │ (OID 0xE0E0/E1)          │                    │              │
│  │                          │◀── Challenge ──────│              │
│  │                          │                    │              │
│  │◀── Sign Request ─────────│                    │              │
│  │ Private Key              │                    │              │
│  │ (OID 0xE0F0/F1)          │                    │              │
│  │ NEVER LEAVES CHIP        │                    │              │
│  │      │                   │                    │              │
│  │      └── Signature ─────▶│─── Send ──────────▶│ Verify       │
│  │                          │                    │ ✓ Authentic  │
│  └──────────────────────────┴────────────────────┴──────────────┘
```

### What Makes This Secure?

1. **Hardware Root of Trust**
   - OPTIGA Trust M is CC EAL6+ certified secure element
   - Tamper-resistant hardware with secure key storage
   - Factory UID cannot be modified or cloned

2. **Private Key Protection**
   - Keys generated inside OPTIGA, never exported
   - All cryptographic operations happen on-chip
   - Even firmware cannot read the private key bytes

3. **License Key is NOT a Secret**
   - License key is an ECDSA signature (public data)
   - Verification uses embedded public key
   - The secret (signing key) stays on TESAIoT Server

4. **mTLS Mutual Authentication**
   - Both device and broker verify each other
   - Device proves identity using hardware-bound key
   - Man-in-the-middle attacks are prevented

### OPTIGA Trust M OID Map

| OID | Purpose | Access |
|-----|---------|--------|
| 0xE0C2 | Factory UID | Read-only (hardware-bound) |
| 0xE0E0 | Factory Certificate | Read-only |
| 0xE0E1 | Device Certificate | Read/Write |
| 0xE0F0 | Factory Private Key | Sign only (never read) |
| 0xE0F1 | Device Private Key | Sign only (never read) |

---

## Memory Footprint

### Library Size (libtesaiot.a)

| Module | Code (text) | Data | BSS (RAM) | Total |
|--------|-------------|------|-----------|-------|
| tesaiot_protected_update_isolated.o | 14,110 B | 0 | 2 B | 14,112 B |
| tesaiot_optiga_trust_m.o | 12,873 B | 5 B | 58 B | 12,936 B |
| tesaiot_license.o | 3,884 B | 0 | 34 B | 3,918 B |
| tesaiot_sntp_client.o | 2,955 B | 0 | 13 B | 2,968 B |
| tesaiot_csr_workflow.o | 2,764 B | 0 | 13 B | 2,777 B |
| tesaiot_protected_update_workflow.o | 2,279 B | 0 | 0 | 2,279 B |
| tesaiot_optiga_manager.o | 1,590 B | 0 | 9 B | 1,599 B |
| tesaiot_mqtt.o | 536 B | 0 | 0 | 536 B |
| tesaiot_optiga.o | 118 B | 0 | 0 | 118 B |
| __Total__ | __41,109 B (~40 KB)__ | __5 B__ | __129 B__ | __41,243 B__ |

### Impact on Application

| Metric | TESAIoT Library | Typical Full App | % Impact |
|--------|-----------------|------------------|----------|
| **Flash (Program)** | ~40 KB | ~945 KB | ~4.3% |
| **RAM (BSS)** | ~129 B | ~250 KB | ~0.05% |
| **Initialized Data** | ~5 B | ~2.4 KB | ~0.2% |

### Notes

- TESAIoT library has minimal footprint (~40 KB code, ~129 bytes RAM)
- No dynamic heap allocation within library
- Most RAM usage comes from dependencies (mbedTLS, lwIP, FreeRTOS, Wi-Fi stack)
- Flash usage is dominated by network stacks, not TESAIoT

---

## Changelog

### v2.8.0 (2026-02-01)

**3-Layer License Architecture:**
- Configuration layer: `tesaiot_license_config.h` (macros only)
- Data binding layer: `tesaiot_license_data.c` (link-time binding via extern variables)
- Verification layer: `tesaiot_license.c` (IP-protected, compiled into library)

**Complete IP Protection:**
- Public key embedded in `.c` file (not in header)
- All workflow `.c` files compiled into library only
- 4,021 lines of proprietary code removed from client projects
- Clear separation: Headers (distribute) vs Source (IP-protected)

**MQTT Config Binding:**
- New `mqtt_device_config_data.c` for link-time binding of DEVICE_ID and API_KEY
- Same pattern as license data binding

**Protected Update Workflow:**
- Complete Menu 4 implementation
- MQTT lifecycle management (auto-cleanup)
- OPTIGA timeout fixes

### v2.2.0 (2026-01-29)

- Removed hardcoded menu numbers from all code
- Functions named by feature, not menu position
- Easier to maintain and refactor menus

### v2.1.0 (2026-01-19)

- Header consolidation: 21 files → 8 files
- Improved umbrella header pattern
- Reduced complexity for customers

### v2.0.0 (2026-01-14)

- Implemented License Key System with ECDSA signatures
- Architecture: OPTIGA for UID, mbedTLS for signature verification
- Production-ready (debug output removed)

### v1.0.0

- Initial release with UID-bound library approach

---

## Support

For licensing and technical support:

- Email: support@tesaiot.com
- Website: https://tesaiot.com

---

## Authors

**Assoc. Prof. Wiroon Sriborrirux (BDH)**

- Thai Embedded Systems Association (TESA)
- TESAIoT Platform Creator
- Email: sriborrirux@gmail.com / wiroon@tesa.or.th

**TESAIoT Platform Developer Team**

- In collaboration with Infineon Technologies AG

---

## Copyright

(c) 2025-2026 TESAIoT AIoT Foundation Platform. All rights reserved.

Developed by Assoc. Prof. Wiroon Sriborrirux (BDH) and Thai Embedded Systems Association (TESA).

This library is protected by hardware-bound licensing. Unauthorized use, copying, or distribution is prohibited.
