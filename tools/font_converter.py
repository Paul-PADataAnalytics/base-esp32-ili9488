#!/usr/bin/env python3
"""
TTF to GFXfont PROGMEM C Header Converter Tool
Converts TrueType/OpenType (.ttf / .otf) fonts into Adafruit GFX / LovyanGFX compatible
GFXfont structures for ESP32/microcontroller embedded graphics libraries.

Requires: freetype-py (or Pillow as fallback)
Usage:
    python3 font_converter.py <font.ttf> <size_pt> [first_char] [last_char] > OutputFont.h
"""

import sys
import os

def generate_gfx_font(font_path, font_size, first_char=32, last_char=126):
    try:
        import freetype
    except ImportError:
        print("// ERROR: freetype-py module not found. Install via 'pip install freetype-py'", file=sys.stderr)
        sys.exit(1)

    face = freetype.Face(font_path)
    face.set_char_size(font_size * 64)

    font_name = os.path.splitext(os.path.basename(font_path))[0].replace("-", "_").replace(" ", "_")
    struct_name = f"{font_name}_{font_size}pt"

    bitmaps = []
    glyphs = []
    bit_offset = 0

    for code in range(first_char, last_char + 1):
        face.load_char(chr(code), freetype.FT_LOAD_RENDER | freetype.FT_LOAD_TARGET_MONO)
        glyph = face.glyph
        bitmap = glyph.bitmap

        width = bitmap.width
        height = bitmap.rows
        advance = glyph.advance.x >> 6
        left = glyph.bitmap_left
        top = glyph.bitmap_top

        glyphs.append({
            'offset': len(bitmaps),
            'width': width,
            'height': height,
            'xAdvance': advance,
            'xOffset': left,
            'yOffset': -top
        })

        # Bitpack 1bpp mono bitmap
        buffer = bitmap.buffer
        pitch = bitmap.pitch
        
        byte_val = 0
        bit_count = 0

        for y in range(height):
            for x in range(width):
                byte_index = y * pitch + (x >> 3)
                bit_index = 7 - (x & 7)
                pixel = (buffer[byte_index] >> bit_index) & 1

                byte_val = (byte_val << 1) | pixel
                bit_count += 1

                if bit_count == 8:
                    bitmaps.append(byte_val)
                    byte_val = 0
                    bit_count = 0

        if bit_count > 0:
            byte_val <<= (8 - bit_count)
            bitmaps.append(byte_val)

    # Output C Header
    print("#ifndef _" + struct_name.upper() + "_H_")
    print("#define _" + struct_name.upper() + "_H_")
    print()
    print("#include <LovyanGFX.hpp>")
    print()
    print(f"// Generated from {os.path.basename(font_path)} ({font_size}pt)")
    print(f"const uint8_t {struct_name}Bitmaps[] PROGMEM = {{")
    
    for i in range(0, len(bitmaps), 12):
        chunk = bitmaps[i:i+12]
        hex_str = ", ".join([f"0x{b:02X}" for b in chunk])
        print(f"  {hex_str},")
    print("};")
    print()

    print(f"const lgfx::GFXglyph {struct_name}Glyphs[] PROGMEM = {{")
    for g in glyphs:
        print(f"  {{ {g['offset']:5d}, {g['width']:3d}, {g['height']:3d}, {g['xAdvance']:3d}, {g['xOffset']:4d}, {g['yOffset']:4d} }},")
    print("};")
    print()

    y_advance = int(font_size * 1.4)
    print(f"const lgfx::GFXfont {struct_name} PROGMEM = {{")
    print(f"  (uint8_t*){struct_name}Bitmaps,")
    print(f"  (lgfx::GFXglyph*){struct_name}Glyphs,")
    print(f"  0x{first_char:02X}, 0x{last_char:02X}, {y_advance}")
    print("};")
    print()
    print("#endif")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 font_converter.py <font.ttf> <size_pt> [first_char] [last_char]")
        sys.exit(1)

    ttf_path = sys.argv[1]
    size = int(sys.argv[2])
    first = int(sys.argv[3]) if len(sys.argv) > 3 else 32
    last = int(sys.argv[4]) if len(sys.argv) > 4 else 126

    generate_gfx_font(ttf_path, size, first, last)
