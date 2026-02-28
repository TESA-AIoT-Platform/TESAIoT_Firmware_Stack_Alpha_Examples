# lib_ui_layout Static Library Makefile (self-contained)
# Creates libui_layout.a from local sources (CM55 target only)
#
# Usage:
#   make           - Build library
#   make clean     - Clean build artifacts
#
# Override GCC path for your system:
#   GCC_PATH=/path/to/gcc/bin make

SRC_DIR := .
BUILD_DIR := build
OUTPUT_DIR := lib
INCLUDE_DIR := include
LIB_NAME := libui_layout.a

PROJECT_ROOT := ..
MTB_SHARED := $(PROJECT_ROOT)/../mtb_shared
PROJ_CM55 := $(PROJECT_ROOT)/proj_cm55

GCC_PATH ?= C:/Users/drsanti/Infineon/Tools/mtb-gcc-arm-eabi/14.2.1/gcc/bin
CC := $(GCC_PATH)/arm-none-eabi-gcc
AR := $(GCC_PATH)/arm-none-eabi-ar

CFLAGS := -mcpu=cortex-m55 -mthumb -mfloat-abi=softfp -mfpu=fpv5-sp-d16
CFLAGS += -O2 -g -Wall
CFLAGS += -ffunction-sections -fdata-sections

INCLUDES := \
    -I$(SRC_DIR) \
    -I$(INCLUDE_DIR) \
    -Iconfig \
    -I$(PROJ_CM55)/src/lvgl_src \
    -I$(MTB_SHARED)/lvgl/release-v9.2.0 \
    -I$(MTB_SHARED)/core-lib/release-v1.6.0/include \
    -I$(MTB_SHARED)/cmsis/release-v6.1.0/Core/Include \
    -I$(MTB_SHARED)/cmsis/release-v6.1.0/Core/Include/m-profile \
    -I$(MTB_SHARED)/freertos/release-v10.6.202/Source/include \
    -I$(MTB_SHARED)/freertos/release-v10.6.202/Source/portable/COMPONENT_CM55 \
    -I$(MTB_SHARED)/freertos/release-v10.6.202/Source/portable/COMPONENT_CM55/TOOLCHAIN_GCC_ARM \
    -I$(MTB_SHARED)/abstraction-rtos/release-v1.12.0/include

DEFINES := \
    -DCOMPONENT_CM55 \
    -DCOMPONENT_PSE84 \
    -DCOMPONENT_SOFTFP \
    -DCOMPONENT_FREERTOS \
    -DCY_RTOS_AWARE

UI_LAYOUT_SOURCES := $(SRC_DIR)/ui_layout.c
UI_LAYOUT_OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(UI_LAYOUT_SOURCES))

.PHONY: all clean copy_headers

all: copy_headers $(OUTPUT_DIR)/$(LIB_NAME)
	@echo ""
	@echo "============================================"
	@echo "  lib_ui_layout Built Successfully!"
	@echo "============================================"
	@echo "Library: $(OUTPUT_DIR)/$(LIB_NAME)"
	@echo "Headers: $(INCLUDE_DIR)/"
	@echo ""
	@$(AR) -t $(OUTPUT_DIR)/$(LIB_NAME)
	@echo ""

$(OUTPUT_DIR)/$(LIB_NAME): $(UI_LAYOUT_OBJECTS) | $(OUTPUT_DIR)
	@echo "Creating static library: $@"
	$(AR) rcs $@ $(UI_LAYOUT_OBJECTS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "Compiling: $<"
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

copy_headers:
	@echo "Copying headers to $(INCLUDE_DIR)/..."
	@mkdir -p $(INCLUDE_DIR)
	@cp -f $(SRC_DIR)/ui_layout.h $(INCLUDE_DIR)/ 2>/dev/null || true
	@cp -f $(SRC_DIR)/ui_style.h $(INCLUDE_DIR)/ 2>/dev/null || true
	@echo "Headers copied."

clean:
	rm -rf $(BUILD_DIR) $(OUTPUT_DIR)/$(LIB_NAME)
