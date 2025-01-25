#pragma once
#include "vm_page_families.h"

#define MM_REGISTER_STRUCT(struct_name) \
    mm_instantiate_vm_page_family(#struct_name, sizeof(struct_name));

void init_mmap();
void mm_instantiate_vm_page_family(char * struct_name, int size);
void mm_print_vm_page_families();