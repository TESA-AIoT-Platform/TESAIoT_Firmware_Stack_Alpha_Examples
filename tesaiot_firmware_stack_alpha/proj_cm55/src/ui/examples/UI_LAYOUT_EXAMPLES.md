# LVGL Layout Examples

This document provides a comprehensive guide, specification, and layout details for the 10 UI layout examples implemented in this directory. These examples demonstrate the usage of the custom layout utilities (`ui_layout.h`) for building clean, responsive interfaces with LVGL.

---

## 1. Directory Structure

The files are organized to keep core logic separate from demonstration code:

```text
src/
  ui/
    core/
      ui_layout.h           # Row/Col/Grid/Stack helpers
      ui_layout.c
      ui_style.h            # Spacing, padding, and radius constants

    examples/
      ui_layout_examples.h  # Aggregator declaring all example functions
      ui_run_examples.c     # Runner to launch specific examples
      
      ui_layout_example01_flex_basics.c/h
      ui_layout_example02_spacer_grow.c/h
      ui_layout_example03_alignment.c/h
      ui_layout_example04_wrap_chips.c/h
      ui_layout_example05_reverse.c/h
      ui_layout_example06_scroll.c/h
      ui_layout_example07_grid_form.c/h
      ui_layout_example08_grid_dashboard.c/h
      ui_layout_example09_stack_overlay.c/h
      ui_layout_example10_cards_pills.c/h
      ui_layout_example11_symbols.c/h
```

---

## 2. Implementation Specification

All examples follow a consistent template to ensure they are easy to compare and maintain.

### Public API
Each example provides exactly one entry function:
* **Function**: `void ui_layout_exampleXX_name(lv_obj_t *parent);`
* **Dependency**: Includes only its own header and `ui_layout.h` (and `ui_style.h` if needed).

### Screen Lifecycle
* **Parent Object**: The `parent` is assumed to be an empty container or screen.
* **Root Container**: Every example creates a single root container (usually via `ui_clean_col`) that is set to `100% x 100%` size.

### Layout Skeleton
Inside each root container, the layout usually follows this pattern:
1. **Header Bar**: Displays the Example ID and Title.
2. **Body Container**: Houses the actual demonstration content.
3. **Pass Criteria (Optional)**: A label or comment describing the expected visual behavior.

### Spacing & Sizing
To maintain a premium feel, the examples use a consistent spacing scale:
* **Small Gap**: 4px
* **Normal Gap**: 8px
* **Large Gap**: 12px
* **Screen Padding**: 12px or 16px

---

## 3. Example Catalog

### Example 01 – Flex Basics
*   **Focus**: Row vs Column, gaps, and basic padding.
*   **Layout**: A "ROW" section with buttons A-D and a "COLUMN" section with buttons 1-4.
*   **Correct Look**: Distinct horizontal and vertical stacks with visible, consistent spacing.

### Example 02 – Spacer & Grow
*   **Focus**: Flexible spacing and expanding items (`flex_grow`).
*   **Layout**: A toolbar with a spacer pushing items to the far right, and a "Grow" demo where the middle item expands.
*   **Correct Look**: Right-side icons stick to the edge; the "Grow" box consumes all available middle space.

### Example 03 – Alignment
*   **Focus**: Main-axis and Cross-axis alignment variations.
*   **Layout**: Multiple boxes demonstrating `START`, `CENTER`, and `END` alignments.
*   **Correct Look**: Items shift position exactly as labeled.

### Example 04 – Wrap Chips
*   **Focus**: Automatic line wrapping for flow-style layouts.
*   **Layout**: A collection of chips/pills with varying text lengths (e.g., "LVGL", "Embedded", "Very long label").
*   **Correct Look**: Items flow to the next line automatically when the edge is hit, maintaining consistent row and item gaps.

### Example 05 – Reverse & Wrap-Reverse
*   **Focus**: Reversed ordering and reverse wrapping.
*   **Layout**: Sections showing `Row Reverse` (D C B A) and `Column Reverse`, plus a `Wrap Reverse` demo.
*   **Correct Look**: Visual order is the opposite of the creation order.

### Example 06 – Scroll Containers
*   **Focus**: Independent vertical and horizontal scrolling.
*   **Layout**: A vertical "feed" with many items and a horizontal "strip" of cards.
*   **Correct Look**: Natural scrolling behavior in both dimensions within constrained containers.

### Example 07 – Grid Form
*   **Focus**: Structured 2-column layouts and column spanning.
*   **Layout**: Labels on the left, "inputs" on the right, and a "Submit" button spanning both columns at the bottom.
*   **Correct Look**: Perfectly aligned columns; the submit button stretches across the full grid width.

### Example 08 – Grid Dashboard
*   **Focus**: Complex grid layouts with varied spans.
*   **Layout**: A tile-based dashboard where some tiles span 2 columns or 2 rows.
*   **Correct Look**: A clean, snapping tile interface without overlapping or inconsistent gaps.

### Example 09 – Stack Overlay
*   **Focus**: Layering items (e.g., badges or loading screens) using the stack helper.
*   **Layout**: A "Messages" card with a notification badge and a "Loading" layer covering a content area.
*   **Correct Look**: The badge floats correctly over the corner; the loading layer dims and centers text over the background.

### Example 10 – Cards & Pills
*   **Focus**: Consistent component styling (radius, shadows, padding).
*   **Layout**: A list of cards (title/subtitle) and a row of pills in Small/Medium/Large variants.
*   **Correct Look**: Uniform visual language; pills are fully rounded; cards have consistent elevation/borders.

### Example 11 – Symbols Browser
*   **Focus**: Visualizing built-in FontAwesome symbols and using `lv_tabview` for categorization.
*   **Layout**: A TabView with 4 categories (Media, Common, Hardware, Arrows), each containing a scrollable grid of mini-cards.
*   **Correct Look**: Smooth tab switching and vertical scrolling within each tab; a comprehensive catalog of all LVGL symbols.

---

## 4. How to Run Examples

Each example is designed to be run "one-by-one".

1.  **Select Example**: In `ui_run_examples.c`, uncomment the specific example you wish to view.
2.  **Clean State**: The runner creates a fresh screen or clears the current one before passing it to the example function.
3.  **Execute**: Call `run_ui_layout_examples()` from your main application loop or initialization sequence.

```c
// Example: Switching to the Grid Form demo in ui_run_examples.c
void run_ui_layout_examples(void) {
    lv_obj_t *scr = lv_scr_act();
    lv_obj_clean(scr);
    ui_layout_example07_grid_form(scr);
}
```

---

## 5. Semantic Layout API

The library provides a set of high-level helpers in `ui_layout.h` to simplify UI development:

| Category | Helpers |
| :--- | :--- |
| **Spacing** | `ui_pad_all`, `ui_pad_x`, `ui_pad_y`, `ui_pad_xy`, `ui_gap`, `ui_margin_all`, `ui_margin_x`, `ui_margin_y`, `ui_margin_xy` |
| **Sizing** | `ui_width`, `ui_width_pct`, `ui_height`, `ui_height_pct`, `ui_size`, `ui_size_pct`, `ui_width_fill`, `ui_height_fill`, `ui_wrap_content`, `ui_width_content`, `ui_height_content` |
| **Positioning** | `ui_fill_parent`, `ui_center_in_parent` |
| **Flex Presets** | `ui_flex_center`, `ui_flex_between`, `ui_flex_around`, `ui_flex_start`, `ui_flex_end` |
| **Visual Styling** | `ui_radius`, `ui_bg_color`, `ui_bg_opa`, `ui_border`, `ui_make_pill` |
| **State** | `ui_show`, `ui_hide`, `ui_toggle`, `ui_opacity` |
