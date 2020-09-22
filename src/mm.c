#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

/*Init*/
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);

/* Constant to request memory from the OS in increments of CHUNK_SIZE. */
#define CHUNK_SIZE (1<<12)

/*
 *This function is defined in bulk.c and
 * provided to me by Professor Blanton.
 */
extern void *bulk_alloc(size_t size);

/*
 *This function is defined in bulk.c and
 * provided to me by Professor Blanton.
 */
extern void bulk_free(void *ptr, size_t size);

/*
 * This function computes the log base 2 of the allocation block size
 * for a given allocation.  To find the allocation block size from the
 * result of this function, use 1 << block_size(x).
 * This function was also provided to me by Professor Blanton
 */
static inline __attribute__((unused)) int block_index(size_t x) {
    if (x <= 8) {
        return 5;
    } else {
        return 32 - __builtin_clz((unsigned int)x + 7);
    }
}


/* Header struct to be used for free blocks of memory*/
typedef struct header {
    size_t *size;
    struct header *next;
} header;

/*Free list*/
static struct header **FREE_LIST = NULL;

/*This function adds a flag to the lowest bit of an int
 * representing whether the block is free or not
 */

void *freeAllocator(){
    void *chunk = sbrk(CHUNK_SIZE);
    memset(chunk,'\0',CHUNK_SIZE);
    FREE_LIST = (struct header **)chunk;
    void *freeBlocks = (void *)(chunk + 128);
    //Insert first block into Free List
    struct header *firstBlock = (struct header *)freeBlocks;
    firstBlock->size = (size_t *)(size_t)128;
    struct header *prevNode = firstBlock;
    FREE_LIST[7] = firstBlock;
    //Create linked list of free blocks
    freeBlocks += 128;
    for(int i = 0; i < (CHUNK_SIZE/128) - 2; i++){
        struct header *header = (struct header *)freeBlocks;
        header->size = (size_t *)(size_t)128;
        header->next = NULL;
        prevNode->next = header;
        prevNode = header;
        freeBlocks += 128;
    }
    return chunk;
}


/*
 * This function will retrieve a block of memory from the OS and
 * break up the blocks according to the given size and power of 2;
 * Returning the first block and adding the rest to free list.
*/

static void *block_request(int size, int exponent){
    void *chunk = sbrk(CHUNK_SIZE);
    int numOf_blocks = (CHUNK_SIZE/size);
    if(numOf_blocks > 1){
        void *freeBlocks = (void *)(chunk + size);
        //Insert first block into Free List
        struct header *firstBlock = (struct header *)freeBlocks;
        firstBlock->size = (size_t *)(size_t)size;
        struct header *prevNode = firstBlock;
        FREE_LIST[exponent] = firstBlock;
        //Create linked list of free blocks
        freeBlocks += size;
        for(int i = 0; i < numOf_blocks - 2; i++){
            struct header *header = (struct header *)freeBlocks;
            header->size = (size_t *)(size_t)size;
            header->next = NULL;
            prevNode->next = header;
            prevNode = header;
            freeBlocks += size;
        }
    }
    //return the first block of the allocation
    struct header *chunkHeader = (struct header *)chunk;
    chunkHeader->size = (size_t *)(size_t)size;
    //returning pointer after header
    return chunk + 8;
}


void *malloc(size_t size) {
    if (size == 0) return NULL;
    //Allocate memory for free if true
    if(FREE_LIST == NULL){
        void * newBlock = freeAllocator();
        struct header **structHead = (struct header **)newBlock;
        FREE_LIST = structHead;
    }
    size_t blockSize = size + 8;
    //Check that size is less than 4088 Bytes
    if(blockSize <= CHUNK_SIZE){
        int powerSize = block_index(size);
        int allocSize = 1 << powerSize;
        //Check if there is a free block
        if(FREE_LIST[powerSize] != NULL){
            struct header *block = FREE_LIST[powerSize];
            //Replace head of free list
            FREE_LIST[powerSize] = block->next;
            //insert header and return pointer
            return ((void *)block + 8);
        }
        //Get more memory from OS to allocate
        else{
            return block_request(allocSize,powerSize);
        }
    }
    //Call the bulk allocator
    else{
        struct header *bulk = (struct header *)bulk_alloc(blockSize);
        bulk->size = (size_t *)(blockSize);
        return (void *)(bulk + 8);
    }
}



void *calloc(size_t nmemb, size_t size) {
    size_t totalSize = nmemb * size;
    void *userPtr = malloc(totalSize);
    memset(userPtr,0,totalSize);
    return userPtr;
}



void *realloc(void *ptr, size_t size) {
    //NULL pointer and 0 size checks/handling
    if(ptr == NULL){return malloc(size);}
    if(size == 0){
        free(ptr);
        return NULL;
    }
    //Get orginal size (excluding header) from ptr
    size_t originalSize = (*(size_t *)(ptr - 8) - 8);
    //Check if realloc can return original pointer
    if(size <= originalSize){
        return ptr;
    }
    //Allocate new memory and copy memory over
    else{
        void *newMem = malloc(size);
        //Copy memory to newly allocated block
        memcpy(newMem,ptr,originalSize);
        //Free old memory
        free(ptr);
        return newMem;
    }
}


void free(void *ptr) {
    //Check for bulk allocation
    if(ptr != NULL){
        size_t size = *(size_t *)(ptr-8);
        if(size == 0){
            return;
        }
        if(size > CHUNK_SIZE){
            bulk_free(ptr - 8, size);
        }
        else{
            //Insert freed block into free list
            int powerSize = block_index(size - 8);
            struct header *freeBlock = (struct header *)(ptr - 8);
            freeBlock->next = NULL;
            if(FREE_LIST[powerSize] != NULL){
                struct header *curNode = FREE_LIST[powerSize];
                for(int i = 0; curNode->next != NULL; i++){
                    curNode = curNode->next;
                }
                curNode->next = freeBlock;
            }
            else{
                FREE_LIST[powerSize] = freeBlock;
            }
        }
    }
}
