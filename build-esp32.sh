#!/bin/bash
# ESP32-S3 Build Script for Adafruit Qualia RGB666 with TinyUF2 bootloader
# This uses arduino-cli to build the GraphicsController sketch

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build-esp32"
BOARD_FQBN="esp32:esp32:adafruit_qualia_s3_rgb666"
SKETCH_DIR="$SCRIPT_DIR/src/GraphicsController"

# Check for arduino-cli
if ! command -v arduino-cli &> /dev/null; then
    echo "Error: arduino-cli not found"
    echo "Install with: curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh"
    exit 1
fi

# Create build directory
mkdir -p "$BUILD_DIR"

echo "Building for Adafruit Qualia ESP32-S3 RGB666..."

# Build with arduino-cli using local core
# Speed optimizations: QIO 120MHz flash, 240MHz CPU, OPI PSRAM
arduino-cli compile \
  --fqbn "$BOARD_FQBN:FlashMode=qio120,CPUFreq=240,PSRAM=opi" \
  --build-path "$BUILD_DIR/output" \
  --build-property "build.partitions=tinyuf2-partitions-16MB" \
  --build-property "upload.maximum_size=2097152" \
  --build-property "compiler.cpp.extra_flags=-O3 -ffast-math -fno-rtti -fno-exceptions" \
  --build-property "compiler.c.extra_flags=-O3" \
  --build-property "compiler.ldflags=-O3"\
  "$SKETCH_DIR"


# Convert to UF2 format for TinyUF2 bootloader
# The UF2 family ID for ESP32-S3 is 0xc47e5767
# Use base address 0x0000 for bootloader compatibility
echo ""
echo "Converting to UF2 format..."

# Check if uf2conv is available, if not provide instructions
if command -v uf2conv &> /dev/null; then
    uf2conv "$BUILD_DIR/output/GraphicsController.ino.bin" \
        --family ESP32S3 \
        --base 0x0000 \
        --output "$BUILD_DIR/ej-mon.uf2"
elif [ -f "$SCRIPT_DIR/tools/uf2conv.py" ]; then
    python3 "$SCRIPT_DIR/tools/uf2conv.py" \
        "$BUILD_DIR/output/GraphicsController.ino.bin" \
        --family ESP32S3 \
        --base 0x0000 \
        --output "$BUILD_DIR/ej-mon.uf2"
else
    echo "Note: uf2conv not found. Binary built but not converted to UF2."
    echo "To convert manually, install uf2conv:"
    echo "  pip install uf2conv"
    echo ""
    echo "Then run:"
    echo "  uf2conv $BUILD_DIR/output/GraphicsController.ino.bin --family ESP32S3 -o $BUILD_DIR/ej-mon.uf2"
fi

echo ""
echo "Build complete!"
