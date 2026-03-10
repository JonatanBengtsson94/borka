GAME_OUT = hello_world
ifeq ($(COMPILING_PLATFORM),linux)
	GAME_SRC = $(shell find src/games/hello_world -name '*.c')
else ifeq ($(COMPILING_PLATFORM),windows)
	GAME_SRC = $(wildcard src/games/hello_world/*.c)
endif