CC = gcc
CFLAGS = -Wall -Wextra -std=c99 \
				 -Iinclude \
				 -Isrc/engine \
				 -Isrc/games/$(GAME)
LDFLAGS =
VGFLAGS = --leak-check=full --track-origins=yes --show-leak-kinds=all

# Configuration
BUILD ?= debug
WINDOW_BACKEND ?= wayland
RENDER_BACKEND ?= software
PLATFORM ?= linux
GAME ?= breakout
COMPILING_PLATFORM ?= linux

# Include game-specific config
include src/games/$(GAME).mk

# Output dirs
OUT_DIR = bin/$(BUILD)
OUT = $(OUT_DIR)/$(GAME_OUT)

# Precompiled header
PCH = include/pch.h
PCH_GCH = $(PCH).gch

# Source files
ifeq ($(COMPILING_PLATFORM),linux)
	ENGINE_SRC = $(shell find src/engine -name '*.c' ! -path '*/platform/*')
else ifeq ($(COMPILING_PLATFORM),windows)
	ENGINE_SRC = $(wildcard src/engine/**/*.c)
endif
SRC = $(ENGINE_SRC) $(GAME_SRC)

# Build specific flags
ifeq ($(BUILD),debug)
	CFLAGS += -DBR_LOG_LEVEL_MIN=1 -g
else ifeq ($(BUILD),trace)
	CFLAGS += -DBR_LOG_LEVEL_MIN=0 -g
else ifeq ($(BUILD),release)
	CFLAGS += -DNDEBUG -DBR_LOG_LEVEL_MIN=3 -O3 -flto -ffunction-sections -fdata-sections
	LDFLAGS += -Wl,--gc-sections -s
endif

# Platform configuration
ifeq ($(PLATFORM),linux)
	SRC += $(wildcard src/engine/logger/platform/linux/*.c)
	SRC += $(wildcard src/engine/audio/platform/linux/*.c)
	LDFLAGS += -lm -lasound
endif

# Window backend configuration 
ifeq ($(WINDOW_BACKEND),wayland)
	CFLAGS += -DWINDOW_BACKEND_WAYLAND -D_POSIX_C_SOURCE=199309L
	LDFLAGS += -lwayland-client
	SRC += $(wildcard src/engine/window/platform/wayland/*.c)

	ifeq ($(RENDER_BACKEND),software)
		CFLAGS += -DRENDER_BACKEND_SOFTWARE
		SRC += src/engine/renderer/software/br_software_renderer.c
		SRC += $(wildcard src/engine/renderer/software/platform/wayland/*.c)
	endif
endif

# Game-specific libs
LDFLAGS += -lm

# Object files
OBJ_DIR = $(OUT_DIR)/obj
OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o,$(SRC))

.PHONY: all run valgrind clean clean-all copy_assets


all: $(OUT) copy_assets

$(PCH_GCH): $(PCH)
	$(CC) $(CFLAGS) -x c-header $< -o $@

$(OBJ_DIR)/%.o: %.c $(PCH_GCH)
ifeq ($(COMPILING_PLATFORM),windows)
	@mkdir $(subst /,\,$(dir $@)) 2>nul & exit 0
else
	@mkdir -p $(dir $@)
endif
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT): $(OBJ)
ifeq ($(COMPILING_PLATFORM),windows)
	@mkdir $(subst /,\,$(OUT_DIR)) 2>nul & exit 0
else
	@mkdir -p $(OUT_DIR)
endif
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

copy_assets:
	@mkdir -p $(OUT_DIR)/assets
	@cp -r assets/$(GAME)/* $(OUT_DIR)/assets/

run: $(OUT)
	cd $(OUT_DIR) && ./$(GAME_OUT)

valgrind: $(OUT)
	valgrind $(VGFLAGS) ./$(OUT)

clean:
	rm -rf $(OUT_DIR) $(PCH_GCH)

clean-all:
	rm -rf bin/ $(PCH_GCH)
