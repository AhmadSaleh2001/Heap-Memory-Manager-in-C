#pragma once
#include <unistd.h>
#include <sys/mman.h>
#include <memory.h>
#include "vm_page_family.h"

#define MM_MAX_FAMILIES_PER_PAGE \
    (getpagesize() - sizeof(vm_page_families_t)) / sizeof(vm_page_family_t)

#define ITERATE_PAGE_FAMILY_BEGIN(vm_page_family_ptr, current_page_family) { \
    int count = 0; \
    for(current_page_family=(vm_page_family_t*)&vm_page_family_ptr;count < MM_MAX_FAMILIES_PER_PAGE && current_page_family->size > 0;current_page_family++){ \

#define ITERATE_PAGE_FAMILY_END(vm_page_family_ptr, current_page_family) }}


#define ITERATE_PAGE_FAMILIES_BEGIN(vm_page_families_ptr, current_page_families) { \
    current_page_families = vm_page_families_ptr; \
    for(current_page_families=(vm_page_families_t*)vm_page_families_ptr;current_page_families != NULL;current_page_families = current_page_families->next) { \

#define ITERATE_PAGE_FAMILIES_END(vm_page_families_ptr, current_page_families, current_page_family) }}

typedef struct vm_page_families_ {
    struct vm_page_families_ *next;
    vm_page_family_t vm_page_family[];

} vm_page_families_t;

void print_vm_page_families(vm_page_families_t * vm_page_families);