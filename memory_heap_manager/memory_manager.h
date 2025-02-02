#pragma once
#include "vm_page_families.h"
#include "block_metadata.h"

#define MM_REGISTER_STRUCT(struct_name) \
    mm_instantiate_vm_page_family(#struct_name, sizeof(struct_name));

#define mm_bind_blocks_for_allocation(allocated_meta_block, free_meta_block) \
    allocated_meta_block->next = free_meta_block; \
    allocated_meta_block->prev = free_meta_block->prev; \
    if(free_meta_block->prev) { \
        free_meta_block->prev->next = allocated_meta_block; \
    } \
    free_meta_block->prev = allocated_meta_block; \
    

void init_mmap();
void mm_instantiate_vm_page_family(char * struct_name, int size);
void mm_print_registered_page_families();
vm_page_family_t * lookup_page_family_by_name(char *struct_name);
void mm_union_free_blocks(block_metadata_t * a, block_metadata_t * b);