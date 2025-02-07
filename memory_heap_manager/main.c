#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
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

void print_employee_info(emp_t * emp){
    printf("emp id: %d\n", emp->eid);
    printf("emp name: %s\n", emp->emp_name);
    printf("emp salary: %d\n", emp->salary);
}

int main() {

    init_mmap();
    MM_REGISTER_STRUCT(std_t);
    MM_REGISTER_STRUCT(emp_t);
    // mm_print_registered_page_families();


    emp_t * emp = xmalloc("emp_t");
    emp->eid = 1;
    memcpy(emp->emp_name, "ahmad", 5);
    emp->salary = 3000;


    emp_t * emps = xcalloc("emp_t", 5);
    for(int i=0;i<5;i++) {
        emps[i].eid = i + 100;
        memcpy(emps[i].emp_name, "aaaa", 4);
        emps[i].salary = 4000 + i;
    }

    for(int i=0;i<5;i++) {
        print_employee_info(&emps[i]);
        printf("\n");
    }

    vm_page_family_t * page_family = lookup_page_family_by_name("emp_t");
    print_vm_pages(page_family);

   
    return 0;
}