# Switching Target Kits (TARGET)

This project supports multiple PSoC Edge E84 target kits. Use the steps below to switch from one kit to another.

## Supported targets

| TARGET value | Kit |
|--------------|-----|
| `APP_KIT_PSE84_EVAL_EPC2` | PSoC Edge E84 Evaluation Kit (EPC2) – default / tested |
| `APP_KIT_PSE84_EVAL_EPC4` | PSoC Edge E84 Evaluation Kit (EPC4) |
| `APP_KIT_PSE84_AI` | PSoC Edge E84 AI Kit |

## Git Bash: make in PATH

If you use Git Bash and get `make: command not found`, add ModusToolbox's make to your PATH. ModusToolbox installs GNU make under `tools_3.x\modus-shell\bin` (e.g. `C:\Users\drsanti\ModusToolbox\tools_3.6\modus-shell\bin`).

1. **Find the make bin path**
   Under `%USERPROFILE%\ModusToolbox\tools_3.x`, open `modus-shell\bin`; that directory contains `make.exe`. Or run in Command Prompt: `dir /s /b "%USERPROFILE%\ModusToolbox\make.exe"` and use the `bin` directory that contains it.

2. **Add to Git Bash**
   Convert the path to Git Bash form (e.g. `C:\Users\drsanti\ModusToolbox\tools_3.6\modus-shell\bin` → `/c/Users/drsanti/ModusToolbox/tools_3.6/modus-shell/bin`). Edit `~/.bashrc` (or `~/.bash_profile`) and add:
   ```bash
   export PATH="/c/Users/drsanti/ModusToolbox/tools_3.6/modus-shell/bin:$PATH"
   ```
   Replace the path with your actual `modus-shell\bin` directory if different. Save, then run `source ~/.bashrc` or restart Git Bash.

3. **Verify**
   From the repo root: `make --version` and `make getlibs`.

See [BUILD_SETUP.md](BUILD_SETUP.md) for more detail.

---

## How to switch to EPC2 or EPC4

The EPC2 BSP is already in the repository under `bsps/TARGET_APP_KIT_PSE84_EVAL_EPC2`. For EPC4, the BSP may need to be added via Library Manager or the repo may already include it.

### 1. Set TARGET in common.mk

Edit **`common.mk`** at the repository root:

```makefile
TARGET=APP_KIT_PSE84_EVAL_EPC2
```

Or for EPC4:

```makefile
TARGET=APP_KIT_PSE84_EVAL_EPC4
```

### 2. Resolve libraries and rebuild

From the repository root:

```bash
make getlibs
make clean build -j8
```

### 3. Program (optional)

```bash
make program
```

### 4. Library Manager (optional)

To refresh IDE launch configurations:

```bash
make library-manager
```

Note: `make library-manager` can fail with "Libraries: bsp ... not found" if `getlibs` has not been run first. Always run `make getlibs` before `make library-manager` or `make build`.

---

## How to switch to AI Kit (APP_KIT_PSE84_AI)

### One-time setup: add the AI Kit BSP

The build expects the BSP at **`bsps/TARGET_APP_KIT_PSE84_AI`**. If that folder does not exist, do this once.

#### Step A: Clone the BSP into bsps

From the repository root:

```bash
cd bsps
git clone --depth 1 https://github.com/Infineon/TARGET_KIT_PSE84_AI.git TARGET_APP_KIT_PSE84_AI
cd ..
```

The folder **must** be named `TARGET_APP_KIT_PSE84_AI` (the build looks for `bsps/TARGET_$(TARGET)`).

#### Step B: Add BSP dependency descriptors

The cloned BSP does not include a `deps` folder. Create it and add the following `.mtbx` files so the build can resolve `core-make`, `recipe-make` (from mtb-dsl-pse8xxgp), and other BSP dependencies.

Create the directory:

```bash
mkdir -p bsps/TARGET_APP_KIT_PSE84_AI/deps
```

Create these files in `bsps/TARGET_APP_KIT_PSE84_AI/deps/` with the exact contents below.

**core-make.mtbx**

```
https://github.com/Infineon/core-make#release-v3.8.0#$$ASSET_REPO$$/core-make/release-v3.8.0
```

**core-lib.mtbx**

```
https://github.com/Infineon/core-lib#release-v1.6.0#$$ASSET_REPO$$/core-lib/release-v1.6.0
```

**mtb-dsl-pse8xxgp.mtbx**

```
https://github.com/Infineon/mtb-dsl-pse8xxgp#release-v1.2.0#$$ASSET_REPO$$/mtb-dsl-pse8xxgp/release-v1.2.0
```

**se-rt-services-utils.mtbx**

```
https://github.com/Infineon/se-rt-services-utils#release-v1.2.0#$$ASSET_REPO$$/se-rt-services-utils/release-v1.2.0
```

**bt-fw-mur-cyw55513.mtbx**

```
https://github.com/Infineon/bt-fw-mur-cyw55513#release-v1.0.0#$$ASSET_REPO$$/bt-fw-mur-cyw55513/release-v1.0.0
```

(Each file contains a single line; no trailing newline required.)

After this one-time setup, the folder `bsps/TARGET_APP_KIT_PSE84_AI` (and its `deps`) can be committed so others do not need to repeat Steps A and B.

### Switch to AI Kit and build

1. **Set TARGET in common.mk**

   Edit **`common.mk`** at the repository root:

   ```makefile
   TARGET=APP_KIT_PSE84_AI
   ```

2. **Resolve libraries**

   ```bash
   make getlibs
   ```

   This fetches BSP dependencies (e.g. `bt-fw-mur-cyw55513`, core-make, mtb-dsl-pse8xxgp) into the shared repo.

3. **Build**

   ```bash
   make clean build -j8
   ```

   Or without clean:

   ```bash
   make build -j8
   ```

4. **Program (optional)**

   ```bash
   make program
   ```

---

## Switching back from AI Kit to EPC2 (or EPC4)

1. Edit **`common.mk`** and set:

   ```makefile
   TARGET=APP_KIT_PSE84_EVAL_EPC2
   ```

   (or `APP_KIT_PSE84_EVAL_EPC4` if that BSP is present).

2. From the repo root:

   ```bash
   make getlibs
   make clean build -j8
   ```

The EPC2 BSP is already in `bsps/TARGET_APP_KIT_PSE84_EVAL_EPC2`, so no clone or deps setup is needed. You can switch back and forth by changing `TARGET` and running `make getlibs` then `make build`.

---

## Kit-specific behavior

- **EPC2 / EPC4**: Only the BSP and target name differ; no extra build defines.
- **AI Kit** (`APP_KIT_PSE84_AI`): The CM55 build adds the define `USE_KIT_PSE84_AI`, which is used in `proj_cm55/modules/lvgl_display/core/display_i2c_config.h` to select the AI kit display I2C controller. No manual code changes are required when switching to the AI kit.

---

## Troubleshooting

### "Libraries: bsp core-make recipe-make not found"

- The BSP for the selected `TARGET` is missing or its dependencies are not resolved.
- For **APP_KIT_PSE84_AI**: complete the [one-time AI Kit BSP setup](#one-time-setup-add-the-ai-kit-bsp) (clone BSP into `bsps/TARGET_APP_KIT_PSE84_AI` and add the `deps` folder with the five `.mtbx` files), then run `make getlibs`.
- For any target: run `make getlibs` before `make build` or `make library-manager`.

### "unknown type name 'QueueHandle_t'" or "implicit declaration of function 'xQueueSend'"

- FreeRTOS queue types and APIs are used before the queue header is included. In `proj_cm33_ns`, ensure any source that uses queues includes `FreeRTOS.h` and `queue.h` before using `QueueHandle_t`, `xQueueCreate`, `xQueueSend`, or `xQueueReceive`.

### "#error include FreeRTOS.h must appear in source files before include task.h"

- FreeRTOS requires `FreeRTOS.h` to be included before `task.h`. In the failing source file, add `#include "FreeRTOS.h"` before `#include "task.h"` (or before any header that pulls in `task.h`).
