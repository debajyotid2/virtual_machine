#include "vm.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "ERROR: Usage: %s <input.vm>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    VM vm = {0};
    vm_load_program_from_file(&vm, argv[1]);
    Err tr = vm_execute_program(&vm, VM_EXECUTION_LIMIT);
    if (tr != ERR_OK) {
        fprintf(stderr, "ERROR: %s\n", error_as_cstr(tr));
        vm_dump_stack(stderr, &vm);
        return EXIT_FAILURE;
    }
    vm_dump_stack(stdout, &vm);
    return EXIT_SUCCESS;
}
