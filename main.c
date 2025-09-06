#include "vm.h"
#include <stdio.h>

VM   vm = {0};
Inst program[] = {MAKE_INST_PUSH(0), MAKE_INST_PUSH(1), MAKE_INST_DUP(1),
                  MAKE_INST_DUP(1),  MAKE_INST_PLUS,    MAKE_INST_JMP(2)};

int main(void) {
    vm_load_program_from_memory(&vm, program, ARRAY_SIZE(program));
    vm_dump_stack(stdout, &vm);
    for (int i = 0; i < VM_EXECUTION_LIMIT && !vm.halt; ++i) {
        Err tr = vm_execute_inst(&vm);
        vm_dump_stack(stderr, &vm);
        if (tr) {
            fprintf(stderr, "ERROR: %s\n", error_as_cstr(tr));
            exit(EXIT_FAILURE);
        }
    }
    vm_dump_stack(stdout, &vm);
    return 0;
}
