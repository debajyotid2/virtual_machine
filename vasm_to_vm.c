#include "vm.h"
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "ERROR: Usage: %s <input.vasm> <output.vm>\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }
    const char *input_file_path = argv[1];
    const char *output_file_path = argv[2];

    VM vm = {0};
    StringView source = slurp_file(input_file_path);
    size_t     program_size =
        vm_translate_source(source, vm.program, VM_PROGRAM_CAPACITY);
    vm_save_program_to_file(&vm, output_file_path);
    return EXIT_SUCCESS;
}
