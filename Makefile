CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -pedantic
LDFLAGS=
BIN=main

all: $(BIN)

main: main.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(BIN)
