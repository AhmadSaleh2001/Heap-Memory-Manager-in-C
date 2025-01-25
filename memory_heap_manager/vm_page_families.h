#pragma once
#include <unistd.h>
#include <sys/mman.h>
#include <memory.h>
#include "vm_page_family.h"

#define MM_MAX_FAMILIES_PER_PAGE \
    (getpagesize() - sizeof(vm_page_families_t)) / sizeof(vm_page_family_t)

#define ITERATE_PAGE_FAMILY_BEGIN(vm_page_family_ptr, curr) { \
    int count = 0; \
    for(curr=(vm_page_family_t*)vm_page_family_ptr;count < MM_MAX_FAMILIES_PER_PAGE && curr->size > 0;curr++){ \

#define ITERATE_PAGE_FAMILY_END(vm_page_family_ptr, curr) }}

typedef struct vm_page_families_ {
    struct vm_page_families_ *next;
    vm_page_family_t vm_page_family[];

} vm_page_families_t;

void print_vm_page_families(vm_page_families_t * vm_page_families);