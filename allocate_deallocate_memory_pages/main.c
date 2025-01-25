#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include <memory.h>

static int32_t SYSTEM_PAGE_SIZE = 0;

static void init_mmap() {
    SYSTEM_PAGE_SIZE = getpagesize();
}

static void * mm_get_new_vm_page_from_kernel(int number_of_pages) {
    char * vm_page = mmap(
        0,  // pass it as null currently
        number_of_pages * SYSTEM_PAGE_SIZE, // number of memory pages
        PROT_READ|PROT_WRITE, // permissions on the allocated pages
        MAP_ANON|MAP_PRIVATE,
        0,
        0
    );
    if (vm_page == MAP_FAILED) {
        perror("failed to allocate pages");
        exit(1);
    }

    memset(vm_page, 0, SYSTEM_PAGE_SIZE);
    return vm_page;
}

static void mm_return_vm_page_to_kernel(char * vm_page, int number_of_pages) {
    if(munmap(vm_page, number_of_pages*SYSTEM_PAGE_SIZE)) {
        printf("failed to deallocate vm page");
        exit(1);
    }
}

int main() {

    init_mmap();
    printf("page size: %d\n", SYSTEM_PAGE_SIZE);

    void * vm_page1 = mm_get_new_vm_page_from_kernel(1);
    void * vm_page2 = mm_get_new_vm_page_from_kernel(1);

    printf("address for vm page 1: %p\n", vm_page1);
    printf("address for vm page 2: %p\n", vm_page2);

    mm_return_vm_page_to_kernel(vm_page1, 1);
    mm_return_vm_page_to_kernel(vm_page2, 1);

    return 0;
}