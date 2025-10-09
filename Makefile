cc = gcc
CFLAGS = -pthread -Wall -std=c99 -Isrc/engine -Isrc/game

OUT_DIR = bin
OUT = $(OUT_DIR)/breakout

ENGINE_SRC = $(shell find src/engine -name '*.c')
GAME_SRC = src/game/main.c
SRC = $(ENGINE_SRC) $(GAME_SRC)

BUILD ?= debug

ifeq ($(BUILD),debug)
	CFLAGS += -DLOG_LEVEL_MIN=0 -g
else ifeq ($(BUILD),release)
	CFLAGS += -DLOG_LEVEL_MIN=2 -O2
endif

all: $(OUT)

$(OUT) : $(SRC)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $^

run: $(OUT)
	./$(OUT)

clean:
	rm -f $(OUT)
