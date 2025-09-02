#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define VM_STACK_CAPACITY 1

typedef int64_t Word;

typedef enum { ERR_OK = 0, ERR_STACK_OVERFLOW, ERR_STACK_UNDERFLOW, ERR_ILLEGAL_INST } Err;

const char *error_as_cstr(Err error) {
    switch (error) {
    case ERR_OK:
        return "ERR_OK";
    case ERR_STACK_OVERFLOW:
        return "ERR_STACK_OVERFLOW";
    case ERR_STACK_UNDERFLOW:
        return "ERR_STACK_UNDERFLOW";
    case ERR_ILLEGAL_INST:
        return "ERR_ILLEGAL_INST";
    default:
        assert(0 && "error_as_cstr: Unreachable");
    }
}

typedef struct {
    Word stack[VM_STACK_CAPACITY];
    size_t stack_size;
} VM;

typedef enum {
    INST_PUSH,
    INST_PLUS,
} InstType;

typedef struct {
    InstType type;
    Word operand;
} Inst;

#define MAKE_INST_PUSH(op) {.type = INST_PUSH, .operand = (op)}
#define MAKE_INST_PLUS {.type = INST_PLUS}

Err vm_execute_inst(VM *vm, Inst inst) {
    switch (inst.type) {
    case INST_PUSH:
        if (vm->stack_size >= VM_STACK_CAPACITY) {
            return ERR_STACK_OVERFLOW;
        };
        vm->stack[vm->stack_size++] = inst.operand;
        break;
    case INST_PLUS:
        if (vm->stack_size < 2) {
            return ERR_STACK_UNDERFLOW;
        }
        vm->stack[vm->stack_size - 2] += vm->stack[vm->stack_size - 1];
        vm->stack_size -= 1;
        break;
    default:
        return ERR_ILLEGAL_INST;
    }
    return ERR_OK;
}

void vm_dump(FILE *stream, const VM *vm) {
    if (vm->stack_size == 0) {
        fprintf(stream, "  [EMPTY]  \n");
        return;
    }
    printf("Stack:\n");
    for (size_t i = 0; i < vm->stack_size; ++i) {
        fprintf(stream, "\t%ld\n", vm->stack[i]);
    }
}

#define ARRAY_SIZE(arg) (sizeof(arg) / sizeof((arg)[0]))

VM vm = {0};
Inst program[] = {
    MAKE_INST_PUSH(68),
    MAKE_INST_PUSH(42),
    MAKE_INST_PLUS,
};

int main() {
    vm_dump(stdout, &vm);
    for (size_t i = 0; i < ARRAY_SIZE(program); ++i) {
        Err tr = vm_execute_inst(&vm, program[i]);
        if (tr) {
            fprintf(stderr, "Err activated: %s\n", error_as_cstr(tr));
            vm_dump(stderr, &vm);
            exit(EXIT_FAILURE);
        }
    }
    vm_dump(stdout, &vm);
    return 0;
}
