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

echo "Done!"
