#ifndef _VM_H_
#define _VM_H_

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VM_STACK_CAPACITY 1024
#define VM_PROGRAM_CAPACITY 65536
#define VM_EXECUTION_LIMIT 69
typedef int64_t Word;

typedef struct {
    size_t count;
    char  *data;
} StringView;

StringView cstr_as_sv(char *str) {
    return (StringView){.count = strlen(str), .data = str};
}

// NOTE: This leaks memory if string is heap allocated.
// So the original pointer `str` needs to be freed instead
// of the pointer returned.
StringView sv_trim_left(StringView sv) {
    for (size_t i = 0; i < sv.count; ++i) {
        if (!isspace(sv.data[i])) {
            return (StringView){.data = sv.data + i, .count = sv.count - i};
        }
    }
    return (StringView){.data = sv.data + sv.count, .count = 0};
}

StringView sv_trim_right(StringView sv) {
    for (size_t i = 0; i < sv.count; ++i) {
        if (!isspace(sv.data[sv.count - 1 - i])) {
            return (StringView){.data = sv.data, .count = sv.count - i};
        }
    }
    return (StringView){.data = sv.data, .count = 0};
}

StringView sv_trim(StringView sv) { return sv_trim_left(sv_trim_right(sv)); }

StringView sv_chop_by_delim(StringView *sv, char delim) {
    size_t i = 0;
    while (i < sv->count && sv->data[i] != delim) {
        i += 1;
    }
    StringView result = {.count = i, .data = sv->data};
    if (i < sv->count) {
        sv->count -= i + 1;
        sv->data += i + 1;
    } else {
        sv->data += sv->count;
        sv->count = 0;
    }
    return result;
}

int sv_eq(StringView a, StringView b) {
    if (a.count != b.count) {
        return 0;
    }
    return memcmp(a.data, b.data, a.count) == 0;
}

int sv_to_int(StringView sv) {
    int    result = 0;
    size_t i = 0;

    while (i < sv.count && isdigit(sv.data[i])) {
        result = result * 10 + sv.data[i] - '0';
        i++;
    }
    return result;
}

#define ARRAY_SIZE(arg) (sizeof(arg) / sizeof((arg)[0]))

typedef enum {
    ERR_OK = 0,
    ERR_STACK_OVERFLOW,
    ERR_STACK_UNDERFLOW,
    ERR_ILLEGAL_INST,
    ERR_DIV_BY_ZERO,
    ERR_ILLEGAL_INST_ACCESS,
    ERR_ILLEGAL_OPERAND
} Err;

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
    case ERR_DIV_BY_ZERO:
        return "ERR_DIV_BY_ZERO";
    case ERR_ILLEGAL_INST_ACCESS:
        return "ERR_ILLEGAL_INST_ACCESS";
    case ERR_ILLEGAL_OPERAND:
        return "ERR_ILLEGAL_OPERAND";
    default:
        assert(0 && "error_as_cstr: Unreachable");
    }
}

typedef enum {
    INST_NO_OP = 0,
    INST_PUSH,
    INST_PLUS,
    INST_MINUS,
    INST_MULT,
    INST_DIV,
    INST_JMP,
    INST_JMP_IF,
    INST_HALT,
    INST_EQ,
    INST_DUP,
    INST_PRINT_DEBUG,
} InstType;

typedef struct {
    InstType type;
    Word     operand;
} Inst;

typedef struct {
    Word   stack[VM_STACK_CAPACITY];
    size_t stack_size;
    Inst   program[VM_PROGRAM_CAPACITY];
    size_t program_size;
    Word   ip;
    int    halt;
} VM;

#define MAKE_INST_PUSH(op) {.type = INST_PUSH, .operand = (op)}
#define MAKE_INST_PLUS {.type = INST_PLUS}
#define MAKE_INST_MINUS {.type = INST_MINUS}
#define MAKE_INST_MULT {.type = INST_MULT}
#define MAKE_INST_DIV {.type = INST_DIV}
#define MAKE_INST_EQ {.type = INST_EQ}
#define MAKE_INST_JMP(addr) {.type = INST_JMP, .operand = (addr)}
#define MAKE_INST_JMP_IF(addr) {.type = INST_JMP_IF, .operand = (addr)}
#define MAKE_INST_DUP(addr) {.type = INST_DUP, .operand = (addr)}
#define MAKE_INST_HALT {.type = INST_HALT}

const char *inst_type_as_cstr(InstType type) {
    switch (type) {
    case INST_PUSH:
        return "INST_PUSH";
    case INST_PLUS:
        return "INST_PLUS";
    case INST_MINUS:
        return "INST_MINUS";
    case INST_MULT:
        return "INST_MULT";
    case INST_DIV:
        return "INST_DIV";
    case INST_JMP:
        return "INST_JMP";
    case INST_JMP_IF:
        return "INST_JMP_IF";
    case INST_DUP:
        return "INST_DUP";
    case INST_HALT:
        return "INST_HALT";
    case INST_EQ:
        return "INST_EQ";
    case INST_PRINT_DEBUG:
        return "INST_PRINT_DEBUG";
    case INST_NO_OP:
        return "INST_NO_OP";
    default:
        assert(0 && "inst_type_as_cstr: Unreachable");
    }
}

Err vm_execute_inst(VM *vm) {
    if (vm->ip < 0 || vm->ip >= (int)vm->program_size) {
        return ERR_ILLEGAL_INST_ACCESS;
    }
    Inst *inst = &vm->program[vm->ip];
    switch (inst->type) {
    case INST_PUSH:
        if (vm->stack_size >= VM_STACK_CAPACITY) {
            return ERR_STACK_OVERFLOW;
        };
        vm->stack[vm->stack_size++] = inst->operand;
        vm->ip += 1;
        break;
    case INST_PLUS:
        if (vm->stack_size < 2) {
            return ERR_STACK_UNDERFLOW;
        }
        vm->stack[vm->stack_size - 2] += vm->stack[vm->stack_size - 1];
        vm->stack_size -= 1;
        vm->ip += 1;
        break;
    case INST_MINUS:
        if (vm->stack_size < 2) {
            return ERR_STACK_UNDERFLOW;
        }
        vm->stack[vm->stack_size - 2] -= vm->stack[vm->stack_size - 1];
        vm->stack_size -= 1;
        vm->ip += 1;
        break;
    case INST_MULT:
        if (vm->stack_size < 2) {
            return ERR_STACK_UNDERFLOW;
        }
        vm->stack[vm->stack_size - 2] *= vm->stack[vm->stack_size - 1];
        vm->stack_size -= 1;
        vm->ip += 1;
        break;
    case INST_DIV:
        if (vm->stack_size < 2) {
            return ERR_STACK_UNDERFLOW;
        }
        if (vm->stack[vm->stack_size - 1] == 0) {
            return ERR_DIV_BY_ZERO;
        }
        vm->stack[vm->stack_size - 2] /= vm->stack[vm->stack_size - 1];
        vm->stack_size -= 1;
        vm->ip += 1;
        break;
    case INST_JMP:
        vm->ip = inst->operand;
        break;
    case INST_HALT:
        vm->halt = 1;
        break;
    case INST_EQ:
        if (vm->stack_size < 2) {
            return ERR_STACK_UNDERFLOW;
        }
        vm->stack[vm->stack_size - 2] =
            (vm->stack[vm->stack_size - 2] == vm->stack[vm->stack_size - 1]);
        vm->stack_size -= 1;
        vm->ip += 1;
        break;
    case INST_JMP_IF:
        if (vm->stack_size < 1) {
            return ERR_STACK_UNDERFLOW;
        }
        if (vm->stack[vm->stack_size - 1]) {
            vm->stack_size -= 1;
            vm->ip = inst->operand;
        } else {
            vm->ip += 1;
        }
        break;
    case INST_DUP:
        if (vm->stack_size >= VM_STACK_CAPACITY) {
            return ERR_STACK_OVERFLOW;
        };
        if (vm->stack_size - inst->operand <= 0) {
            return ERR_STACK_UNDERFLOW;
        }
        if (inst->operand < 0) {
            return ERR_ILLEGAL_OPERAND;
        }
        vm->stack[vm->stack_size] =
            vm->stack[vm->stack_size - 1 - inst->operand];
        vm->stack_size += 1;
        vm->ip += 1;
        break;
    case INST_PRINT_DEBUG:
        if (vm->stack_size < 1) {
            return ERR_STACK_UNDERFLOW;
        }
        printf("%ld\n", vm->stack[vm->stack_size - 1]);
        vm->stack_size -= 1;
        vm->ip += 1;
        break;
    case INST_NO_OP:
        vm->ip += 1;
        break;
    default:
        return ERR_ILLEGAL_INST;
    }
    return ERR_OK;
}

void vm_dump_stack(FILE *stream, const VM *vm) {
    if (vm->stack_size == 0) {
        fprintf(stream, "  [EMPTY]  \n");
        return;
    }
    printf("Stack:\n");
    for (size_t i = 0; i < vm->stack_size; ++i) {
        fprintf(stream, "\t%ld\n", vm->stack[i]);
    }
}

void vm_push_inst(VM *vm, Inst inst) {
    assert(vm->program_size < VM_PROGRAM_CAPACITY);
    vm->program[vm->program_size++] = inst;
}

void vm_load_program_from_memory(VM *vm, Inst *program, size_t program_size) {
    assert(program_size <= VM_PROGRAM_CAPACITY);
    memcpy(vm->program, program, sizeof(program[0]) * program_size);
    vm->program_size = program_size;
}

void vm_save_program_to_file(VM* vm,
                             const char *file_path) {
    FILE *f = fopen(file_path, "wb");
    if (f == NULL) {
        fprintf(stderr, "ERROR: Could not open file '%s': %s\n", file_path,
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    fwrite(vm->program, sizeof(vm->program[0]), vm->program_size, f);
    if (ferror(f)) {
        fprintf(stderr, "ERROR: Could not write to file '%s': %s\n", file_path,
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    fclose(f);
}

void vm_load_program_from_file(VM *vm, const char *file_path) {
    FILE *f = fopen(file_path, "rb");
    if (f == NULL) {
        fprintf(stderr, "ERROR: Could not open file '%s': %s\n", file_path,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Get file size
    if (fseek(f, 0, SEEK_END) < 0) {
        fprintf(stderr, "ERROR: Could not read file '%s': %s\n", file_path,
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    long res = ftell(f);
    if (res < 0) {
        fprintf(stderr, "ERROR: Could not read file '%s': %s\n", file_path,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Check if the file size is a multiple of instruction size and is
    // within the capacity of the VM
    assert(res % sizeof(vm->program[0]) == 0);
    assert(res <= (long int)(VM_PROGRAM_CAPACITY * sizeof(vm->program[0])));

    // Reset to the beginning of the file
    if (fseek(f, 0, SEEK_SET) < 0) {
        fprintf(stderr, "ERROR: Could not read file '%s': %s\n", file_path,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    vm->program_size = fread(vm->program, sizeof(vm->program[0]),
                             res / sizeof(vm->program[0]), f);
    fclose(f);
}

Inst vm_translate_line(StringView line) {
    line = sv_trim_left(line);
    StringView inst_name = sv_chop_by_delim(&line, ' ');

    if (sv_eq(inst_name, cstr_as_sv("push"))) {
        int operand = sv_to_int(sv_trim(line));
        return (Inst)MAKE_INST_PUSH(operand);
    } else if (sv_eq(inst_name, cstr_as_sv("dup"))) {
        int addr = sv_to_int(sv_trim(line));
        return (Inst)MAKE_INST_DUP(addr);
    } else if (sv_eq(inst_name, cstr_as_sv("jmp"))) {
        int addr = sv_to_int(sv_trim(line));
        return (Inst)MAKE_INST_JMP(addr);
    } else if (sv_eq(inst_name, cstr_as_sv("jmp_if"))) {
        int addr = sv_to_int(sv_trim(line));
        return (Inst)MAKE_INST_JMP_IF(addr);
    } else if (sv_eq(inst_name, cstr_as_sv("plus"))) {
        return (Inst)MAKE_INST_PLUS;
    } else if (sv_eq(inst_name, cstr_as_sv("minus"))) {
        return (Inst)MAKE_INST_MINUS;
    } else if (sv_eq(inst_name, cstr_as_sv("mul"))) {
        return (Inst)MAKE_INST_MULT;
    } else if (sv_eq(inst_name, cstr_as_sv("div"))) {
        return (Inst)MAKE_INST_DIV;
    } else if (sv_eq(inst_name, cstr_as_sv("eq"))) {
        return (Inst)MAKE_INST_EQ;
    } else if (sv_eq(inst_name, cstr_as_sv("halt"))) {
        return (Inst)MAKE_INST_HALT;
    } else {
        fprintf(stderr, "ERROR: Unknown instruction `%.*s`.\n",
                (int)inst_name.count, inst_name.data);
        exit(EXIT_FAILURE);
    }
    return (Inst){0};
}

size_t vm_translate_source(StringView source, Inst *program,
                           size_t program_capacity) {
    size_t program_size = 0;
    while (source.count > 0) {
        assert(program_size < program_capacity);
        StringView line = sv_trim(sv_chop_by_delim(&source, '\n'));
        // Skip empty lines
        if (line.count > 0) {
            program[program_size++] = vm_translate_line(line);
        }
    }
    return program_size;
}

StringView slurp_file(const char *file_path) {
    FILE *f = fopen(file_path, "r");
    if (f == NULL) {
        fprintf(stderr, "ERROR: Could not read file `%s`: %s\n", file_path,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Get file size
    if (fseek(f, 0, SEEK_END) < 0) {
        fprintf(stderr, "ERROR: Could not read file '%s': %s\n", file_path,
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    long res = ftell(f);
    if (res < 0) {
        fprintf(stderr, "ERROR: Could not read file '%s': %s\n", file_path,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    char *buf = (char *)malloc(res);
    if (buf == NULL) {
        fprintf(stderr, "ERROR: Could not allocate memory for buffer: %s.\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Reset to the beginning of the file
    if (fseek(f, 0, SEEK_SET) < 0) {
        fprintf(stderr, "ERROR: Could not read file '%s': %s\n", file_path,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    fread(buf, 1, res, f);
    if (ferror(f)) {
        fprintf(stderr, "ERROR: Could not read file '%s': %s\n", file_path,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    fclose(f);
    return (StringView){.count = (size_t)res, .data = buf};
}

#endif // _VM_H_
