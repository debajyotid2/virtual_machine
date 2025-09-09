#include "vm.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "ERROR: Usage: %s <input.vm>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    VM vm = {0};
    vm_load_program_from_file(&vm, argv[1]);
    for (size_t i = 0; i < vm.program_size; ++i) {
        inst_print(vm.program[i]);
    }
    return EXIT_SUCCESS;
}
