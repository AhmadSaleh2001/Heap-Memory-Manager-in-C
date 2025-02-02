#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include <memory.h>
#include "memory_manager.h"
#include "vm_page_family.h"
#include "vm_page_families.h"

static int32_t SYSTEM_PAGE_SIZE = 0;
vm_page_families_t * first_familiy_page = NULL;

void init_mmap() {
    SYSTEM_PAGE_SIZE = getpagesize();
}

static void * mm_get_new_vm_page_from_kernel(int number_of_pages) {
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

void mm_instantiate_vm_page_family(char * struct_name, int size) {
    if(size > SYSTEM_PAGE_SIZE) {
        printf("Error: %s  to allocate instantiate page with size %d, system page size is %d",
        __FUNCTION__,size, SYSTEM_PAGE_SIZE
        );

        exit(1);
    }

    if(!first_familiy_page) {
        first_familiy_page = (vm_page_families_t*)mm_get_new_vm_page_from_kernel(1);
        first_familiy_page->next = NULL;
    }

    int first_empty_memory_index = 0;
    vm_page_family_t * curr = NULL;
    ITERATE_PAGE_FAMILY_BEGIN(first_familiy_page->vm_page_family, curr) {
        if(memcmp(curr->struct_name, struct_name, MM_MAX_STRUCT_NAME) == 0) {
            assert(0);
        }
        first_empty_memory_index++;
    } ITERATE_PAGE_FAMILY_END(first_familiy_page->vm_page_family, curr)

    if(first_empty_memory_index == MM_MAX_FAMILIES_PER_PAGE) {
        vm_page_families_t * new_page = (vm_page_families_t*)mm_get_new_vm_page_from_kernel(1);
        new_page->next = first_familiy_page;
        first_familiy_page = new_page;
        first_empty_memory_index = 0;
    }

    first_familiy_page->vm_page_family[first_empty_memory_index].size = size;
    memcpy(&first_familiy_page->vm_page_family[first_empty_memory_index].struct_name, struct_name, MM_MAX_STRUCT_NAME);
}

void mm_print_registered_page_families() {
    print_vm_page_families(first_familiy_page);
}

vm_page_family_t * lookup_page_family_by_name(char *struct_name) {
    vm_page_families_t * current_page_families = NULL;
    ITERATE_PAGE_FAMILIES_BEGIN(first_familiy_page, current_page_families) {
        vm_page_family_t * current_page_family = NULL;
        ITERATE_PAGE_FAMILY_BEGIN(current_page_families->vm_page_family, current_page_family) {
            if(strcmp(current_page_family->struct_name, struct_name) == 0) {
                return current_page_family;
            }
        } ITERATE_PAGE_FAMILY_END(current_page_families, current_page_family)
    } ITERATE_PAGE_FAMILIES_END(vm_page_families, current_page_families, current_page_family)

    return NULL;
}

void mm_union_free_blocks(block_metadata_t * a, block_metadata_t * b) {
    assert(a->is_free && b->is_free);
    a->next = b->next;
    if(b->next)b->prev = a;
    a->block_size+=sizeof(block_metadata_t) + b->block_size;
    free(b);
}