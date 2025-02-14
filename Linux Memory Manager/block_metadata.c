#include <stdio.h>
#include <assert.h>
#include "block_metadata.h"

void print_block_metadata(block_metadata_t * block_metadata) {
    printf("block size: %d\n", block_metadata->block_size);
    printf("block is free: %d\n", block_metadata->is_free);
    printf("block is offset: %d\n", block_metadata->offset);
}

void mm_union_free_blocks(block_metadata_t * a, block_metadata_t * b) {
    assert(a != NULL);
    assert(b != NULL);
    assert(a->is_free && b->is_free);
    a->next = b->next;
    if(b->next)b->next->prev = a;
    a->block_size+=sizeof(block_metadata_t) + b->block_size;
}