CC=gcc
CFLAGS=-Wall -Wextra -Wswitch-enum -std=c11 -pedantic
LDFLAGS=
BIN=main evasm_to_vm

all: $(BIN)

main: main.c vm.h
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

evasm_to_vm: evasm_to_vm.c vm.h
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(BIN)
