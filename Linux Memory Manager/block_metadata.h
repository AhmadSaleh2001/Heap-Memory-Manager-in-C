#pragma once
#include <inttypes.h>
#include <stdbool.h>

#define offset_of(struct_name, field_name) \
    &(((struct_name*)0)->field_name)

#define MM_GET_PAGE_FROM_META_BLOCK(block_metadata_ptr) \
    ((char*)block_metadata_ptr) - block_metadata_ptr->offset

#define MM_NEXT_META_BLOCK(block_metadata_ptr) \
    block_metadata_ptr->next

#define MM_NEXT_META_BLOCK_BY_SIZE(block_metadata_ptr) \
    (char*)(block_metadata_ptr + 1) + block_metadata_ptr->block_size

#define MM_PREV_META_BLOCK(block_metadata_ptr) \
    block_metadata_ptr->prev

#define ITERATE_VM_PAGE_BLOCKS_BEGIN(vm_page_metadata_blocks, current_metadata_block) { \
    int limit = (int)&vm_page_metadata_blocks + getpagesize(); \
    for(current_metadata_block=(block_metadata_t*)vm_page_metadata_blocks;(int)current_metadata_block < limit;current_metadata_block = (char*)(current_metadata_block + 1) + current_metadata_block->block_size) { \

#define ITERATE_VM_PAGE_BLOCKS_END(vm_page_metadata_blocks, current_metadata_block) }}


typedef struct block_metadata_
{
    struct block_metadata_ * prev;
    struct block_metadata_ * next;
    bool is_free;
    uint32_t block_size;
    int offset;
} block_metadata_t;

void print_block_metadata(block_metadata_t * block_metadata);
void mm_union_free_blocks(block_metadata_t * a, block_metadata_t * b);