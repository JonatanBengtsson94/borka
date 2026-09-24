# Borka

**Borka** is a game engine written in C, with minimal external dependencies beyond the standard C library and platform APIs.

## Features

- Multithreaded logging system with compile-time log level filtering.
- PNG image decoding.
- DEFLATE decompression.
- Entity-Component-System (ECS) architecture.
- Bitmap font rendering with per-font character sets.
- Asynchronous audio playback with mixing and looping.

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
| `AUDIO_FORMATS` | space separated list of `wav`, `flac` | `flac` |

Only the audio formats listed in `AUDIO_FORMATS` are compiled in, so a game
that ships `.flac` assets links no `.wav` decoding code and vice versa. Set it
per game in `src/games/<game>.mk` to override the default.

### Other Makefile Targets

| Target | Description |
|---|---|
| `make` / `make all` | Build the selected `GAME` |
| `make run` | Build (if needed) and run the selected `GAME` |
| `make valgrind` | Build and run under Valgrind with leak-check enabled, using the suppressions in `valgrind.supp` |
| `make clean` | Remove build output for the current `BUILD` type |
| `make clean-all` | Remove all build output |

### Audio Assets

Audio has to be 8bit mono at 22050 Hz, matching `BR_AUDIO_BITS_PER_SAMPLE`,
`BR_AUDIO_CHANNELS` and `BR_AUDIO_SAMPLE_RATE` in `include/borka_audio.h`.
Anything else is rejected when the sound is loaded.

Author sounds as `.wav`, then encode them to `.flac` for shipping:

```sh
flac --best --no-padding --no-seektable -o sound.flac sound.wav
metaflac --remove --block-type=VORBIS_COMMENT --dont-use-padding sound.flac
```

### Texture Assets

Textures have to be 8bit RGBA PNGs without interlacing. The decoder checks the
IHDR and refuses anything else:

| IHDR field | Required | Rejected with |
|---|---|---|
| `bit_depth` | 8 | `Only 8-bit PNG supported` |
| `color_type` | 6 (RGBA) | `Only RGBA PNG supported` |
| `interlace_method` | 0 (none) | `Interlaced PNG not supported` |

RGBA is required even when the image is fully opaque, which is the easy one to
get wrong: editors commonly save small sprites as palette or plain RGB, and
those fail to load rather than being converted.

Only `IHDR`, `IDAT` and `IEND` are read and every other chunk is skipped, so
colour profiles, text and timestamps that an exporter adds are dead weight in
the shipped asset. Strip metadata on export, or afterwards with a tool such as
`optipng -strip all texture.png`.

### Font Assets

A font atlas is a texture (see Texture Assets) holding equally sized glyphs,
packed with no padding, left to right and row by row. The atlas can be a
single row or a grid. The game names the characters in atlas order when it
sets up the font:

```c
BrFont font;
br_font_init(&font, atlas, (BrVec2){8, 8}, (BrVec2){2, 2},
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
```

Characters not in the charset, including space unless it is listed, are left
blank. Text is monospace, so every character takes the same width.
`br_font_init` fails if the charset has more characters than the atlas has
glyphs.

### Adding a new game

1. Create `src/games/mygame` and add your source files there
2. Create `src/games/mygame.mk`:
```makefile
GAME_OUT = mygame
GAME_SRC = $(shell find src/games/mygame -name '*.c')
```
3. Add assets to `assets/mygame/`
4. Build with `make GAME=mygame`
