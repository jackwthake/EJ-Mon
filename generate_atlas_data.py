#!/usr/bin/env python3
"""
Convert BMP image to RGB565 format and generate atlas_data.h header file
"""

import struct
import sys
from pathlib import Path

def read_bmp(filepath):
    """Read BMP file and return pixel data as RGB tuples"""
    with open(filepath, 'rb') as f:
        # Read BMP header
        header = f.read(54)
        
        if header[0:2] != b'BM':
            raise ValueError(f"Not a BMP file: {filepath}")
        
        # Parse header
        width = struct.unpack('<I', header[18:22])[0]
        height = struct.unpack('<I', header[22:26])[0]
        bpp = struct.unpack('<H', header[28:30])[0]
        compression = struct.unpack('<I', header[30:34])[0]
        
        if compression != 0:
            raise ValueError("Compressed BMPs not supported")
        
        if bpp not in (24, 32):
            raise ValueError(f"Unsupported BMP format: {bpp} bpp")
        
        bytes_per_pixel = bpp // 8
        
        # Calculate row size (padded to 4 bytes)
        row_size = ((width * bytes_per_pixel + 3) // 4) * 4
        
        # Read pixel data (BMP is stored bottom-up)
        pixels = []
        for y in range(height - 1, -1, -1):
            row_data = f.read(row_size)
            row_pixels = []
            for x in range(width):
                offset = x * bytes_per_pixel
                b = row_data[offset]
                g = row_data[offset + 1]
                r = row_data[offset + 2]
                row_pixels.append((r, g, b))
            pixels.append(row_pixels)
        
        return width, height, pixels

def rgb_to_rgb565(r, g, b):
    """Convert RGB888 to RGB565"""
    r5 = (r >> 3) & 0x1F
    g6 = (g >> 2) & 0x3F
    b5 = (b >> 3) & 0x1F
    return (r5 << 11) | (g6 << 5) | b5

def generate_header(width, height, pixels, output_path):
    """Generate atlas_data.h header file"""
    
    # Flatten pixel array
    rgb565_data = []
    for row in pixels:
        for r, g, b in row:
            rgb565_data.append(rgb_to_rgb565(r, g, b))
    
    pixel_count = len(rgb565_data)
    byte_count = pixel_count * 2
    
    # Generate header file
    output = ""
    output += "// Auto-generated from atlas.bmp\n"
    output += f"// Size: {width}x{height} pixels, RGB565 format\n"
    output += f"// Total size: {byte_count} bytes\n"
    output += "\n"
    output += "#pragma once\n"
    output += "\n"
    output += "#include <stdint.h>\n"
    output += "\n"
    output += f"constexpr int ATLAS_DATA_WIDTH = {width};\n"
    output += f"constexpr int ATLAS_DATA_HEIGHT = {height};\n"
    output += "\n"
    output += "#if defined(ESP32) || defined(ARDUINO)\n"
    output += "#include <pgmspace.h>\n"
    output += "// Store in flash - PROGMEM keeps data out of DRAM\n"
    output += f"const uint16_t atlas_data[{pixel_count}] PROGMEM = {{\n"
    output += "#else\n"
    output += f"static const uint16_t atlas_data[{pixel_count}] = {{\n"
    output += "#endif\n"
    
    # Write pixel data in rows of 16 values
    hex_values = []
    for val in rgb565_data:
        hex_values.append("0x{:04X}".format(val))
    
    # Write 16 values per line
    for i in range(0, len(hex_values), 16):
        chunk = hex_values[i:i+16]
        line = "  " + ", ".join(chunk)
        if i + 16 < len(hex_values):
            line += ","
        output += line + "\n"
    
    output += "};\n"
    output += "\n"
    
    # Write to file
    with open(output_path, 'w') as f:
        f.write(output)
    
    return pixel_count

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: generate_atlas_data.py <input.bmp> <output.h>")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    try:
        width, height, pixels = read_bmp(input_file)
        pixel_count = generate_header(width, height, pixels, output_file)
        print(f"✓ Generated {output_file} ({width}x{height}, {pixel_count} pixels, {pixel_count * 2} bytes)")
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)
