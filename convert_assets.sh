#!/bin/bash
# Convert PNG assets to BMP format for embedded use

echo "Converting PNG assets to BMP..."

# Convert atlas (flatten transparency to black)
if [ -f "res/atlas.png" ]; then
  convert res/atlas.png -background black -alpha remove -alpha off -type truecolor BMP3:res/atlas.bmp
  echo "✓ Converted atlas.png -> atlas.bmp (transparency -> black)"
fi

# Convert background (flatten transparency to black)
if [ -f "res/background.png" ]; then
  convert res/background.png -background black -alpha remove -alpha off -type truecolor BMP3:res/background.bmp
  echo "✓ Converted background.png -> background.bmp (transparency -> black)"
fi

# Generate atlas_data.h from atlas.bmp
if [ -f "res/atlas.bmp" ]; then
  python3 generate_atlas_data.py res/atlas.bmp src/GraphicsController/atlas_data.h
fi

echo "Done!"
