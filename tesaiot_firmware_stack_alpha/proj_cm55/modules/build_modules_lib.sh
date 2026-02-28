#!/usr/bin/env bash
#
# Build module *.c sources into static libraries and save *.a in modules/lib.
# Run from proj_cm55 (or set PROJ_CM55_DIR).
#
# Usage:
#   ./modules/build_modules_lib.sh [module_name ...]
#   If no module names given, builds all modules listed in MODULES below.
#
# Env (optional): CC, AR, CFLAGS, CINCLUDES (full -I/-D flags), PROJ_CM55_DIR

set -e
CONTINUE_ON_ERROR="${CONTINUE_ON_ERROR:-0}"

PROJ_CM55_DIR="${PROJ_CM55_DIR:-$(cd "$(dirname "$0")/.." && pwd)}"
MODULES_DIR="${PROJ_CM55_DIR}/modules"
OUT_DIR="${MODULES_DIR}/lib"
BUILD_BASE="${MODULES_DIR}/.build_lib"

MODULES="${*:-cm55_system cm55_fatal_error rtos_stats cm55_ipc_pipe cm55_ipc_app led_controller lvgl_display}"

CC="${CC:-arm-none-eabi-gcc}"
AR="${AR:-arm-none-eabi-ar}"
CFLAGS="${CFLAGS:--mcpu=cortex-m55 -mthumb -mfloat-abi=softfp -mfpu=fpv5-sp-d16 -O2 -g -Wall -ffunction-sections -fdata-sections}"
CINCLUDES="${CINCLUDES:-}"

if [ -z "$CINCLUDES" ]; then
  MTB_SHARED="${PROJ_CM55_DIR}/../../mtb_shared"
  BSP_DIR="${PROJ_CM55_DIR}/../bsps/TARGET_APP_KIT_PSE84_AI"
  PDL_VER="release-v1.3.0"
  CINCLUDES="-I${PROJ_CM55_DIR}/../shared/include"
  CINCLUDES="$CINCLUDES -I${MODULES_DIR}/cm55_fatal_error -I${MODULES_DIR}/cm55_system"
  CINCLUDES="$CINCLUDES -I${MODULES_DIR}/rtos_stats -I${MODULES_DIR}/cm55_ipc_pipe -I${MODULES_DIR}/cm55_ipc_app -I${MODULES_DIR}/led_controller"
  CINCLUDES="$CINCLUDES -I${MODULES_DIR}/lvgl_display/core -I${MODULES_DIR}/lvgl_display/controller"
  CINCLUDES="$CINCLUDES -I${PROJ_CM55_DIR}/src -I${PROJ_CM55_DIR}/src/ui/core -I${PROJ_CM55_DIR}/src/ui/examples"
  CINCLUDES="$CINCLUDES -I${BSP_DIR} -I${BSP_DIR}/config/GeneratedSource"
  CINCLUDES="$CINCLUDES -I${MTB_SHARED}/mtb-dsl-pse8xxgp/${PDL_VER}/pdl/drivers/include -I${MTB_SHARED}/mtb-dsl-pse8xxgp/${PDL_VER}/pdl/devices/include -I${MTB_SHARED}/mtb-dsl-pse8xxgp/${PDL_VER}/pdl/devices/include/ip"
  CINCLUDES="$CINCLUDES -I${MTB_SHARED}/mtb-dsl-pse8xxgp/${PDL_VER}/hal/include -I${MTB_SHARED}/mtb-dsl-pse8xxgp/${PDL_VER}/device-utils/syspm/include"
  CINCLUDES="$CINCLUDES -I${MTB_SHARED}/mtb-srf/release-v1.1.0/include -I${MTB_SHARED}/mtb-srf/release-v1.1.0/include/COMPONENT_NON_SECURE_DEVICE"
  CINCLUDES="$CINCLUDES -I${MTB_SHARED}/core-lib/release-v1.6.0/include"
  CINCLUDES="$CINCLUDES -I${MTB_SHARED}/cmsis/release-v6.1.0/Core/Include -I${MTB_SHARED}/cmsis/release-v6.1.0/Core/Include/m-profile"
  CINCLUDES="$CINCLUDES -I${MTB_SHARED}/clib-support/release-v1.8.0/include -I${MTB_SHARED}/retarget-io/release-v1.9.0/include"
  CINCLUDES="$CINCLUDES -I${MTB_SHARED}/abstraction-rtos/release-v1.12.0/include -I${MTB_SHARED}/abstraction-rtos/release-v1.12.0/include/COMPONENT_FREERTOS"
  CINCLUDES="$CINCLUDES -I${MTB_SHARED}/freertos/release-v10.6.202/Source/include -I${MTB_SHARED}/freertos/release-v10.6.202/Source/portable/COMPONENT_CM55 -I${MTB_SHARED}/freertos/release-v10.6.202/Source/portable/COMPONENT_CM55/TOOLCHAIN_GCC_ARM"
  CINCLUDES="$CINCLUDES -I${MTB_SHARED}/lvgl/release-v9.2.0 -I${MTB_SHARED}/lvgl/release-v9.2.0/src -I${MTB_SHARED}/lvgl/release-v9.2.0/src/core -I${MTB_SHARED}/lvgl/release-v9.2.0/src/draw -I${MTB_SHARED}/lvgl/release-v9.2.0/src/draw/vg_lite -I${MTB_SHARED}/lvgl/release-v9.2.0/src/draw/sw -I${MTB_SHARED}/lvgl/release-v9.2.0/src/indev"
  CINCLUDES="$CINCLUDES -I${MTB_SHARED}/display-dsi-waveshare-7-0-lcd-c/release-v1.0.0"
  CINCLUDES="$CINCLUDES -I${MTB_SHARED}/mtb-dsl-pse8xxgp/${PDL_VER}/pdl/drivers/third_party/COMPONENT_GFXSS -I${MTB_SHARED}/mtb-dsl-pse8xxgp/${PDL_VER}/pdl/drivers/third_party/COMPONENT_GFXSS/vsi/dcnano8000/include -I${MTB_SHARED}/mtb-dsl-pse8xxgp/${PDL_VER}/pdl/drivers/third_party/COMPONENT_GFXSS/vsi/dcnano8000/DCUser -I${MTB_SHARED}/mtb-dsl-pse8xxgp/${PDL_VER}/pdl/drivers/third_party/COMPONENT_GFXSS/vsi/dcnano8000/DCKernel -I${MTB_SHARED}/mtb-dsl-pse8xxgp/${PDL_VER}/pdl/drivers/third_party/COMPONENT_GFXSS/vsi/dcnano8000/DCKernel/hardware/8000Nano -I${MTB_SHARED}/mtb-dsl-pse8xxgp/${PDL_VER}/pdl/drivers/third_party/COMPONENT_GFXSS/vsi/gcnano/inc -I${MTB_SHARED}/mtb-dsl-pse8xxgp/${PDL_VER}/pdl/drivers/third_party/COMPONENT_GFXSS/vsi/gcnano/VGLiteKernel/rtos"
  CINCLUDES="$CINCLUDES -DCY_RETARGET_IO_CONVERT_LF_TO_CRLF -D_BAREMETAL=0 -DCOMPONENT_CM55 -DCOMPONENT_PSE84 -DCOMPONENT_FREERTOS -DCY_RTOS_AWARE"
  CINCLUDES="$CINCLUDES -DCOMPONENT_MTB_HAL -DMTB_HAL_DRIVER_AVAILABLE_RTC=1 -DMTB_HAL_DRIVER_AVAILABLE_LPTIMER=1"
  if command -v cygpath >/dev/null 2>&1; then
    CINCLUDES_NATIVE=""
    for opt in $CINCLUDES; do
      case "$opt" in
        -I*) CINCLUDES_NATIVE="$CINCLUDES_NATIVE -I$(cygpath -w "${opt#-I}" 2>/dev/null || echo "${opt#-I}")" ;;
        *)   CINCLUDES_NATIVE="$CINCLUDES_NATIVE $opt" ;;
      esac
    done
    CINCLUDES="$CINCLUDES_NATIVE"
  fi
fi

mkdir -p "$OUT_DIR"

to_native_path() {
  if command -v cygpath >/dev/null 2>&1; then
    cygpath -w "$1"
  else
    echo "$1"
  fi
}

for mod in $MODULES; do
  mod_dir="${MODULES_DIR}/${mod}"
  if [ ! -d "$mod_dir" ]; then
    echo "Skip (no dir): $mod"
    continue
  fi
  lib_name="lib${mod}.a"
  build_dir="${BUILD_BASE}/${mod}"
  rm -rf "$build_dir"
  mkdir -p "$build_dir"
  objs=()
  compile_ok=1
  while IFS= read -r -d '' c; do
    [ -n "$c" ] || continue
    base=$(basename "$c" .c)
    o="${build_dir}/${base}.o"
    echo "  CC $c"
    c_native=$(to_native_path "$c")
    o_native=$(to_native_path "$o")
    if ! "$CC" $CFLAGS $CINCLUDES -c "$c_native" -o "$o_native"; then
      if [ "$CONTINUE_ON_ERROR" = "1" ]; then echo "  Skip (build failed): $mod"; compile_ok=0; break; else exit 1; fi
    fi
    objs+=("$o")
  done < <(find "$mod_dir" -maxdepth 3 -name "*.c" -print0 2>/dev/null || true)
  if [ ${#objs[@]} -eq 0 ] || [ "$compile_ok" = "0" ]; then
    [ "$compile_ok" = "1" ] && echo "Skip (no .c): $mod"
    continue
  fi
  echo "  AR $lib_name"
  out_a_native=$(to_native_path "${OUT_DIR}/${lib_name}")
  objs_native=()
  for o in "${objs[@]}"; do objs_native+=("$(to_native_path "$o")"); done
  "$AR" rcs "$out_a_native" "${objs_native[@]}"
done

echo "Libraries in ${OUT_DIR}:"
ls -la "${OUT_DIR}"/*.a 2>/dev/null || true
