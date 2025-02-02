#include <stdlib.h>
#include <stdio.h>
#include "vm_page_family.h"
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
    vm_page_families_t * vm_page_families = get_vm_page_families();
    vm_page_t * vm1 = mm_allocate_vm_page(vm_page_families);
    vm_page_t * vm2 = mm_allocate_vm_page(vm_page_families);

    vm_page_t * curr = NULL;
    ITERATE_VM_PAGES_BEGIN(vm_page_families->first_vm_page, curr) {
        printf("page\n");
    } ITERATE_VM_PAGES_END(vm_page_families->first_vm_page, curr)

    return 0;
}