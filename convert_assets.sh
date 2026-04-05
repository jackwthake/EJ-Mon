#!/bin/bash
# Convert PNG assets to BMP format for embedded use

echo "Converting PNG assets to BMP..."

# Convert atlas (flatten transparency to black)
if [ -f "res/atlas.png" ]; then
  convert res/atlas.png -background black -alpha remove -alpha off -type truecolor BMP3:res/atlas.bmp
  echo "✓ Converted atlas.png -> atlas.bmp (transparency -> black)"
fi

# Convert splash (flatten transparency to black)
if [ -f "res/splash.png" ]; then
  convert res/splash.png -background black -alpha remove -alpha off -type truecolor BMP3:res/splash.bmp
  echo "✓ Converted splash.png -> splash.bmp (transparency -> black)"
fi

# Generate atlas_data.h from atlas.bmp
if [ -f "res/atlas.bmp" ]; then
  python3 generate_atlas_data.py res/atlas.bmp src/GraphicsController/atlas_data.h
fi

# Generate splash_data.h from splash.bmp
if [ -f "res/splash.bmp" ]; then
  python3 generate_atlas_data.py res/splash.bmp src/GraphicsController/splash_data.h
fi


echo "Done!"
