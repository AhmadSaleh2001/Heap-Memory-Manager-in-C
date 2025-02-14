#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include <memory.h>
#include "vm_page_families.h"

void print_vm_page_families(vm_page_families_t * vm_page_families);