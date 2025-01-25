#include "vm_page_families.h"

void print_vm_page_families(vm_page_families_t * vm_page_families) {
    vm_page_families_t * current_page_families = NULL;
    ITERATE_PAGE_FAMILIES_BEGIN(vm_page_families, current_page_families) {
        vm_page_family_t * current_page_family = NULL;
        ITERATE_PAGE_FAMILY_BEGIN(current_page_families->vm_page_family, current_page_family) {
            print_page_family_info(current_page_family);
        } ITERATE_PAGE_FAMILY_END(current_page_families, current_page_family)
    } ITERATE_PAGE_FAMILIES_END(vm_page_families, current_page_families, current_page_family)
}