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
    // mm_print_registered_page_families();

    vm_page_family_t * stds = lookup_page_family_by_name("std_t");
    vm_page_t * vm1 = mm_allocate_vm_page(stds);
    vm_page_t * vm2 = mm_allocate_vm_page(stds);

    int number_of_items = 341;
    for(int i=0;i<number_of_items;i++) {
        block_metadata_t * first_free_enough_block = get_first_empty_block(stds);
        if(first_free_enough_block != NULL){
            mm_add_free_block_metadata_to_free_block_list(stds, first_free_enough_block);
        }
    }
   
    return 0;
}