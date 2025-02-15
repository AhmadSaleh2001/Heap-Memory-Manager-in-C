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
    mm_print_registered_page_families();

    // std_t * std = xmalloc("std_t");
    // std->age = 19;
    // memcpy(std->name, "ali", 3);
    
    emp_t * all_emps_ptrs[10000];

    for(int i=0;i<10000;i++) {
        emp_t * emps = xcalloc("emp_t", i%30 + 1);
        all_emps_ptrs[i] = emps;
    }


    // emp_t * emps = xcalloc("emp_t", 5);
    // for(int i=0;i<5;i++) {
    //     emps[i].eid = i + 100;
    //     memcpy(emps[i].emp_name, "aaaa", 4);
    //     emps[i].salary = 4000 + i;
    // }

    print_memory_status_using_glthreads();
    // print_memory_status();
    print_memory_usage();

    // xfree(std);
    // xfree(emp);
    // xfree(emps);
    for(int i=0;i<10000;i++) {
        xfree(all_emps_ptrs[i]);
    }

    print_memory_status_using_glthreads();
    // print_memory_status();
    print_memory_usage();
   
    return 0;
}