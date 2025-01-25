#include "vm_page_families.h"

void print_vm_page_families(vm_page_families_t * vm_page_families) {
    vm_page_families_t * curr_page_families = vm_page_families;
    while(curr_page_families) {
        printf(" ----- Printing vm page families ----- \n");

        vm_page_family_t * curr_page_family = NULL;
        ITERATE_PAGE_FAMILY_BEGIN(curr_page_families->vm_page_family, curr_page_family) {
            print_page_family_info(curr_page_family);
            printf("\n");
        }ITERATE_PAGE_FAMILY_END(curr_page_families, curr_page_family)

        printf(" ----- Printing vm page families ----- \n");

        curr_page_families = curr_page_families->next;
    }
}