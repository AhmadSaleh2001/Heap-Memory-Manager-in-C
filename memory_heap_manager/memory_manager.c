#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include <memory.h>
#include "memory_manager.h"

static int32_t SYSTEM_PAGE_SIZE = 0;
static vm_page_families_t * first_family_page = NULL;

void * xmalloc(char * struct_name) {
    vm_page_family_t * page_family = lookup_page_family_by_name(struct_name);
    if(page_family == NULL) {
        printf("Struct with name %s not registerd\n", struct_name);
        exit(0);
    }
    block_metadata_t * free_block = first_fit_block(page_family, 1);
    if(free_block == NULL)mm_allocate_vm_page(page_family);
    free_block = first_fit_block(page_family, 1);
    assert(free_block != NULL);
    block_metadata_t * new_block = mm_allocate_block_metadata(page_family, free_block, 1);
    return (void *)(new_block + 1);
}

void * xcalloc(char * struct_name, int units) {
    vm_page_family_t * page_family = lookup_page_family_by_name(struct_name);
    if(page_family == NULL) {
        printf("Struct with name %s not registerd\n", struct_name);
        exit(0);
    }
    block_metadata_t * free_block = first_fit_block(page_family, units);
    if(free_block == NULL)mm_allocate_vm_page(page_family);
    free_block = first_fit_block(page_family, units);
    assert(free_block != NULL);
    block_metadata_t * new_block = mm_allocate_block_metadata(page_family, free_block, units);
    return (void *)(new_block + 1);
}

void init_mmap() {
    SYSTEM_PAGE_SIZE = getpagesize();
}

vm_page_families_t * get_vm_page_families() {
    return first_family_page;
}

void * mm_get_new_vm_page_from_kernel(int number_of_pages) {
    char * vm_page = mmap(
        0,  // pass it as null currently
        number_of_pages * SYSTEM_PAGE_SIZE, // number of memory pages
        PROT_READ|PROT_WRITE, // permissions on the allocated pages
        MAP_ANON|MAP_PRIVATE,
        0,
        0
    );
    if (vm_page == MAP_FAILED) {
        perror("failed to allocate pages");
        exit(1);
    }

    memset(vm_page, 0, SYSTEM_PAGE_SIZE);
    return vm_page;
}

static void mm_return_vm_page_to_kernel(char * vm_page, int number_of_pages) {
    if(munmap(vm_page, number_of_pages*SYSTEM_PAGE_SIZE)) {
        printf("failed to deallocate vm page");
        exit(1);
    }
}

void remove_family_vm_page(vm_page_family_t * vm_page_familiy, vm_page_t * vm_page) {
    if(vm_page_familiy->first_vm_page == vm_page) {
        vm_page_familiy->first_vm_page = vm_page_familiy->first_vm_page->next;
        if(vm_page_familiy->first_vm_page)vm_page_familiy->first_vm_page->prev = NULL;
        return;
    }
    vm_page_t * current_vm_page = NULL;
    ITERATE_VM_PAGES_BEGIN(vm_page_familiy, current_vm_page) {
        if(current_vm_page == vm_page) {
            if(current_vm_page->prev)current_vm_page->prev->next = current_vm_page->next;
            if(current_vm_page->next)current_vm_page->next->prev = current_vm_page->prev;
            break;
        }
    } ITERATE_VM_PAGES_END(vm_page_familiy, current_vm_page)
}

void freeUnusedPage(block_metadata_t * block) {
    assert(block);
    while(block->prev)block = block->prev;
    if(block->is_free && block->next == NULL) {
        vm_page_t * vm_page = (char*)block - block->offset;
        remove_family_vm_page(vm_page->vm_page_familiy, vm_page);
        mm_return_vm_page_to_kernel(vm_page, 1);
    }
}

void * xfree(void * block_metadata) {
    assert(block_metadata != NULL);
    block_metadata_t * block = (char*)block_metadata - sizeof(block_metadata_t);
    block->is_free = true;
    if(block->next && block->next->is_free)mm_union_free_blocks(block, block->next);
    if(block->prev && block->prev->is_free)mm_union_free_blocks(block->prev, block);

    freeUnusedPage(block);
}

void mm_instantiate_vm_page_family(char * struct_name, int size) {
    if(size > SYSTEM_PAGE_SIZE) {
        printf("Error: %s  to allocate instantiate page with size %d, system page size is %d",
        __FUNCTION__,size, SYSTEM_PAGE_SIZE
        );

        exit(1);
    }

    if(!first_family_page) {
        first_family_page = (vm_page_families_t*)mm_get_new_vm_page_from_kernel(1);
        first_family_page->next = NULL;
    }

    int first_empty_memory_index = 0;
    vm_page_family_t * curr = NULL;
    ITERATE_PAGE_FAMILY_BEGIN(first_family_page->vm_page_family, curr) {
        if(memcmp(curr->struct_name, struct_name, MM_MAX_STRUCT_NAME) == 0) {
            assert(0);
        }
        first_empty_memory_index++;
    } ITERATE_PAGE_FAMILY_END(first_family_page->vm_page_family, curr)

    if(first_empty_memory_index == MM_MAX_FAMILIES_PER_PAGE) {
        vm_page_families_t * new_page = (vm_page_families_t*)mm_get_new_vm_page_from_kernel(1);
        new_page->next = first_family_page;
        first_family_page = new_page;
        first_empty_memory_index = 0;
    }

    first_family_page->vm_page_family[first_empty_memory_index].size = size;
    memcpy(&first_family_page->vm_page_family[first_empty_memory_index].struct_name, struct_name, MM_MAX_STRUCT_NAME);
}

vm_page_t * mm_allocate_vm_page(vm_page_family_t * vm_page_family) {
    vm_page_t * vm_page = (vm_page_t*)mm_get_new_vm_page_from_kernel(1);

    vm_page->next = NULL;
    vm_page->prev = NULL;

    vm_page->blocks[0].is_free = true;
    vm_page->blocks[0].block_size = SYSTEM_PAGE_SIZE - sizeof(block_metadata_t);
    vm_page->blocks[0].prev = NULL;
    vm_page->blocks[0].next = NULL;
    vm_page->blocks[0].offset = offset_of(vm_page_t, blocks);
    vm_page->vm_page_familiy = vm_page_family;
    
    if(vm_page_family->first_vm_page == NULL) {
        vm_page_family->first_vm_page = vm_page;
    } else {
        vm_page->next = vm_page_family->first_vm_page;
        vm_page_family->first_vm_page = vm_page;
    }

    return vm_page;
}

void print_vm_pages(vm_page_family_t * vm_page_family) {
    vm_page_t * curr_vm_page = NULL;
    ITERATE_VM_PAGES_BEGIN(vm_page_family->first_vm_page, curr_vm_page) {
        block_metadata_t * curr_block = NULL;
        ITERATE_VM_PAGE_BLOCKS_BEGIN(curr_vm_page->blocks, curr_block) {
            print_block_metadata(curr_block);
            printf("\n");
        } ITERATE_VM_PAGE_BLOCKS_END(curr_vm_page->blocks, curr_block)
        printf("--------\n");
    } ITERATE_VM_PAGES_END(vm_page_families->first_vm_page, curr_vm_page)
}

block_metadata_t * first_fit_block(vm_page_family_t * vm_page_family, int units) {
    vm_page_t * curr_vm_page = NULL;
    block_metadata_t * first_free_enough_block = NULL;
    ITERATE_VM_PAGES_BEGIN(vm_page_family->first_vm_page, curr_vm_page) {
        block_metadata_t * curr_block = NULL;
        ITERATE_VM_PAGE_BLOCKS_BEGIN(curr_vm_page->blocks, curr_block) {
            if(curr_block->is_free && curr_block->block_size >= sizeof(block_metadata_t) + units*vm_page_family->size) {
                first_free_enough_block = curr_block;
                break;
            }
        } ITERATE_VM_PAGE_BLOCKS_END(curr_vm_page->blocks, curr_block)
        if(first_free_enough_block)break;
    } ITERATE_VM_PAGES_END(vm_page_families->first_vm_page, curr_vm_page)

    return first_free_enough_block;
}

void mm_delete_vm_page(vm_page_t * vm_page) {

    vm_page_family_t * vm_page_family = vm_page->vm_page_familiy;
    if(vm_page_family->first_vm_page == vm_page) {
        vm_page_family->first_vm_page = vm_page->next;
        if(vm_page_family->first_vm_page) {
            vm_page_family->first_vm_page->prev = NULL;
        }
        vm_page->prev = NULL;
        vm_page->next = NULL;
        mm_return_vm_page_to_kernel(vm_page, 1);
        return;
    }

    if(vm_page->prev) {
        vm_page->prev->next = vm_page->next;
    }
    if(vm_page->next) {
        vm_page->next->prev = vm_page->prev;
    }

    mm_return_vm_page_to_kernel(vm_page, 1);
}

void mm_print_registered_page_families() {
    print_vm_page_families(first_family_page);
}

vm_page_family_t * lookup_page_family_by_name(char *struct_name) {
    vm_page_families_t * current_page_families = NULL;
    ITERATE_PAGE_FAMILIES_BEGIN(first_family_page, current_page_families) {
        vm_page_family_t * current_page_family = NULL;
        ITERATE_PAGE_FAMILY_BEGIN(current_page_families->vm_page_family, current_page_family) {
            if(strcmp(current_page_family->struct_name, struct_name) == 0) {
                return current_page_family;
            }
        } ITERATE_PAGE_FAMILY_END(current_page_families->vm_page_family, current_page_family)
    } ITERATE_PAGE_FAMILIES_END(first_family_page, current_page_families)

    return NULL;
}

void mm_union_free_blocks(block_metadata_t * a, block_metadata_t * b) {
    assert(a != NULL);
    assert(b != NULL);
    assert(a->is_free && b->is_free);
    a->next = b->next;
    if(b->next)b->next->prev = a;
    a->block_size+=sizeof(block_metadata_t) + b->block_size;
}

bool mm_is_page_free(vm_page_t * vm_page) {
    return vm_page->blocks[0].is_free == true && vm_page->prev == NULL && vm_page->next == NULL;
}

void print_vm_page_families(vm_page_families_t * vm_page_families) {
    vm_page_families_t * current_page_families = NULL;
    ITERATE_PAGE_FAMILIES_BEGIN(vm_page_families, current_page_families) {
        vm_page_family_t * current_page_family = NULL;
        ITERATE_PAGE_FAMILY_BEGIN(current_page_families->vm_page_family, current_page_family) {
            print_page_family_info(current_page_family);
        } ITERATE_PAGE_FAMILY_END(current_page_families->vm_page_family, current_page_family)
    } ITERATE_PAGE_FAMILIES_END(vm_page_families, current_page_families)
}

void print_page_family_info(vm_page_family_t* vm_page_family) {
    printf("struct name: %s\n", vm_page_family->struct_name);
    printf("struct size: %d\n", vm_page_family->size);
}

block_metadata_t * mm_allocate_block_metadata(vm_page_family_t * vm_page_family, block_metadata_t * free_block, int units) {
    assert(free_block->is_free == true);
    int old_offset = free_block->offset;
    int total_size = sizeof(block_metadata_t) + units*vm_page_family->size;

    // handle interanl fragmentation case
    // soft fragmentation => sizeof(block_metadata_t) < remaining size
    // hard fragmentation => sizeof(block_metadata_t) >= remaining size
    if(free_block->block_size - total_size <= sizeof(block_metadata_t)) {
        free_block->is_free = false;
        return free_block;
    }
    
    free_block->offset += total_size;
    free_block->block_size -= total_size;

    // print_block_metadata(free_block);
    // printf("old block offset: %d\n", (char*)free_block);
    // printf("\n");

    block_metadata_t * new_block = (char*)free_block;
    memcpy((char*)free_block + total_size, (char*)free_block, sizeof(block_metadata_t));
    block_metadata_t * old_free_block = (char*)free_block + total_size;

    new_block->is_free = false;
    new_block->block_size = vm_page_family->size*units;
    new_block->next = new_block->prev = NULL;
    new_block->offset = old_offset;

    // print_block_metadata(old_free_block);
    // print_block_metadata(new_block);

    mm_bind_blocks_for_allocation(old_free_block, new_block);

    return new_block;
}

void print_memory_status() {
    vm_page_families_t * current_page_families = NULL;
    ITERATE_PAGE_FAMILIES_BEGIN(first_family_page, current_page_families) {
        vm_page_family_t * current_page_family = NULL;
        ITERATE_PAGE_FAMILY_BEGIN(current_page_families->vm_page_family, current_page_family) {
            printf("\n----- %s ------\n", current_page_family->struct_name);
            print_vm_pages(current_page_family);
            printf("\n----- %s ------\n", current_page_family->struct_name);
        } ITERATE_PAGE_FAMILY_END(current_page_families->vm_page_family, current_page_family)
    } ITERATE_PAGE_FAMILIES_END(first_family_page, current_page_families)
}

int get_total_number_of_created_blocks(vm_page_t * vm_page) {
    int total = 0;

    vm_page_t * current_vm_page = NULL;
    ITERATE_VM_PAGES_BEGIN(vm_page, current_vm_page) {
        block_metadata_t * current_block_metadata = NULL;
        ITERATE_VM_PAGE_BLOCKS_BEGIN(vm_page->blocks, current_block_metadata) {
            total++;
        } ITERATE_VM_PAGE_BLOCKS_END(vm_page->blocks, current_block_metadata)
        vm_page = vm_page->next;
    } ITERATE_VM_PAGES_END(vm_page, current_vm_page)
    return total;
}

int get_total_number_of_used_blocks(vm_page_t * vm_page) {
    int total = 0;

    vm_page_t * current_vm_page = NULL;
    ITERATE_VM_PAGES_BEGIN(vm_page, current_vm_page) {
        block_metadata_t * current_block_metadata = NULL;
        ITERATE_VM_PAGE_BLOCKS_BEGIN(vm_page->blocks, current_block_metadata) {
            total+=current_block_metadata->is_free == true;
        } ITERATE_VM_PAGE_BLOCKS_END(vm_page->blocks, current_block_metadata)
        vm_page = vm_page->next;
    } ITERATE_VM_PAGES_END(vm_page, current_vm_page)
    return total;
}

int print_and_get_vm_page_statistcs(vm_page_family_t * vm_page_family) {
    int total_number_of_created_blocks = get_total_number_of_created_blocks(vm_page_family->first_vm_page);
    int total_number_of_used_blocks = get_total_number_of_used_blocks(vm_page_family->first_vm_page);

    printf("struct: %s: \n", vm_page_family->struct_name);
    printf("blocks status:\tcreated blocks: %d\tused blocks: %d\tfree blocks: %d\n", total_number_of_created_blocks, total_number_of_used_blocks, total_number_of_created_blocks - total_number_of_used_blocks);
    return total_number_of_created_blocks;
}

void print_memory_usage() {
    vm_page_families_t * current_page_families = NULL;
    ITERATE_PAGE_FAMILIES_BEGIN(first_family_page, current_page_families) {
        vm_page_family_t * current_page_family = NULL;
        ITERATE_PAGE_FAMILY_BEGIN(current_page_families->vm_page_family, current_page_family) {
            printf("\n----- %s ------\n", current_page_family->struct_name);
            print_and_get_vm_page_statistcs(current_page_family);
            printf("\n----- %s ------\n", current_page_family->struct_name);
        } ITERATE_PAGE_FAMILY_END(current_page_families->vm_page_family, current_page_family)
    } ITERATE_PAGE_FAMILIES_END(first_family_page, current_page_families)
}