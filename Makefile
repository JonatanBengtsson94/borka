cc = gcc
CFLAGS = -Wall -std=c99 -Isrc/engine -Isrc/game

OUT_DIR = bin
OUT = $(OUT_DIR)/breakout

ENGINE_SRC = src/engine/engine.c
GAME_SRC = src/game/main.c
SRC = $(ENGINE_SRC) $(GAME_SRC)

all: $(OUT)

$(OUT) : $(SRC)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OUT)
