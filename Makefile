CC=gcc
CFLAGS=-Wall -Wextra -Wswitch-enum -std=c11 -pedantic
LDFLAGS=
BIN=vm_interpreter evasm_to_vm

.PHONY: all examples clean

all: $(BIN)

%: %.c vm.h
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

EXAMPLES_DIR=./examples
EXAMPLES=$(wildcard $(EXAMPLES_DIR)/*.evasm)
EXAMPLE_TARGETS=$(patsubst %.evasm,%.vm,$(EXAMPLES))

examples: $(EXAMPLE_TARGETS)

$(EXAMPLES_DIR)/%.vm: $(EXAMPLES_DIR)/%.evasm
	./evasm_to_vm $^ $@

clean:
	rm -rf $(BIN) $(EXAMPLE_TARGETS)
