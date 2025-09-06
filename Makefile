CC=gcc
CFLAGS=-Wall -Wextra -Wswitch-enum -std=c11 -pedantic
LDFLAGS=
BIN=vm_interpreter evasm_to_vm

all: $(BIN)

vm_interpreter: vm_interpreter.c vm.h
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

evasm_to_vm: evasm_to_vm.c vm.h
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(BIN)
