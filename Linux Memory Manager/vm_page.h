#pragma once
#include "block_metadata.h"

typedef struct vm_page_family_t;

// ############ MACROS FOR VM_PAGE_T ############

#define MAX_METADATA_BLOCKS_PER_VM_PAGE \
    (getpagesize() - sizeof(vm_page_t)) / sizeof(vm_page_t)

#define MARK_VM_PAGE_AS_EMPTY(vm_page) \
        vm_page->prev = NULL; \
        vm_page->next = NULL; \
        vm_page->block_metadata.is_free = true; \

#define ITERATE_VM_PAGES_BEGIN(vm_pages_ptr, current_vm_page) { \
    for(current_vm_page=(vm_page_t*)vm_pages_ptr;current_vm_page != NULL;current_vm_page = current_vm_page->next){ \

#define ITERATE_VM_PAGES_END(vm_page_ptr, current_vm_page) }}

// ############ MACROS FOR VM_PAGE_T ############

// ############ MACROS FOR VM_PAGE_FAMILY_T ############

#define MM_MAX_STRUCT_NAME 100
#define ITERATE_PAGE_FAMILY_BEGIN(vm_page_family_ptr, current_page_family) { \
    int count = 0; \
    for(current_page_family=(vm_page_family_t*)vm_page_family_ptr;count < MM_MAX_FAMILIES_PER_PAGE && current_page_family->size > 0;current_page_family++, count++){ \

#define ITERATE_PAGE_FAMILY_END(vm_page_family_ptr, current_page_family) }}

// ############ MACROS FOR VM_PAGE_FAMILY_T ############

typedef struct vm_page_ {
    struct vm_page_ * prev;
    struct vm_page_ * next;
    struct vm_page_family_t * vm_page_familiy;
    block_metadata_t blocks[];
} vm_page_t;

typedef struct vm_page_family_ {
    char struct_name[MM_MAX_STRUCT_NAME];
    int size;
    vm_page_t * first_vm_page;
} vm_page_family_t;

// ############ FUNCTIONS FOR VM_PAGE_T ############

bool mm_is_page_free(vm_page_t * vm_page);
int get_total_number_of_created_blocks(vm_page_t * vm_page);
int get_total_number_of_used_blocks(vm_page_t * vm_page);
void print_vm_pages(vm_page_family_t * vm_page_family);

// ############ FUNCTIONS FOR VM_PAGE_T ############


// ############ FUNCTIONS FOR VM_PAGE_FAMILIY ############

void print_page_family_info(vm_page_family_t* vm_page_family);

// ############ FUNCTIONS FOR VM_PAGE_FAMILIY ############