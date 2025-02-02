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


typedef struct block_metadata_
{
    bool is_free;
    uint32_t block_size;
    struct block_metadata_ * prev;
    struct block_metadata_ * next;
    int offset;
} block_metadata_t;
