# Resource Definition File
# This file defines all graphics, sprites, sounds, and music resources
# The SGDK resource compiler (rescomp) processes this file
# Format: RESOURCETYPE name "path/to/file" [compression] [options]

# Sprites
# Crosshair is 16x16 (2x2 tiles)
SPRITE sprite_crosshair "sprites/crosshair.png" 2 2 FAST 0

# Cannon is 16x16 (2x2 tiles)
SPRITE sprite_cannon "sprites/cannon.png" 2 2 FAST 0

# Snowball missile is 8x8 (1x1 tile)
SPRITE sprite_snowball "sprites/snowball.png" 1 1 FAST 0

# Enemy plane is 16x16 (2x2 tiles) - reusing cannon sprite for now
SPRITE sprite_plane "sprites/cannon.png" 2 2 FAST 0

# Bomb is 8x8 (1x1 tile) - reusing snowball sprite for now
SPRITE sprite_bomb "sprites/snowball.png" 1 1 FAST 0

# Igloo is 16x16 (2x2 tiles) - reusing cannon sprite for now
SPRITE sprite_igloo "sprites/cannon.png" 2 2 FAST 0

# Background image - removed for now
# IMAGE bg_test "images/test-bg.png" FAST
