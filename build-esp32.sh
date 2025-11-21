#!/bin/bash
# ESP32-S3 Build Script for Adafruit Qualia RGB666 with TinyUF2 bootloader
# This uses arduino-cli to build with the Arduino-ESP32 core

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build-esp32"
ARDUINO_DIR="$SCRIPT_DIR/third-party/arduino"
BOARD_FQBN="esp32:esp32:adafruit_qualia_s3_rgb666"

# Check for arduino-cli
if ! command -v arduino-cli &> /dev/null; then
    echo "Error: arduino-cli not found"
    echo "Install with: curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh"
    exit 1
fi

# Create build directory
mkdir -p "$BUILD_DIR"

# Create a temporary sketch directory structure Arduino expects
SKETCH_DIR="$BUILD_DIR/ej-mon"
mkdir -p "$SKETCH_DIR"

# Create the .ino file with platform definitions and main code
{
    # Add platform defines
    echo "#define PLATFORM_DESKTOP 0"
    echo "#define PLATFORM_ESP32 1"
    echo ""
    echo "#define IF_DESKTOP(x)"
    echo "#define IF_ESP32(x) x"
    echo ""
    echo "constexpr int DISPLAY_WIDTH = 960;"
    echo "constexpr int DISPLAY_HEIGHT = 320;"
    echo ""

    # Check if the file has platform guards
    if grep -q "^#if PLATFORM_ESP32\|^// #if PLATFORM_ESP32" "$SCRIPT_DIR/src/main.cpp"; then
        # Extract code between guards, removing platform.hpp include
        sed -n '/#if PLATFORM_ESP32/,/#endif/p' "$SCRIPT_DIR/src/main.cpp" | \
            sed '1d;$d' | \
            grep -v '#include "platform.hpp"'
    else
        # No guards, use entire file but skip Arduino.h includes that might conflict
        cat "$SCRIPT_DIR/src/main.cpp" | \
            grep -v '#include "platform.hpp"'
    fi
} > "$SKETCH_DIR/ej-mon.ino"

echo "Building for Adafruit Qualia ESP32-S3 RGB666..."

# Build with arduino-cli using local core
# Speed optimizations: QIO 120MHz flash, 240MHz CPU, OPI PSRAM
arduino-cli compile \
    --fqbn "$BOARD_FQBN:FlashMode=qio120,CPUFreq=240,PSRAM=opi" \
    --build-path "$BUILD_DIR/output" \
    --libraries "$ARDUINO_DIR/libraries" \
    --build-property "build.partitions=tinyuf2-partitions-16MB" \
    --build-property "upload.maximum_size=2097152" \
    --build-property "compiler.cpp.extra_flags=-O3" \
    --build-property "compiler.c.extra_flags=-O3" \
    "$SKETCH_DIR"

# Convert to UF2 format for TinyUF2 bootloader
# The UF2 family ID for ESP32-S3 is 0xc47e5767
# Use base address 0x0000 for bootloader compatibility
echo ""
echo "Converting to UF2 format..."

# Check if uf2conv is available, if not provide instructions
if command -v uf2conv &> /dev/null; then
    uf2conv "$BUILD_DIR/output/ej-mon.ino.bin" \
        --family ESP32S3 \
        --base 0x0000 \
        --output "$BUILD_DIR/ej-mon.uf2"
elif [ -f "$SCRIPT_DIR/tools/uf2conv.py" ]; then
    python3 "$SCRIPT_DIR/tools/uf2conv.py" \
        "$BUILD_DIR/output/ej-mon.ino.bin" \
        --family ESP32S3 \
        --base 0x0000 \
        --output "$BUILD_DIR/ej-mon.uf2"
else
    echo "Note: uf2conv not found. Binary built but not converted to UF2."
    echo "To convert manually, install uf2conv:"
    echo "  pip install uf2conv"
    echo ""
    echo "Then run:"
    echo "  uf2conv $BUILD_DIR/output/ej-mon.ino.bin --family ESP32S3 -o $BUILD_DIR/ej-mon.uf2"
fi

echo ""
echo "Build complete!"
echo ""
echo "To flash via TinyUF2 bootloader:"
echo "  1. Double-tap the reset button on the Qualia S3"
echo "  2. A USB drive named 'QUALIAS3BOOT' will appear"
echo "  3. Copy $BUILD_DIR/ej-mon.uf2 to the drive"
echo "  4. The board will automatically reset and run the firmware"
echo ""
echo "To monitor serial output:"
echo "  screen /dev/ttyACM0 115200"
