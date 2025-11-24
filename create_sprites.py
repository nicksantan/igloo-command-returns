from PIL import Image, ImageDraw

# SGDK requires images to be at least 128x32 pixels to store palette data
# We'll create 128x32 images with the sprite in the top-left corner

# Create 128x32 plane sprite (actual sprite is 16x16 in top-left)
plane = Image.new('RGB', (128, 32), (255, 0, 255))  # Magenta background (transparent color)
draw = ImageDraw.Draw(plane)

# Plane body (gray) - positioned in top-left 16x16 area
draw.rectangle([4, 6, 12, 9], fill=(128, 128, 128))
# Cockpit (darker gray)
draw.rectangle([9, 5, 11, 6], fill=(64, 64, 64))
# Wings
draw.rectangle([2, 7, 14, 8], fill=(96, 96, 96))
# Tail
draw.rectangle([4, 5, 6, 7], fill=(96, 96, 96))
# Engine detail (dark)
draw.rectangle([12, 7, 13, 8], fill=(32, 32, 32))

plane.save('res/sprites/plane.png')
print("Created plane.png (128x32, sprite at top-left)")

# Create 128x32 bomb sprite (actual sprite is 8x8 in top-left)
bomb = Image.new('RGB', (128, 32), (255, 0, 255))  # Magenta background
draw = ImageDraw.Draw(bomb)

# Bomb body (black oval)
draw.ellipse([1, 2, 6, 6], fill=(32, 32, 32))
# Tail fins (gray)
draw.rectangle([2, 1, 5, 2], fill=(128, 128, 128))
# Highlight
draw.rectangle([2, 3, 3, 4], fill=(64, 64, 64))

bomb.save('res/sprites/bomb.png')
print("Created bomb.png (128x32, sprite at top-left)")

# Create 128x32 igloo sprite (actual sprite is 16x16 in top-left)
igloo = Image.new('RGB', (128, 32), (255, 0, 255))  # Magenta background
draw = ImageDraw.Draw(igloo)

# Igloo dome (white with shading)
draw.ellipse([2, 6, 13, 15], fill=(240, 240, 255))
# Shading lines (light blue)
draw.arc([2, 6, 13, 15], 180, 0, fill=(200, 220, 255))
# Ice blocks outline
draw.rectangle([3, 8, 12, 9], fill=(180, 200, 220))
draw.rectangle([3, 10, 12, 11], fill=(180, 200, 220))
# Door (dark blue/black)
draw.rectangle([6, 11, 9, 15], fill=(20, 40, 80))
# Door arch top
draw.ellipse([6, 10, 9, 13], fill=(20, 40, 80))

igloo.save('res/sprites/igloo.png')
print("Created igloo.png (128x32, sprite at top-left)")

print("\nAll sprites created successfully!")
