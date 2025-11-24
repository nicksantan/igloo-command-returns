# Missile Command - Sega Genesis

A Missile Command style game for the Sega Genesis, built with SGDK (Sega Genesis Development Kit).

## Prerequisites

1. **SGDK** - Download and install from https://github.com/Stephane-D/SGDK
   - Windows: Extract to `C:\sgdk` or set `SGDK` environment variable
   - Linux/Mac: Extract to `~/sgdk` or `/opt/sgdk` and set `GDK` environment variable

2. **Emulator** (for testing)
   - Gens/GS: http://segaretro.org/Gens/GS
   - Blastem: https://www.retrodev.com/blastem/
   - Fusion: https://segaretro.org/Gens#Fusion

## Project Structure

```
sega-experiment/
├── Makefile          # Build configuration
├── src/              # C source files
│   └── main.c       # Main entry point
├── inc/              # Header files
├── res/              # Resources
│   ├── resources.res # Resource definition file
│   ├── sprites/     # Sprite images
│   ├── images/      # Background/tileset images
│   ├── sounds/      # Sound effects
│   └── music/       # Music files
└── out/              # Build output (ROM files)
```

## Building

1. Make sure `SGDK` environment variable is set:
   ```bash
   # Windows
   set SGDK=C:\sgdk

   # Linux/Mac
   export GDK=/path/to/sgdk
   ```

2. Build the project:
   ```bash
   make
   ```

3. The ROM file will be in the `out/` directory: `missile_command.bin`

## Running

Open the generated ROM file in your emulator:
```bash
# Example with Gens
gens out/missile_command.bin
```

## Cleaning Build Files

```bash
make clean
```

## Development Notes

- Main game loop is in [src/main.c](src/main.c)
- Game runs at 60 FPS (NTSC) or 50 FPS (PAL)
- Available RAM: 64KB
- Available sprites: 80 (max 20 per scanline)
- Screen resolution: 320x224 pixels
- Color limit: 64 colors on screen (4 palettes × 16 colors)

## Current Status

- [x] Project structure setup
- [x] Hello World test program
- [ ] Game design and architecture
- [ ] Graphics assets
- [ ] Core gameplay implementation
- [ ] Sound effects
- [ ] Polish and testing
