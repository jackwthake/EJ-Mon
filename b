#!/bin/bash

FILENAME=$(mktemp)

# 2. Run the build, capturing all output to the file
# Note: Output is not printed live to terminal here.
./build-esp32.sh > "$FILENAME" 2>&1
BUILD_STATUS=$?

if [ $BUILD_STATUS -ne 0 ]; then
    # FAILURE: Print full traceback
    clear
    echo "--- 🛑 BUILD FAILED: FULL COMPILER TRACEBACK ---"
    cat "$FILENAME"
    rm -f \"$FILENAME\"
    exit 1
else
    # SUCCESS: Print only the filtered and colored summary
    echo "--- ✅ BUILD SUCCESS ---"
    grep -E 'Sketch|Global|Wrote.*bytes to|Build complete!' "$FILENAME" | grep --color=always '([0-9]+ bytes|\b[0-9]+%|Wrote|Sketch|Build complete)'
    tail -n 12 "$FILENAME"
    rm -f \"$FILENAME\"
fi