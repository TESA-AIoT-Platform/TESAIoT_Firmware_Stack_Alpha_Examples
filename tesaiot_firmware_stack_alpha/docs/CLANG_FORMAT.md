# Clang Format Configuration

This project uses `clang-format` to maintain a consistent coding style across the codebase.

## Configuration File

The project contains a `.clang-format` file in the root directory. This file defines the styling rules such as indentation, brace placement, and line length.

### Column Limit

The line length limit is currently set to **120 characters**:

```yaml
ColumnLimit: 120
```

This prevents the formatter from automatically wrapping lines that are less than 120 characters long, keeping the code more readable on modern displays.

## Controlling the Formatter

### Disabling Formatting for Specific Blocks

If you have a block of code (like a large array initialization or a specific function call) that looks better with manual formatting, you can disable the formatter using special comments:

```c
// clang-format off
static const uint8_t large_table[] = {
    0x01, 0x02, 0x03, 0x04,
    ...
};
// clang-format on
```

### Visual Studio Code Integration

If you are using VS Code, you can:
1.  Install the **C/C++** extension from Microsoft.
2.  Enable "Format on Save" in settings: `Editor: Format On Save`.
3.  The editor will automatically use the `.clang-format` file in the project root.

## Usage

To format a file manually from the command line (if `clang-format` is installed):

```bash
clang-format -i path/to/file.c
```
