#include <stdlib.h>
#include <stdio.h>
#include "memory_manager.h"

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

    return 0;
}