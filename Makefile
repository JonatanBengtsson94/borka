cc = gcc
CFLAGS = -pthread -Wall -Wextra -std=c99 -Isrc/engine -Isrc/game

OUT_DIR = bin
OUT = $(OUT_DIR)/breakout

ENGINE_SRC = $(shell find src/engine -name '*.c' ! -path '*/platform/*')
GAME_SRC = src/game/main.c
SRC = $(ENGINE_SRC) $(GAME_SRC)

BUILD ?= debug
WINDOW_BACKEND ?= wayland
RENDER_BACKEND ?= software

# Build specific flags
ifeq ($(BUILD),debug)
	CFLAGS += -DLOG_LEVEL_MIN=0 -g
else ifeq ($(BUILD),release)
	CFLAGS += -DLOG_LEVEL_MIN=2 -O2
endif

# Window backend flags
ifeq ($(WINDOW_BACKEND),wayland)
	CFLAGS += -DWINDOW_BACKEND_WAYLAND
	LDFLAGS += -lwayland-client
	SRC += src/engine/window/platform/wayland/*.c

	ifeq ($(RENDER_BACKEND),software)
		CFLAGS += -DRENDER_BACKEND_SOFTWARE
		SRC += src/engine/renderer/software/br_software_renderer.c
		SRC += src/engine/renderer/software/platform/wayland/*.c
	endif
endif

all: $(OUT)

$(OUT) : $(SRC)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: $(OUT)
	./$(OUT)

clean:
	rm -f $(OUT)
