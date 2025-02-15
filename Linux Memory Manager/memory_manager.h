#pragma once
#include "vm_page_families.h"
#include "vm_page.h"
#include "block_metadata.h"

#define MM_REGISTER_STRUCT(struct_name) \
    mm_instantiate_vm_page_family(#struct_name, sizeof(struct_name));

#define offset_of(container_structure, field_name)  \
    ((size_t)&(((container_structure *)0)->field_name))

void * xmalloc(char * struct_name);
void * xcalloc(char * struct_name, int units);
void * xfree(void * block_metadata);

void init_mmap();
vm_page_families_t * get_vm_page_families();
void mm_instantiate_vm_page_family(char * struct_name, int size);
void mm_print_registered_page_families();
vm_page_family_t * lookup_page_family_by_name(char *struct_name);
block_metadata_t * mm_allocate_block_metadata(vm_page_family_t * vm_page_family, block_metadata_t * free_block, int units);
block_metadata_t * first_fit_page(vm_page_family_t * vm_page_family, int units);
block_metadata_t * worst_fit_page(vm_page_family_t * vm_page_family, int units);
void print_memory_status();
void print_memory_status_using_glthreads();
void print_memory_usage();