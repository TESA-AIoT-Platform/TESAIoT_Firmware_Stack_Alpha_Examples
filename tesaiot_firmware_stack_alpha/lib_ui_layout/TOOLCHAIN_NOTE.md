# Toolchain Note: proj_cm55 and LLVM_ARM

The lib_ui_layout static library (`libui_layout.a`) is built with **GCC** (arm-none-eabi-gcc), while proj_cm55 uses **LLVM_ARM** (armclang) by default.

For C code with the `softfp` ABI, GCC-built object files are typically link-compatible with armclang-built applications. The library is compiled with:

- `-mfloat-abi=softfp -mfpu=fpv5-sp-d16`
- `-mcpu=cortex-m55`

These flags match the proj_cm55 configuration, so the library should link without issues.

If linker or ABI errors occur when linking `libui_layout.a` into proj_cm55, consider:

1. **Switch proj_cm55 to GCC_ARM**: In `proj_cm55/Makefile`, change `TOOLCHAIN=LLVM_ARM` to `TOOLCHAIN=GCC_ARM` so the application and library share the same toolchain.

2. **Build the library with armclang**: Modify `scripts/ui_layout/Makefile` to use the ARM/LLVM toolchain instead of GCC, so the library is built with the same compiler as the application.
