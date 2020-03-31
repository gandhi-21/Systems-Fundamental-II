/**
This is a file to define custom helper functions 
*/
#ifndef custom
#define custom

// Function to round the size to the nearest 64 
size_t roundTo64(size_t size);
// Function to get the size class index for the requested size in the free lists
int getIndexInList(size_t bytesRequested);
// Function to write the prologue when initializing the heap
void writePrologue();
// Function to write the epilogue when initializing the heap or adding pages to the wilderness block
void writeEpilogue();
// Function to search the appropriate block to return from malloc
sf_block *searchFreeLists(size_t size, int index);
// Function to initialize all the free lists and make the pointers point to themselves for the heads of the lists
void initFreeLists();
// Function to initialize the heap
int initHeap();
// Function to check if wilderness block is large enough
sf_block *checkWildernessBlock(size_t size);
// Function to add a block to the free list
void addNewBlockToList(sf_block *block, int index);
// Function to grow the heap 
int grow_heap();
// Function to use a regular block to allocate
sf_block *allocateBlock(sf_block *block, size_t sizeRequested);
// Function to use the wilderness block to allocate
sf_block *allocateWilderness(size_t sizeRequested);

#endif