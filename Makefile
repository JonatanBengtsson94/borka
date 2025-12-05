cc = gcc
CFLAGS = -Wall -Wextra -std=c99 \
				 -Iinclude \
				 -Isrc/engine \
				 -Isrc/game
VGFLAGS = --leak-check=full --track-origins=yes --show-leak-kinds=all

OUT_DIR = bin
OUT = $(OUT_DIR)/breakout

ENGINE_SRC = $(shell find src/engine -name '*.c' ! -path '*/platform/*')
GAME_SRC = $(shell find src/game -name '*.c')
SRC = $(ENGINE_SRC) $(GAME_SRC)

BUILD ?= debug
WINDOW_BACKEND ?= wayland
RENDER_BACKEND ?= software

# Build specific flags
ifeq ($(BUILD),debug)
	CFLAGS += -DBR_LOG_LEVEL_MIN=1 -g
else ifeq ($(BUILD),trace)
	CFLAGS += -DBR_LOG_LEVEL_MIN=0 -g
else ifeq ($(BUILD),release)
	CFLAGS += -DNDEBUG -DBR_LOG_LEVEL_MIN=3 -O3 -ffunction-sections -fdata-sections
	LDFLAGS += -Wl,--gc-sections -s
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

all: $(OUT) copy_assets

copy_assets:
	@mkdir -p $(OUT_DIR)/assets
	@cp -r assets/* $(OUT_DIR)/assets/

$(OUT) : $(SRC)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: $(OUT)
	./$(OUT)

valgrind:
	valgrind $(VGFLAGS) ./$(OUT)

clean:
	rm -f $(OUT)
