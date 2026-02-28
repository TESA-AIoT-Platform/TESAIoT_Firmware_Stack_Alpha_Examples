
In **LVGL v9**, getting a Thai font to *show Thai characters* is straightforward — but getting **correct Thai mark positioning** (vowels + tone marks stacking) is the hard part.

## 1) Make Thai characters render at all (glyph coverage)

LVGL expects **UTF-8 text** in labels (e.g. `lv_label_set_text(label, "สวัสดี");`) and you must use a font that contains Thai glyphs.

### Option A — Generate a C font (offline)

Use LVGL’s font converter workflow and **include the Thai Unicode range**:

* Thai block: **U+0E00–U+0E7F**
* You can also specify exact characters you need.

LVGL docs overview the font system and how fonts are applied via styles. ([docs.lvgl.io][1])

### Option B — Use FreeType (load TTF/OTF at runtime)

If you can afford FreeType, LVGL has a FreeType font engine integration for runtime font rasterization. ([docs.lvgl.io][2])

(Your snippet will look similar to what people post in the Thai-thread: `lv_freetype_init(...)`, `lv_ft_font_init(&info)`, then `lv_style_set_text_font(&style, info.font)`.) ([LVGL Forum][3])

### Optional — Fallback font

LVGL supports a **fallback chain**: if a glyph is missing in your main font, it tries the fallback font. ([docs.lvgl.io][4])

## 2) The real issue: Thai shaping/stacking marks

If you see tone marks/vowels **overlapping or placed wrong** (e.g. “ทิ๋” looking broken), that’s typically because Thai needs **text shaping / mark positioning** (often via OpenType tables like GPOS), and **LVGL’s built-in text handling doesn’t fully cover complex Thai shaping**.

This is a known pain point discussed by LVGL devs/users (they explicitly mention HarfBuzz / shaping engines as the missing piece). ([LVGL Forum][3])

## 3) Practical workarounds (pick what fits your device)

1. **Try different Thai fonts**
   Some fonts “look acceptable” even with limited shaping (depends on how much they rely on GPOS). This is the cheapest experiment.

2. **Do shaping outside LVGL**
   If you need correctness, shape Thai text with a shaping engine (commonly **HarfBuzz**; some mention compiling with `HB_TINY` to reduce size) and then render using the shaped glyph positions. ([LVGL Forum][3])
   *(This requires custom integration work.)*

3. **Pre-render text to bitmap**
   For fixed UI strings (menus, labels), render Thai text offline to bitmaps/sprites and show them as images. It’s crude but reliable.

---

[1]: https://docs.lvgl.io/9.0/overview/font.html "Fonts — LVGL documentation"
[2]: https://docs.lvgl.io/master/libs/font_support/freetype.html "FreeType Font Engine - LVGL 9.5 documentation"
[3]: https://forum.lvgl.io/t/thai-cannot-be-fully-displayed/12430 "Thai cannot be fully displayed - How-to - LVGL Forum"
[4]: https://docs.lvgl.io/9.2/overview/font.html "Fonts — LVGL documentation"
