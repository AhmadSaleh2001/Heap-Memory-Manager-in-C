#pragma once
#include "vm_page.h"

#define MM_MAX_STRUCT_NAME 100

typedef struct vm_page_family_ {
    char struct_name[MM_MAX_STRUCT_NAME];
    int size;
    vm_page_t * first_vm_page;
} vm_page_family_t;