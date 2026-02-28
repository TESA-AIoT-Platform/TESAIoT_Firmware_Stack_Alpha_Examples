# Fix Clangd IDE Errors

Clangd (the C/C++ language server in VS Code/Cursor) can report errors in this project that the ModusToolbox compiler does not. This happens because clangd lacks the full build environment (sysroot, PDL/BSP paths) that the build system provides.

**If errors persist after editing `.clangd`:** reload the window via **Command Palette → "Developer: Reload Window"**.

---

## Quick reference

| Approach        | Use when                                      |
|----------------|-----------------------------------------------|
| [Ignore all](#ignore-all-diagnostics) | You want to silence all clangd errors (simplest) |
| [LVGL fix](#1-lvgl-undeclared-identifiers) | Only LVGL symbols are undeclared             |
| [Targeted suppressions](#2-individual-suppressions) | You prefer to suppress specific diagnostics  |

---

## Ignore all diagnostics

To suppress every clangd diagnostic (recommended for PDL/BSP projects with many spurious errors):

```yaml
Diagnostics:
  Suppress:
    - '*'
```

---

## Targeted fixes

### 1. LVGL undeclared identifiers

**Error:** `Use of undeclared identifier 'LV_OPA_TRANSP'` (and similar: `LV_SCROLLBAR_MODE_OFF`, `LV_OBJ_FLAG_SCROLLABLE`, `lv_obj_t`, etc.)

**Cause:** Clangd does not know where the LVGL headers are.

**Fix:** Add the LVGL include path to `.clangd`:

```yaml
CompileFlags:
  Add:
    - -I../../mtb_shared/lvgl/release-v9.2.0
```

### 2. Individual suppressions

For PDL/BSP-related errors, add these to `Diagnostics.Suppress`:

| Diagnostic                   | Typical location          | Suppress with                |
|-----------------------------|---------------------------|------------------------------|
| `undeclared_var_use`        | `cy_syslib.h` (uint32_t)  | `undeclared_var_use`         |
| `-Wimplicit-int`            | `cy_crypto_core_mem.h`    | `-Wimplicit-int`             |
| `func_returning_array_function` | `cy_crypto_core_mem.h` | `func_returning_array_function` |
| `typecheck_cond_expect_scalar`  | `cy_dma.h`            | `typecheck_cond_expect_scalar` |

Example:

```yaml
Diagnostics:
  Suppress:
    - undeclared_var_use
    - -Wimplicit-int
    - func_returning_array_function
    - typecheck_cond_expect_scalar
```

---

## Full `.clangd` example

Combining LVGL include path and all suppressions:

```yaml
CompileFlags:
  Add:
    - -I../../mtb_shared/lvgl/release-v9.2.0
    # ... other includes ...

Diagnostics:
  Suppress:
    - '*'
```
