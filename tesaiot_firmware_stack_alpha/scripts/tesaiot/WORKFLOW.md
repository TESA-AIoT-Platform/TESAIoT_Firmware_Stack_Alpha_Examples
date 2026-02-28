# TESAIoT Development Workflow

**Version:** 2.0.0
**Last Updated:** 2026-01-14

---

## Overview

This document describes the development workflow for the TESAIoT Library, including how to make changes, build the library, and deploy to different projects.

---

## Project Hierarchy

```
POC_PSE84_Workspace/
│
├── official_pse84_trustm_mTLS_tesaiot/    [1] MASTER PROJECT
│   ├── proj_cm33_ns/                       Source code location
│   │   ├── tesaiot_*.c                     Library source files
│   │   ├── tesaiot_*.h                     Header files
│   │   └── tesaiot_license_config.h        License config (dev)
│   └── tesaiot/
│       ├── scripts/
│       │   └── build_library.sh            Library build script
│       ├── lib/
│       │   └── libtesaiot.a                Built library output
│       └── include/                        Public headers
│
├── TESAIoT_Distribution/
│   └── pse84_tesaiot_template/            [2] DISTRIBUTION TEMPLATE
│       ├── proj_cm33_ns/
│       │   └── tesaiot_license_config.h    Placeholder config
│       └── tesaiot/
│           ├── lib/libtesaiot.a            Library for distribution
│           └── include/                    Headers for distribution
│
└── pse84_tesaiot_client/                  [3] DEPLOYMENT/TEST PROJECT
    ├── proj_cm33_ns/
    │   └── tesaiot_license_config.h        Real license config
    └── tesaiot/
        ├── lib/libtesaiot.a                Deployed library
        └── include/                        Deployed headers
```

---

## Workflow Steps

### Step 1: Develop in Master Project

All development work should be done in the **master project** (`official_pse84_trustm_mTLS_tesaiot`).

```bash
# Navigate to master project
cd /path/to/POC_PSE84_Workspace/official_pse84_trustm_mTLS_tesaiot

# Edit source files in proj_cm33_ns/
# - tesaiot_license.c
# - tesaiot_mqtt.c
# - tesaiot_csr_workflow.c
# - etc.

# Build the project
cd proj_cm33_ns
make build -j4
```

#### Key Files to Edit

| File | Purpose |
|------|---------|
| `tesaiot_license.c` | License verification (ECDSA) |
| `tesaiot_license.h` | License API definitions |
| `tesaiot_mqtt.c` | MQTT client implementation |
| `tesaiot_csr_workflow.c` | CSR generation workflow |
| `tesaiot_optiga*.c` | OPTIGA Trust M integration |

---

### Step 2: Build the Library

After making changes and successfully building the project, create the distributable library.

```bash
# From master project root
cd tesaiot/scripts
./build_library.sh
```

#### What build_library.sh Does

1. Copies headers from `proj_cm33_ns/` to `tesaiot/include/`
2. Copies object files from build output to `tesaiot/build/`
3. Strips debug symbols (production build)
4. Creates `libtesaiot.a` in `tesaiot/lib/`
5. Generates SHA256 checksum for integrity verification

#### Output

```
tesaiot/
├── lib/
│   ├── libtesaiot.a           # Universal static library
│   └── libtesaiot.a.sha256    # Integrity checksum
└── include/
    ├── tesaiot.h
    ├── tesaiot_license.h
    ├── tesaiot_mqtt.h
    └── ...
```

---

### Step 3: Copy to Distribution Template

Copy the built library to the distribution template for customer delivery.

```bash
# From master project root
cd /path/to/POC_PSE84_Workspace/official_pse84_trustm_mTLS_tesaiot

# Copy library
cp tesaiot/lib/libtesaiot.a ../TESAIoT_Distribution/pse84_tesaiot_template/tesaiot/lib/
cp tesaiot/lib/libtesaiot.a.sha256 ../TESAIoT_Distribution/pse84_tesaiot_template/tesaiot/lib/

# Copy headers
cp tesaiot/include/*.h ../TESAIoT_Distribution/pse84_tesaiot_template/tesaiot/include/
```

#### Verify Template Has Placeholder Config

The template should have a placeholder `tesaiot_license_config.h`:

```c
// tesaiot_license_config.h - PLACEHOLDER
// Replace with your actual license configuration

#ifndef TESAIOT_LICENSE_CONFIG_H
#define TESAIOT_LICENSE_CONFIG_H

#define TESAIOT_DEVICE_UID      "YOUR_DEVICE_UID_HERE"
#define TESAIOT_LICENSE_KEY     "YOUR_LICENSE_KEY_HERE"

#endif
```

---

### Step 4: Deploy to Client Project

For testing or actual deployment, copy to the client project.

```bash
# From master project root
cd /path/to/POC_PSE84_Workspace/official_pse84_trustm_mTLS_tesaiot

# Copy library
cp tesaiot/lib/libtesaiot.a ../pse84_tesaiot_client/tesaiot/lib/
cp tesaiot/lib/libtesaiot.a.sha256 ../pse84_tesaiot_client/tesaiot/lib/

# Copy headers
cp tesaiot/include/*.h ../pse84_tesaiot_client/tesaiot/include/

# Build client project
cd ../pse84_tesaiot_client/proj_cm33_ns
make clean
make build -j4

# Program device
make program
```

---

## Quick Scripts

### Full Build and Deploy Script

Create `deploy.sh` in the master project root:

```bash
#!/bin/bash
# deploy.sh - Build library and deploy to all targets

set -e

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
WORKSPACE="$(dirname "$PROJECT_ROOT")"

echo "=== Building Project ==="
cd "$PROJECT_ROOT/proj_cm33_ns"
make build -j4

echo "=== Building Library ==="
cd "$PROJECT_ROOT/tesaiot/scripts"
./build_library.sh

echo "=== Copying to Template ==="
cp "$PROJECT_ROOT/tesaiot/lib/"* "$WORKSPACE/TESAIoT_Distribution/pse84_tesaiot_template/tesaiot/lib/"
cp "$PROJECT_ROOT/tesaiot/include/"*.h "$WORKSPACE/TESAIoT_Distribution/pse84_tesaiot_template/tesaiot/include/"

echo "=== Copying to Client ==="
cp "$PROJECT_ROOT/tesaiot/lib/"* "$WORKSPACE/pse84_tesaiot_client/tesaiot/lib/"
cp "$PROJECT_ROOT/tesaiot/include/"*.h "$WORKSPACE/pse84_tesaiot_client/tesaiot/include/"

echo "=== Done ==="
echo "Library deployed to:"
echo "  - TESAIoT_Distribution/pse84_tesaiot_template/"
echo "  - pse84_tesaiot_client/"
```

---

## Command Reference

| Task | Command | Location |
|------|---------|----------|
| Build project | `make build -j4` | `proj_cm33_ns/` |
| Clean build | `make clean` | `proj_cm33_ns/` |
| Build library | `./build_library.sh` | `tesaiot/scripts/` |
| Program device | `make program` | `proj_cm33_ns/` |
| Debug | `make debug` | `proj_cm33_ns/` |

---

## Troubleshooting

### "Project not built" Error

```
Error: Project not built
Please build the main project first:
  cd proj_cm33_ns && make build
```

**Solution:** Build the project before running `build_library.sh`.

### "No object files found" Error

**Solution:** Verify the build was successful and check the build output directory exists:
```bash
ls proj_cm33_ns/build/Debug/local/tesaiot_*.o
```

### Library Not Linking in Client

**Check:**
1. Library path in Makefile: `LDFLAGS += -L../tesaiot/lib -ltesaiot`
2. Header path: `CFLAGS += -I../tesaiot/include`
3. Library file exists: `ls tesaiot/lib/libtesaiot.a`

---

## License Configuration

### For Development (Master Project)

Use your actual device UID and license key:

```c
// proj_cm33_ns/tesaiot_license_config.h
#define TESAIOT_DEVICE_UID      "CD16339301001C000500000A01BB820003004000AE801010712440"
#define TESAIOT_LICENSE_KEY     "MEUCIEjUX7JFWXDpOTXbKLxiNpj5X22d..."
```

### For Distribution (Template)

Use placeholder values:

```c
// proj_cm33_ns/tesaiot_license_config.h
#define TESAIOT_DEVICE_UID      "YOUR_DEVICE_UID_HERE"
#define TESAIOT_LICENSE_KEY     "YOUR_LICENSE_KEY_HERE"
```

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 2.0.0 | 2026-01-14 | Universal library with ECDSA License Key System |
| 1.0.0 | 2026-01-13 | Initial UID-bound library approach |

---

## Contact

For technical support: support@tesaiot.com
