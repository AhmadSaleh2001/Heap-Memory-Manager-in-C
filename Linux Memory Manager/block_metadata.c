#include <stdio.h>
#include "block_metadata.h"

void print_block_metadata(block_metadata_t * block_metadata) {
    printf("block size: %d\n", block_metadata->block_size);
    printf("block is free: %d\n", block_metadata->is_free);
    printf("block is offset: %d\n", block_metadata->offset);
}