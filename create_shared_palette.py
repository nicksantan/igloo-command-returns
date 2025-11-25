#!/usr/bin/env python3
"""
Create a shared palette from multiple sprite images and remap them to use it.
This ensures all sprites can share PAL1 with their proper colors.
"""

from PIL import Image
import os

def collect_colors_from_sprites():
    """Collect all unique colors from sprite files"""
    sprite_files = [
        'res/sprites/crosshair.png',
        'res/sprites/cannon.png',
        'res/sprites/snowball.png',
        'res/sprites/polarbear.png'
    ]

    all_colors = set()
    all_colors.add((0, 0, 0))  # Always include transparent/black as first color

    for sprite_file in sprite_files:
        if not os.path.exists(sprite_file):
            print(f"Warning: {sprite_file} not found, skipping")
            continue

        img = Image.open(sprite_file)

        # Convert to RGB if needed
        if img.mode == 'P':
            img = img.convert('RGBA')
        elif img.mode != 'RGBA' and img.mode != 'RGB':
            img = img.convert('RGBA')

        # Collect unique colors
        pixels = img.getdata()
        for pixel in pixels:
            if len(pixel) == 4:  # RGBA
                r, g, b, a = pixel
                if a > 127:  # Not transparent
                    all_colors.add((r, g, b))
            else:  # RGB
                all_colors.add(pixel[:3])

    return list(all_colors)

def create_palette(colors):
    """Create a 16-color palette (Genesis limitation)"""
    # Ensure black/transparent is first
    palette_colors = [(0, 0, 0)]

    # Add other colors (skip duplicate black)
    for color in colors:
        if color != (0, 0, 0) and len(palette_colors) < 16:
            palette_colors.append(color)

    # Pad to 16 colors with black if needed
    while len(palette_colors) < 16:
        palette_colors.append((0, 0, 0))

    return palette_colors[:16]

def remap_sprite_to_palette(sprite_path, palette_colors, output_path=None):
    """Remap a sprite to use the shared palette"""
    if output_path is None:
        output_path = sprite_path

    img = Image.open(sprite_path)

    # Convert to RGBA if needed
    if img.mode == 'P':
        img = img.convert('RGBA')
    elif img.mode != 'RGBA':
        img = img.convert('RGBA')

    # Create new paletted image
    new_img = Image.new('P', img.size)

    # Set the palette (need to flatten RGB tuples to flat list)
    flat_palette = []
    for r, g, b in palette_colors:
        flat_palette.extend([r, g, b])
    new_img.putpalette(flat_palette)

    # Remap pixels
    pixels = img.load()
    new_pixels = new_img.load()

    for y in range(img.height):
        for x in range(img.width):
            pixel = pixels[x, y]

            if len(pixel) == 4:
                r, g, b, a = pixel
                if a < 128:  # Transparent
                    new_pixels[x, y] = 0  # Use index 0 (transparent)
                else:
                    # Find closest color in palette
                    color = (r, g, b)
                    if color in palette_colors:
                        new_pixels[x, y] = palette_colors.index(color)
                    else:
                        # Find closest color
                        min_dist = float('inf')
                        best_idx = 0
                        for idx, pal_color in enumerate(palette_colors):
                            if idx == 0:  # Skip transparent
                                continue
                            dist = sum((a - b) ** 2 for a, b in zip(color, pal_color))
                            if dist < min_dist:
                                min_dist = dist
                                best_idx = idx
                        new_pixels[x, y] = best_idx
            else:
                r, g, b = pixel[:3]
                color = (r, g, b)
                if color in palette_colors:
                    new_pixels[x, y] = palette_colors.index(color)
                else:
                    new_pixels[x, y] = 1  # Default to white

    new_img.save(output_path)
    print(f"Remapped {sprite_path} -> {output_path}")

def main():
    print("Collecting colors from all sprites...")
    colors = collect_colors_from_sprites()
    print(f"Found {len(colors)} unique colors")

    print("\nCreating shared 16-color palette...")
    palette = create_palette(colors)

    print("\nShared palette (16 colors):")
    for idx, color in enumerate(palette):
        print(f"  {idx:2d}: RGB{color}")

    if len(colors) > 16:
        print(f"\nWARNING: Found {len(colors)} colors, but Genesis only supports 16 per palette!")
        print("Some colors will be approximated to the nearest palette color.")

    print("\nRemapping sprites to use shared palette...")
    sprites = [
        'res/sprites/crosshair.png',
        'res/sprites/cannon.png',
        'res/sprites/snowball.png',
        'res/sprites/polarbear.png'
    ]

    for sprite in sprites:
        if os.path.exists(sprite):
            remap_sprite_to_palette(sprite, palette)

    print("\nDone! All sprites now use the shared palette.")
    print("You can now rebuild your project.")

if __name__ == '__main__':
    main()
