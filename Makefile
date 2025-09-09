CC=gcc
CFLAGS=-Wall -Wextra -Wswitch-enum -std=c11 -pedantic
LDFLAGS=
BIN=vm_interpreter vasm_to_vm devasm

.PHONY: all examples clean

all: $(BIN)

%: %.c vm.h
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

EXAMPLES_DIR=./examples
EXAMPLES=$(wildcard $(EXAMPLES_DIR)/*.vasm)
EXAMPLE_TARGETS=$(patsubst %.vasm,%.vm,$(EXAMPLES))

examples: $(EXAMPLE_TARGETS)

$(EXAMPLES_DIR)/%.vm: $(EXAMPLES_DIR)/%.vasm
	./vasm_to_vm $^ $@

clean:
	rm -rf $(BIN) $(EXAMPLE_TARGETS)
