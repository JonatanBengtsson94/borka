GAME_OUT = audio
GAME_SRC = $(shell find src/games/audio -name '*.c')

# Exercise both format decoders, unlike the default (flac-only) build.
AUDIO_FORMATS = wav flac
