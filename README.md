Dynamic Memory Allocator: Multi-Pool
===

A simple Dynamic Memory Allocator that implements malloc() calloc() realloc()
and free() as multi-pool allocators. Meaning they reuse blocks of memory previously requested in a system call (sbrk()). They will only request more memory from the operating system when they have no more blocks of memory in their current "pool".
