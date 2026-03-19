# Borka

**Borka** is a game engine written in C, with minimal external dependencies beyond the standard C library and platform APIs.

## Features

- Multithreaded logging system with compile-time log level filtering.
- PNG image decoding.
- DEFLATE decompression.
- Entity-Component-System (ECS) architecture.
- Bitmap font rendering.
- Asynchronous audio playback.

## Supported Platforms

### Linux

- **Windowing** Wayland
- **Rendering** Software renderer

## Local Development

### Prerequisites

- gcc
- make

### Example Games

Two example games are included to verify your build environment:

- `hello_world` - creates a window and does basic logging, good first sanity check
- `breakout` - a full game, test the renderer, audio and input systems

### Build Options

| Variable | Values | Default |
|---|---|---|
| `GAME` | `breakout`, `hello_world` | `breakout` |
| `BUILD` | `debug`, `trace`, `release` | `debug` |
| `PLATFORM` | `linux`, `windows` | `linux` |
| `WINDOW_BACKEND` | `wayland`, `win32` | `wayland` |
| `RENDER_BACKEND` | `software` | `software` |

### Adding a new game

1. Create `src/games/mygame` and add your source files there
2. Create `games/mygame.mk`:
```makefile
GAME_OUT = mygame
GAME_SRC = $(shell find src/games/mygame -name '*.c')
```
3. Add assets to `assets/mygame/`
4. Build with `make GAME=mygame`
