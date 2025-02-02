#include <stdlib.h>
#include <stdio.h>
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

    vm_page_family_t * search = lookup_page_family_by_name("std_t");
    printf("%p\n", search);

    return 0;
}