#pragma once
#include <stdio.h>

#define MM_MAX_STRUCT_NAME 100

typedef struct vm_page_family_ {
    char struct_name[MM_MAX_STRUCT_NAME];
    int size;
} vm_page_family_t;

void print_page_family_info(vm_page_family_t* vm_page_family);