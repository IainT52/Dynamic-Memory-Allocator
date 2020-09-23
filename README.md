Dynamic Memory Allocator: Multi-Pool
===

A simple Dynamic Memory Allocator that implements malloc() calloc() realloc() <br>
and free() as multi-pool allocators. Meaning they reuse blocks of memory previously <br>
requested in a system call (sbrk()). They will only request more memory from the <br>
operating system when they have no more blocks of memory in their current "pool". <br>

Unfortunately, there is no visualization to view how memory blocks are being <br>
allocated and called from the operating system. However, as a future project I may try <br>
to incorporate a visualization using javascript and some helpful plugins. <br>
