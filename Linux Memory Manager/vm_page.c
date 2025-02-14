#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include <memory.h>
#include "block_metadata.h"
#include "vm_page.h"

// ############ FUNCTIONS FOR VM_PAGE_T ############

bool mm_is_page_free(vm_page_t * vm_page) {
    return vm_page->blocks[0].is_free == true && vm_page->prev == NULL && vm_page->next == NULL;
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
            total+=current_block_metadata->is_free == false;
        } ITERATE_VM_PAGE_BLOCKS_END(vm_page->blocks, current_block_metadata)
        vm_page = vm_page->next;
    } ITERATE_VM_PAGES_END(vm_page, current_vm_page)
    return total;
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

// ############ FUNCTIONS FOR VM_PAGE_T ############

// ############ FUNCTIONS FOR VM_PAGE_FAMILIY ############

void print_page_family_info(vm_page_family_t* vm_page_family) {
    printf("struct name: %s\n", vm_page_family->struct_name);
    printf("struct size: %d\n", vm_page_family->size);
}

// ############ FUNCTIONS FOR VM_PAGE_FAMILIY ############