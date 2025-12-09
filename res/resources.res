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

# Enemy plane is 24x16 (3x2 tiles)
SPRITE sprite_plane "sprites/sm-enemy.png" 3 2 FAST 0

# Large enemy plane is 40x24 (5x3 tiles)
SPRITE sprite_plane_large "sprites/enemy-lg.png" 5 3 FAST 0

# Large enemy plane hurt state is 40x24 (5x3 tiles)
SPRITE sprite_plane_large_hurt "sprites/enemy-lg-hurt.png" 5 3 FAST 0

# Bomb is 8x16 (1x2 tiles)
SPRITE sprite_bomb "sprites/bomb-new-bigger.png" 1 2 FAST 0

# Igloo is 16x16 (2x2 tiles) - reusing cannon sprite for now
SPRITE sprite_igloo "sprites/cannon.png" 2 2 FAST 0

# Polar bear is 24x16 (3x2 tiles) - using actual sprite
SPRITE sprite_polarbear "sprites/polarbear.png" 3 2 FAST 0

# Explosion is 16x16 (2x2 tiles)
SPRITE sprite_explosion "sprites/explosion.png" 2 2 FAST 0

# Music
XGM bgm_music "music/test.vgm" -1

# Background image - removed for now
# IMAGE bg_test "images/test-bg.png" FAST
