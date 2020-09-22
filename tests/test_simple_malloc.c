#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define ALLOC_SIZE 1016


int main(int argc, char *argv[])
{
    void *p1, *p2, *p3, *p4, *p5, *brk1;

    /* First request an allocation of ALLOC_SIZE bytes to cause the
     * allocator to fetch some memory from the OS and "prime" this
     * allocation size block. */
    p1 = malloc(ALLOC_SIZE);

    /* Call sbrk(0) to retrieve the current program break without
     * changing it.  When we allocation again, this value should not
     * change, because the allocator should not fetch more memory from
     * the OS. */
    brk1 = sbrk(0);

    /* Request a second allocation of ALLOC_SIZE, which should simply
     * take an allocation block off the list for ALLOC_SIZE. */
    p2 = malloc(ALLOC_SIZE);
    if(*(size_t *)(p2 - 8) != 1024){
        fprintf(stderr, "failed actual size: %ld\n",*(size_t *)(p2 - 8));
    }
    p3 = calloc(254,sizeof(int));
    if(*(size_t *)(p3 - 8) != 1024){
        fprintf(stderr, "failed c actual size: %ld\n", *(size_t *)(p3 - 8));
    }
    for(int i = 0; i < 1016; i++){
        if(*(char *)p3 != '\0'){
            fprintf(stderr, "failed at byte: %d\n", i);
        }
        p3 += 1;
    }
    p4 = malloc(ALLOC_SIZE);
    free(p4);
    p4 = malloc(ALLOC_SIZE);
    p5 = realloc(p4,1000);
    free(p5);

    /* This should never run -- but it causes p1 and p2 to be "used", so
     * the compiler doesn't fail. */
    if (brk1 == NULL) {
        fprintf(stderr, "sbrk() failed: %p %p\n", p1, p2);
    }

    /* Compare the new program break to the old and make sure that it
     * has not changed. */
    if (brk1 == sbrk(0)) {
        return 0;
    } else {
        return 1;
    }
}
