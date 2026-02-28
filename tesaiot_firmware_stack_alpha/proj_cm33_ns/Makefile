################################################################################
# \file Makefile
# \version 1.0
#
# \brief
# Makefile for configuring the Cortex-M33 core project
#
################################################################################
# \copyright
# Copyright 2024-2025, Cypress Semiconductor Corporation (an Infineon company)
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

include ../common.mk
TOOLCHAIN=GCC_ARM

################################################################################
# Basic Configuration
################################################################################

# Name of application (used to derive name of final linked file).
#
# If APPNAME is edited, ensure to update or regenerate launch
# configurations for your IDE.
APPNAME=proj_cm33_ns

# If set to "true" or "1", display full command-lines when building.
VERBOSE=


################################################################################
# Advanced Configuration
################################################################################

# Enable optional code that is ordinarily disabled by default.
#
# Available components depend on the specific targeted hardware and firmware
# in use. In general, if you have
#
#    COMPONENTS=foo bar
#
# ... then code in directories named COMPONENT_foo and COMPONENT_bar will be
# added to the build
#
COMPONENTS+= FREERTOS LWIP MBEDTLS RTOS_AWARE

# BSXLITE sensor fusion component (BMI270 + Bosch BSXlite).
COMPONENTS+= BSXLITE
SEARCH+= ../COMPONENT_BSXLITE
DEFINES+= COMPONENT_BSXLITE

# Touch on CM33: GT911 driver for Option B (touch via IPC to CM55).
DEFINES+= MTB_CTP_GT911
SEARCH+= ../../mtb_shared/touch-ctp-gt911/release-v1.0.0

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS=

# Use CY_IGNORE to exclude directories from the automatic source discovery process.
CY_IGNORE+=modules/wifi_scan_v0
CY_IGNORE+=../COMPONENT_BSXLITE/genericbsxlite_v1-0-2
CY_IGNORE+=../COMPONENT_BSXLITE/BSXlite_v1.0.2

CORE=CM33
CORE_NAME=CM33_0

# By default the build system automatically looks in the Makefile's directory
# tree for source code and builds it. The SOURCES variable can be used to
# manually add source code to the build process from a location not searched
# by default, or otherwise not found by the build system.
SOURCES+=$(wildcard ../shared/source/COMPONENT_CM33/*.c)

SOURCES+= modules/cm33_system/cm33_system.c
INCLUDES+= modules/cm33_system

SOURCES+= modules/wifi_connect/wifi_connect.c
INCLUDES+= modules/wifi_connect

SOURCES+= modules/wifi_manager/wifi_manager.c
INCLUDES+= modules/wifi_manager

SOURCES+= modules/udp_server/udp_server_lib.c
SOURCES+= modules/udp_server/udp_server_app.c
INCLUDES+= modules/udp_server

SOURCES+= modules/cm33_cli/cm33_cli.c
INCLUDES+= modules/cm33_cli

SOURCES+= modules/date_time/date_time.c
INCLUDES+= modules/date_time

# Like SOURCES, but for include directories. Value should be paths to
# directories (without a leading -I).
INCLUDES+=\
	. \
	../shared/include \
	modules/ipc_log \
	modules/error_handler \
	modules/event_bus

# Custom configuration of mbedtls library.
MBEDTLSFLAGS = MBEDTLS_USER_CONFIG_FILE='"configs/mbedtls_user_config.h"'

# Add additional defines to the build process (without a leading -D).
DEFINES+=$(MBEDTLSFLAGS) CYBSP_WIFI_CAPABLE CY_RETARGET_IO_CONVERT_LF_TO_CRLF CY_RTOS_AWARE

# Default configuration of mbedtls library.
DEFINES+=MBEDTLS_CONFIG_FILE='"mbedtls/mbedtls_config.h"'
# PSA configuration of mbedtls library.
DEFINES+=MBEDTLS_PSA_CRYPTO_CONFIG_FILE='"configs/ifx_psa_crypto_config.h"'

# Select softfp or hardfp floating point. Default is softfp.
VFP_SELECT+=

# Additional / custom C compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
# Suppress MEM_SIZE macro redefinition (lwIP opt.h vs GFXSS viv_dc_options.h).
CFLAGS+= -Wno-macro-redefined

# Additional / custom C++ compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CXXFLAGS+=

# Additional / custom assembler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
ASFLAGS+=

# Additional / custom linker flags.
LDFLAGS+=

# Additional / custom libraries to link in to the application.
LDLIBS+=

# Path to the linker script to use (if empty, use the default linker script).
LINKER_SCRIPT=

# Custom pre-build commands to run.
PREBUILD=

# Custom post-build commands to run.
POSTBUILD=

################################################################################
# Paths
################################################################################

# Relative path to the project directory (default is the Makefile's directory).
#
# This controls where automatic source code discovery looks for code.
CY_APP_PATH=

# Relative path to the shared repo location.
#
# All .mtb files have the format, <URI>#<COMMIT>#<LOCATION>. If the <LOCATION>
# field begins with $$ASSET_REPO$$, then the repo is deposited in the path
# specified by the CY_GETLIBS_SHARED_PATH variable. The default location is one
# directory level above the current app directory.
# This is used with CY_GETLIBS_SHARED_NAME variable, which specifies the
# directory name.
CY_GETLIBS_SHARED_PATH=../..

# Directory name of the shared repo location.
#
CY_GETLIBS_SHARED_NAME=mtb_shared

include $(CY_TOOLS_DIR)/make/start.mk
