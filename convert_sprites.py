#!/usr/bin/env python3
"""Convert RGBA PNGs to indexed color PNGs for SGDK"""

from PIL import Image
import sys

sprites = ['res/sprites/cannon.png', 'res/sprites/snowball.png', 'res/images/test-bg.png']

for sprite_path in sprites:
    try:
        img = Image.open(sprite_path)
        # Convert to indexed color (palette mode)
        img_indexed = img.convert('P', palette=Image.ADAPTIVE, colors=16)
        img_indexed.save(sprite_path)
        print(f"Converted {sprite_path} to indexed color mode")
    except Exception as e:
        print(f"Error converting {sprite_path}: {e}")
