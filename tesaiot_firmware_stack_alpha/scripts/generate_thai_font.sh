#!/usr/bin/env bash
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
LATIN_URL="https://raw.githubusercontent.com/openmaptiles/fonts/master/noto-sans/NotoSans-Regular.ttf"
THAI_URL="https://raw.githubusercontent.com/openmaptiles/fonts/master/noto-sans/NotoSansThai-Regular.ttf"
LATIN_TTF="$REPO_ROOT/NotoSans-Regular.ttf"
THAI_TTF="$REPO_ROOT/NotoSansThai-Regular.ttf"
OUT_C="$REPO_ROOT/proj_cm55/src/ui/widgets/my_thai_font.c"

if [ ! -f "$LATIN_TTF" ]; then
  echo "Downloading Noto Sans (Latin)..."
  curl -sL -o "$LATIN_TTF" "$LATIN_URL"
fi
if [ ! -f "$THAI_TTF" ]; then
  echo "Downloading Noto Sans Thai..."
  curl -sL -o "$THAI_TTF" "$THAI_URL"
fi

echo "Generating my_thai_font.c (Latin + Thai)..."
npx --yes lv_font_conv \
  --font "$LATIN_TTF" -r 0x20-0x7E \
  --font "$THAI_TTF" -r 0x0E00-0x0E7F \
  --size 16 \
  --bpp 4 \
  --format lvgl \
  --lv-font-name my_thai_font \
  -o "$OUT_C"

echo "Done: $OUT_C"
echo "Then in my_thai_font.c:"
echo "  1. Replace the #include block with: #include <stddef.h> and #include \"lvgl.h\""
echo "  2. Change glyph_bitmap to use LV_ATTRIBUTE_LARGE_RAM_ARRAY instead of LV_ATTRIBUTE_LARGE_CONST (so font is in RAM and displays correctly on device)"
