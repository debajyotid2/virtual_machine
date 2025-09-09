#include "vm.h"
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "ERROR: Usage: %s <input.vm>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char *input_file_path = argv[1];

    VM vm = {0};
    vm_load_program_from_file(&vm, input_file_path);
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
