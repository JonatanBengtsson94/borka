cc = gcc
CFLAGS = -Wall -std=c99
OUT_DIR = bin
OUT = $(OUT_DIR)/breakout
SRC = main.c

all: $(OUT)

$(OUT) : $(SRC)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OUT)
