#!/bin/bash

ICON_NAME="YandexHomeDesktop"
SVG_FILE="images/icon.svg"

mkdir -p ${ICON_NAME}.iconset

sizes=(16 32 64 128 256 512)

for size in "${sizes[@]}"; do
  # Default resolution
  rsvg-convert -w $size -h $size "$SVG_FILE" -o ${ICON_NAME}.iconset/icon_${size}x${size}.png

  # Retina (@2x) resolution
  retina_size=$((size * 2))
  rsvg-convert -w $retina_size -h $retina_size "$SVG_FILE" -o ${ICON_NAME}.iconset/icon_${size}x${size}@2x.png
done

echo "Done generating icons for ${ICON_NAME}.iconset"
echo "Now run: iconutil -c icns ${ICON_NAME}.iconset"
