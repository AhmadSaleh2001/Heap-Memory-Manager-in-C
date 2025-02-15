# Heap-Memory-Manager-in-C

In any C program, when dealing with heap memory, we typically rely on functions like malloc, calloc, and free. These functions are managed by the glibc library, which handles memory allocation by requesting pages from the kernel—usually in chunks of 16KB—and reusing them efficiently.

In this project, our goal is to implement similar functionality to what glibc provides, but using our own custom functions: xmalloc, xcalloc, and xfree.

In other words, instead of using malloc, calloc, or free, we will build our own memory manager for Linux, utilizing mmap and munmap for memory allocation and deallocation.