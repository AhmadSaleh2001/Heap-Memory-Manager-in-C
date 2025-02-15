#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include <memory.h>
#include "block_metadata.h"
#include "memory_manager.h"
#include "gluethread/glthread.h"

static int32_t SYSTEM_PAGE_SIZE = 0;
static vm_page_families_t * first_family_page = NULL;

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

vm_page_t * mm_allocate_vm_page(vm_page_family_t * vm_page_family, int (*compFuncPtr)(void *, void *)) {
    vm_page_t * vm_page = (vm_page_t*)mm_get_new_vm_page_from_kernel(1);

    vm_page->next = NULL;
    vm_page->prev = NULL;
    
    vm_page->blocks[0].is_free = true;
    vm_page->blocks[0].block_size = getpagesize() - sizeof(vm_page_t) - sizeof(block_metadata_t);
    vm_page->blocks[0].prev = NULL;
    vm_page->blocks[0].next = NULL;
    vm_page->blocks[0].offset = offset_of(vm_page_t, blocks);
    vm_page->vm_page_familiy = vm_page_family;

    if(vm_page_family->first_vm_page == NULL) {
        vm_page_family->first_vm_page = vm_page;
    } else {
        vm_page_family->first_vm_page->prev = vm_page;
        vm_page->next = vm_page_family->first_vm_page;
        vm_page_family->first_vm_page = vm_page;
    }

    glthread_priority_insert(&vm_page_family->base_glthread, &vm_page->glnode, compFuncPtr, offset_of(vm_page_t, glnode));
    return vm_page;
}

// largest page has page, should be first
int worstFitAlgorithm(void * page1, void * page2) {
    int maxPage1 = get_largest_free_block(page1);
    int maxPage2 = get_largest_free_block(page2);
    if(maxPage1 > maxPage2)return -1;
    else if(maxPage1 < maxPage2)return 1;
    return 0;
}

void * xmalloc(char * struct_name) {
    vm_page_family_t * page_family = lookup_page_family_by_name(struct_name);
    if(page_family == NULL) {
        printf("Struct with name %s not registerd\n", struct_name);
        exit(0);
    }
    block_metadata_t * free_block = worst_fit_page(page_family, 1);
    if(free_block == NULL)mm_allocate_vm_page(page_family, worstFitAlgorithm);
    free_block = worst_fit_page(page_family, 1);
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

    block_metadata_t * free_block = worst_fit_page(page_family, units);
    if(free_block == NULL)mm_allocate_vm_page(page_family, worstFitAlgorithm);
    free_block = worst_fit_page(page_family, units);
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

    int first_empty_index_for_new_page_family = 0;
    vm_page_family_t * current_page_family = NULL;
    ITERATE_PAGE_FAMILY_BEGIN(first_family_page->vm_page_family, current_page_family) {
        if(memcmp(current_page_family->struct_name, struct_name, MM_MAX_STRUCT_NAME) == 0) {
            assert(0);
        }
        first_empty_index_for_new_page_family++;
    } ITERATE_PAGE_FAMILY_END(first_family_page->vm_page_family, current_page_family)

    if(first_empty_index_for_new_page_family == MM_MAX_FAMILIES_PER_PAGE) {
        vm_page_families_t * new_page = (vm_page_families_t*)mm_get_new_vm_page_from_kernel(1);
        new_page->next = first_family_page;
        first_family_page = new_page;
        first_empty_index_for_new_page_family = 0;
    }

    first_family_page->vm_page_family[first_empty_index_for_new_page_family].size = size;
    init_glthread(&first_family_page->vm_page_family[first_empty_index_for_new_page_family].base_glthread);
    memcpy(&first_family_page->vm_page_family[first_empty_index_for_new_page_family].struct_name, struct_name, MM_MAX_STRUCT_NAME);
}

void remove_family_vm_page(vm_page_family_t * vm_page_familiy, vm_page_t * vm_page) {
    remove_glthread(&vm_page->glnode);
    if(vm_page_familiy->first_vm_page == vm_page) {
        vm_page_familiy->first_vm_page = vm_page_familiy->first_vm_page->next;
        if(vm_page_familiy->first_vm_page)vm_page_familiy->first_vm_page->prev = NULL;
        return;
    }
    if(vm_page->prev)vm_page->prev->next = vm_page->next;
    if(vm_page->next)vm_page->next->prev = vm_page->prev;
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

block_metadata_t * first_fit_page(vm_page_family_t * vm_page_family, int units) {
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


block_metadata_t * worst_fit_page(vm_page_family_t * vm_page_family, int units) {
    vm_page_t * curr_vm_page = NULL;
    block_metadata_t * first_free_enough_block = NULL;
    glthread_t *curr_glnode = NULL;
    ITERATE_GLTHREAD_BEGIN(&vm_page_family->base_glthread, curr_glnode){
        vm_page_t *curr_vm_page = thread_to_vm_page(curr_glnode);
        block_metadata_t * curr_block = NULL;
        ITERATE_VM_PAGE_BLOCKS_BEGIN(curr_vm_page->blocks, curr_block) {
            if(curr_block->is_free && curr_block->block_size >= sizeof(block_metadata_t) + units*vm_page_family->size) {
                first_free_enough_block = curr_block;
                break;
            }
        } ITERATE_VM_PAGE_BLOCKS_END(curr_vm_page->blocks, curr_block)
    } ITERATE_GLTHREAD_END(&current_page_family->base_glthread, curr_glnode);
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

void print_vm_page_families(vm_page_families_t * vm_page_families) {
    vm_page_families_t * current_page_families = NULL;
    ITERATE_PAGE_FAMILIES_BEGIN(vm_page_families, current_page_families) {
        vm_page_family_t * current_page_family = NULL;
        ITERATE_PAGE_FAMILY_BEGIN(current_page_families->vm_page_family, current_page_family) {
            print_page_family_info(current_page_family);
        } ITERATE_PAGE_FAMILY_END(current_page_families->vm_page_family, current_page_family)
    } ITERATE_PAGE_FAMILIES_END(vm_page_families, current_page_families)
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
    free_block->block_size -= total_size + sizeof(block_metadata_t);

    block_metadata_t * new_block = free_block;
    memcpy((char*)free_block + total_size, (char*)free_block, sizeof(block_metadata_t));
    block_metadata_t * old_free_block = (char*)free_block + total_size;

    new_block->is_free = false;
    new_block->block_size = vm_page_family->size*units;
    new_block->next = new_block->prev = NULL;
    new_block->offset = old_offset;
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

void print_memory_status_using_glthreads() {
    vm_page_families_t * current_page_families = NULL;
    ITERATE_PAGE_FAMILIES_BEGIN(first_family_page, current_page_families) {
        vm_page_family_t * current_page_family = NULL;
        ITERATE_PAGE_FAMILY_BEGIN(current_page_families->vm_page_family, current_page_family) {
            printf("\n----- %s ------\n", current_page_family->struct_name);
            glthread_t *curr_glnode = NULL;
            ITERATE_GLTHREAD_BEGIN(&current_page_family->base_glthread, curr_glnode){
                vm_page_t *curr_vm_page = thread_to_vm_page(curr_glnode);
                int current_page_max_block_size = get_largest_free_block(curr_vm_page);
                printf("current page max block size: %d\n", current_page_max_block_size);
                block_metadata_t * curr_block = NULL;
                ITERATE_VM_PAGE_BLOCKS_BEGIN(curr_vm_page->blocks, curr_block) {
                    print_block_metadata(curr_block);
                    printf("\n");
                } ITERATE_VM_PAGE_BLOCKS_END(curr_vm_page->blocks, curr_block)
            } ITERATE_GLTHREAD_END(&current_page_family->base_glthread, curr_glnode);
            printf("\n----- %s ------\n", current_page_family->struct_name);
        } ITERATE_PAGE_FAMILY_END(current_page_families->vm_page_family, current_page_family)
    } ITERATE_PAGE_FAMILIES_END(first_family_page, current_page_families)
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