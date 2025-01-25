#include "vm_page_family.h"

void print_page_family_info(vm_page_family_t* vm_page_family) {
    printf("struct name: %s\n", vm_page_family->struct_name);
    printf("struct size: %d\n", vm_page_family->size);
}