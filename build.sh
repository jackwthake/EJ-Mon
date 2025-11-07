#!/bin/bash

# Build script with platform selection
# Usage:
#   ./build.sh [PLATFORM] [BUILD_TYPE]
#   ./build.sh                    # Desktop Debug (default)
#   ./build.sh desktop            # Desktop Debug
#   ./build.sh desktop Release    # Desktop Release
#   ./build.sh esp32              # ESP32 Debug (when implemented)

PLATFORM="${1:-desktop}"
BUILD_TYPE="${2:-Debug}"

# Normalize platform name
case "${PLATFORM,,}" in
  desktop|d)
    TARGET_PLATFORM="DESKTOP"
    BUILD_DIR="build-desktop"
    ;;
  esp32|esp|e)
    TARGET_PLATFORM="ESP32"
    BUILD_DIR="build-esp32"
    ;;
  *)
    echo "Error: Unknown platform '${PLATFORM}'"
    echo "Valid platforms: desktop, esp32"
    exit 1
    ;;
esac

echo "Building for: $TARGET_PLATFORM ($BUILD_TYPE)"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake .. \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
  -DTARGET_PLATFORM="$TARGET_PLATFORM" \
  -G "Unix Makefiles"

cmake --build . -j$(nproc)

echo ""
if [ "$TARGET_PLATFORM" = "DESKTOP" ]; then
  echo "Build complete! Run with: ./$BUILD_DIR/ej-mon"
else
  echo "Build complete! Flash with: cd $BUILD_DIR && make flash"
fi
