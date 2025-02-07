#pragma once
#include "block_metadata.h"

#define MM_REGISTER_STRUCT(struct_name) \
    mm_instantiate_vm_page_family(#struct_name, sizeof(struct_name));


// we need to put allocated block first (first from bottom)
// meaning, before the current big block
#define mm_bind_blocks_for_allocation(allocated_meta_block, free_meta_block) \
    free_meta_block->next = allocated_meta_block; \
    free_meta_block->prev = allocated_meta_block->prev; \
    if(allocated_meta_block->prev) { \
        allocated_meta_block->prev = free_meta_block; \
    } \
    allocated_meta_block->prev = free_meta_block;

#define MM_MAX_FAMILIES_PER_PAGE \
    (getpagesize() - sizeof(vm_page_families_t)) / sizeof(vm_page_family_t)

#define ITERATE_PAGE_FAMILY_BEGIN(vm_page_family_ptr, current_page_family) { \
    int count = 0; \
    for(current_page_family=(vm_page_family_t*)&vm_page_family_ptr;count < MM_MAX_FAMILIES_PER_PAGE && current_page_family->size > 0;current_page_family++, count++){ \

#define ITERATE_PAGE_FAMILY_END(vm_page_family_ptr, current_page_family) }}

#define ITERATE_PAGE_FAMILIES_BEGIN(vm_page_families_ptr, current_page_families) { \
    current_page_families = vm_page_families_ptr; \
    for(current_page_families=(vm_page_families_t*)vm_page_families_ptr;current_page_families != NULL;current_page_families = current_page_families->next) { \

#define ITERATE_PAGE_FAMILIES_END(vm_page_families_ptr, current_page_families) }}    


#define MAX_METADATA_BLOCKS_PER_VM_PAGE \
    (getpagesize() - sizeof(vm_page_t)) / sizeof(vm_page_t)

#define MARK_VM_PAGE_AS_EMPTY(vm_page) \
        vm_page->prev = NULL; \
        vm_page->next = NULL; \
        vm_page->block_metadata.is_free = true; \

#define ITERATE_VM_PAGES_BEGIN(vm_pages_ptr, current_vm_page) { \
    for(current_vm_page=(vm_page_t*)vm_pages_ptr;current_vm_page != NULL;current_vm_page = current_vm_page->next){ \

#define ITERATE_VM_PAGES_END(vm_page_ptr, current_vm_page) }}

#define ITERATE_VM_PAGE_BLOCKS_BEGIN(vm_page_metadata_blocks, current_metadata_block) { \
    int limit = (int)&vm_page_metadata_blocks + getpagesize(); \
    for(current_metadata_block=(block_metadata_t*)vm_page_metadata_blocks;(int)current_metadata_block < limit;current_metadata_block = (char*)(current_metadata_block + 1) + current_metadata_block->block_size) { \

#define ITERATE_VM_PAGE_BLOCKS_END(vm_page_metadata_blocks, current_metadata_block) }}

#define MM_MAX_STRUCT_NAME 100

typedef struct vm_page_;

typedef struct vm_page_family_ {
    char struct_name[MM_MAX_STRUCT_NAME];
    int size;
    struct vm_page_ * first_vm_page;
} vm_page_family_t;

typedef struct vm_page_families_ {
    struct vm_page_families_ *next;
    struct vm_page_families_ *prev;
    vm_page_family_t vm_page_family[];
} vm_page_families_t;

void print_vm_page_families(vm_page_families_t * vm_page_families);

typedef struct vm_page_ {
    struct vm_page_ * prev;
    struct vm_page_ * next;
    vm_page_family_t * vm_page_familiy;
    block_metadata_t blocks[];
} vm_page_t;

vm_page_families_t * get_vm_page_families();

vm_page_t * mm_allocate_vm_page(vm_page_family_t * vm_page_family);

bool mm_is_page_free(vm_page_t * vm_page);
void print_vm_page_families(vm_page_families_t * vm_page_families);

void init_mmap();
void mm_instantiate_vm_page_family(char * struct_name, int size);
void mm_print_registered_page_families();
vm_page_family_t * lookup_page_family_by_name(char *struct_name);
void mm_union_free_blocks(block_metadata_t * a, block_metadata_t * b);

void print_page_family_info(vm_page_family_t* vm_page_family);