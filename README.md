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

### Quickstart

Build and run the default game (`breakout`):

```sh
make
make run
```

Build and run a specific example instead:

```sh
make GAME=audio
make run
```

### Example Games

A few example games are included to verify your build environment:

- `logging` - only exercises the logging system, no window/audio/renderer required
- `window` - opens a window and pumps events until closed, no renderer/audio/ECS
- `render` - opens a window and exercises every renderer primitive (filled/outlined rectangles, textures, texture regions, text), no audio/ECS
- `audio` - loads and plays a sound, no window/renderer/ECS
- `input` - opens a window and logs key press/release events, no audio/ECS
- `breakout` - a full game, test the renderer, audio and input systems

### Build Options

| Variable | Values | Default |
|---|---|---|
| `GAME` | `breakout`, `logging`, `window`, `render`, `audio`, `input` | `breakout` |
| `BUILD` | `debug`, `trace`, `release` | `debug` |
| `PLATFORM` | `linux`, `windows` | `linux` |
| `WINDOW_BACKEND` | `wayland`, `win32` | `wayland` |
| `RENDER_BACKEND` | `software` | `software` |

### Other Makefile Targets

| Target | Description |
|---|---|
| `make` / `make all` | Build the selected `GAME` |
| `make run` | Build (if needed) and run the selected `GAME` |
| `make valgrind` | Build and run under Valgrind with leak-check enabled, using the suppressions in `valgrind.supp` |
| `make clean` | Remove build output for the current `BUILD` type |
| `make clean-all` | Remove all build output |

### Adding a new game

1. Create `src/games/mygame` and add your source files there
2. Create `games/mygame.mk`:
```makefile
GAME_OUT = mygame
GAME_SRC = $(shell find src/games/mygame -name '*.c')
```
3. Add assets to `assets/mygame/`
4. Build with `make GAME=mygame`
