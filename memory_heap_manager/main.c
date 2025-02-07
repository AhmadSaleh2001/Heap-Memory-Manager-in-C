#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "memory_manager.h"
#include "block_metadata.h"

typedef struct std_
{
    char name[10];
    int age;
} std_t;

typedef struct emp_
{
    char emp_name[10];
    int salary;
    int eid;
} emp_t;

int main() {

    init_mmap();
    MM_REGISTER_STRUCT(std_t);
    MM_REGISTER_STRUCT(emp_t);

    mm_print_registered_page_families();
    vm_page_family_t * stds = lookup_page_family_by_name("std_t");
    vm_page_t * vm1 = mm_allocate_vm_page(stds);
    vm_page_t * vm2 = mm_allocate_vm_page(stds);

    vm_page_t * curr = NULL;
    ITERATE_VM_PAGES_BEGIN(stds->first_vm_page, curr) {
        block_metadata_t * curr_block = NULL;
        ITERATE_VM_PAGE_BLOCKS_BEGIN(curr->blocks, curr_block) {
            print_block_metadata(curr_block);
            printf("\n");
        } ITERATE_VM_PAGE_BLOCKS_END(curr->blocks, curr_block)
    } ITERATE_VM_PAGES_END(vm_page_families->first_vm_page, curr)

    return 0;
}