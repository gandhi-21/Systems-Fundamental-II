/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"
#include "custom.h"
#include <errno.h>

/**
Helper variables for the program
*/
void *memstart;
sf_block *sf_prologue_ptr;
sf_block *wilderness_block;
sf_block *sf_epilogue;



// /**
// Helper functions for the program
// */

/**
round the request size of memory to the nearest 64
*/
size_t roundTo64(size_t size)
{
 //   debug("rounding requested size to the nearest multiple of 64");
    int remainder = size % 64;
    if(remainder == 0)
        return size;
    return size + 64 - remainder;
}

/**
get the size class of the requested memory
*/
int getIndexInList(size_t bytesRequested)
{
    if (bytesRequested == 64)
        return 0;
    else if (bytesRequested == 128)
        return 1;
    else if (bytesRequested == 192)
        return 2;
    else if (bytesRequested > 192 && bytesRequested <= 320)
        return 3;
    else if (bytesRequested > 320 && bytesRequested <= 512)
        return 4;
    else if (bytesRequested > 512 && bytesRequested <= 832)
        return 5;
    else if (bytesRequested > 832 && bytesRequested <= 1344)
        return 6;
    else if (bytesRequested > 1344 && bytesRequested <= 2176)
        return 7;
    else return 8;
}

/**
initialize all the free lists and make the pointers point to themselves for the heads of the lists
*/
void initFreeLists()
{
   // debug("starting to initialize the free lists");
    for(int i=0;i<NUM_FREE_LISTS;i++)
    {
        sf_free_list_heads[i].body.links.next = &sf_free_list_heads[i];
        sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];
    }
   // debug("finished initializing the free lists");
}



/**
write the prologue at initialization of the allocator
*/
void writePrologue(sf_block *prologue_ptr)
{
    prologue_ptr->header = 64 + 3;
}

/**
write the epilogue whenever required
*/
void writeEpilogue(sf_block *epilogue_ptr, sf_block *prev_block)
{
    epilogue_ptr->header = 1;
    epilogue_ptr->prev_footer = prev_block->header;
}

/**
Initialize the heap and the contents
*/
int initHeap()
{
    initFreeLists();

    if(sf_mem_grow() == NULL)
        return 0;

    memstart = sf_mem_start();
    // add 48 to get to the sf_prologue_header
    memstart += 48;
    sf_prologue_ptr = (sf_block *) memstart;
    // set the size of the prologue by setting the header
    writePrologue(sf_prologue_ptr);

    // add 64 to get to the wilderness block
    memstart += 64;
    wilderness_block = (sf_block *) memstart;
    wilderness_block->prev_footer = sf_prologue_ptr->header;
    wilderness_block->header = 3968 + 2;

    // add the wilderness block to the free lists
    wilderness_block->body.links.next = &sf_free_list_heads[9];
    wilderness_block->body.links.prev = &sf_free_list_heads[9];
    sf_free_list_heads[9].body.links.next = wilderness_block;
    sf_free_list_heads[9].body.links.prev = wilderness_block;

    // add 3968 to get to the epilogue
    memstart += 3968;
    sf_epilogue = (sf_block *) memstart;
    writeEpilogue(sf_epilogue, wilderness_block);
    return 1;

}

// /**
// Function to add a block to the free lists
// */
// void addBlockToList(sf_block *block, int index)
// {
//     sf_block *temp = &sf_free_list_heads[index];

// }

/**
finding the appropriate block to return by memory request
change return type
*/
sf_block *searchFreeLists(size_t size, int index)
{
    do {
        sf_block *temp = sf_free_list_heads[index].body.links.next;
        while(temp->body.links.next != &sf_free_list_heads[index])
        {
            size_t size_block = (temp->header & BLOCK_SIZE_MASK); 
            if(size_block >= size)
            return temp;
            temp = temp->body.links.next;
        }
        // debug("searched index %d", index);
        index++;
    } while(index != 9);
    return NULL;
}

/**
Function to check if wilderness block can be used
*/
sf_block *checkWildernessBlock(size_t size)
{
    sf_block *temp = sf_free_list_heads[NUM_FREE_LISTS - 1].body.links.next;
    //debug(" current wilderness at check wilderness %p", temp);
    size_t size_block = (temp->header & BLOCK_SIZE_MASK);
    if(size_block >= size)
    return temp;
    else 
    return NULL;
}

/**
Function to add a block to the free list
*/
void addNewBlockToList(sf_block *block, int index)
{
    if(sf_free_list_heads[index].body.links.next == &sf_free_list_heads[index])
    {
        // Empty list adjust accordingly
        debug("adding to start of list");
        sf_free_list_heads[index].body.links.next = block;
        sf_free_list_heads[index].body.links.prev = block;

        block->body.links.next = &sf_free_list_heads[index];
        block->body.links.prev = &sf_free_list_heads[index];

    } else {
        // Not an ampty list store the current head->next
        debug("not adding to empty list");
        sf_block *temp = sf_free_list_heads[index].body.links.next;
        sf_free_list_heads[index].body.links.next = block;
        block->body.links.next = temp;
        block->body.links.prev = &sf_free_list_heads[index];
        temp->body.links.prev = block;
    }
}


/**
Function to grow the heap by a page and add the bytes to the wilderness block
*/
int grow_heap()
{
    sf_epilogue = sf_mem_end() - 16;
    void *new_memstart = sf_mem_grow();
    if(new_memstart == NULL)
    {
        // No more memory left
        return 0;
    }
    // Convert the epilogue to become the new header of the page
    
    // Coalesce both the new blocks
        sf_epilogue->header = 4096;
        sf_block * wilderness = sf_free_list_heads[9].body.links.next;
        //debug("current size of wilderness is %lu", wilderness->header);
//        if(sf_free_list_heads[9].body.links.next != &sf_free_list_heads[9])
        wilderness->header +=  4096;
        // Write the new epilogue
        sf_epilogue = (sf_block*) (new_memstart + 4080);
        writeEpilogue(sf_epilogue, wilderness);
    
    return 1;
}

/**
Function to use the wilderness block to allocate memory
*/
sf_block *allocateWilderness(size_t sizeRequested)
{
    sf_block *wilderness = wilderness_block;
    //debug("current wilderness %p", wilderness);
    void *new_wilderness_ptr = wilderness;
    new_wilderness_ptr += (sizeRequested);

    sf_block *new_wilderness = (sf_block *) new_wilderness_ptr;

    new_wilderness->prev_footer = sizeRequested + 1;
    new_wilderness->header = wilderness->header - sizeRequested;

    wilderness->header = sizeRequested + 3;

    // Add the new wilderness to the list;
    //debug("changing the wilderness block in the list");
    //debug("size of the new wilderness %lu", (new_wilderness->header & BLOCK_SIZE_MASK));
        new_wilderness->body.links.next = &sf_free_list_heads[9];
        new_wilderness->body.links.prev = &sf_free_list_heads[9];
        sf_free_list_heads[9].body.links.next = new_wilderness;
        sf_free_list_heads[9].body.links.prev = new_wilderness;

        sf_block * epilogue = sf_mem_end() - 16;
        //debug("epilogue %p", epilogue);
        writeEpilogue(epilogue, new_wilderness);
         
        if((new_wilderness->header & BLOCK_SIZE_MASK) == 0)
         {
            sf_free_list_heads[9].body.links.next = &sf_free_list_heads[9];
            sf_free_list_heads[9].body.links.prev = &sf_free_list_heads[9];
         }

    wilderness_block = new_wilderness;

    return wilderness;
}

/**
Function to use a normal block to allocate memory
*/
sf_block *allocateBlock(sf_block *block, size_t sizeRequested)
{
    if(((block->header & BLOCK_SIZE_MASK) - sizeRequested) >= 64)
    {
        // Splinter split the block and return the newly allocated block

        block->body.links.prev->body.links.next = block->body.links.next;
        block->body.links.next->body.links.prev = block->body.links.prev;

        size_t new_size = (block->header & BLOCK_SIZE_MASK)- sizeRequested;

        block->header = new_size;
        // change block prev allocated 
        void *next_block = block;
        next_block += new_size;

        sf_block *allocated_block = (sf_block *)next_block;
        allocated_block->header = sizeRequested + 1;
        allocated_block->prev_footer = block->header;

        int index = getIndexInList(new_size);
        addNewBlockToList(block, index);

        return allocated_block;
    } else {
        // change the header to indicate it is allocated
        block->header = block->header + 1;
        // change the prev_footer of the next block to indicate that the prev_block is now allocated
        sf_block *next_block = block + 1;
        next_block->prev_footer = block->header;
        
        // remove the block from the list
        sf_block *temp = block->body.links.prev;
        temp->body.links.next = block->body.links.next;
        block->body.links.next->body.links.prev = temp;
        // return the block
        return block;
    }
}

/*
Helper functions for the sf_free
************************************************************************************************************
*/
int validate_pointer(void *pp)
{

    sf_block *curr_block = pp - 16;
    sf_block *next_block = (pp - 16) + (curr_block->header & BLOCK_SIZE_MASK);


    debug("got curr and next block");

    if(pp == NULL)
    return -1;
    else if(((size_t)pp)%64 != 0)
    return -1;
    else if((curr_block->header & THIS_BLOCK_ALLOCATED) != 1)
    return -1;
    else if(((size_t)next_block) < ((size_t)(sf_mem_start() + 112)))
    return -1;
    else if(((size_t)(next_block->prev_footer)) > ((size_t)(sf_mem_end() - 16)))
    return -1;
    else if((curr_block->header & PREV_BLOCK_ALLOCATED) == 2)
    {
        if((curr_block->prev_footer & THIS_BLOCK_ALLOCATED) != 1)
        return -1;
        else
        return 0;
    }

    return 0;
}


// To do 
// check if the wilderness is coalesced 
// remove free blocks from the list


sf_block *coalesce_blocks(void *pp)
{
    // current_block
    sf_block *curr_block = pp - 16;
    // previous_block
    //sf_block *prev_block = (sf_block*) pp - 16 - (curr_block->prev_footer & BLOCK_SIZE_MASK);
    //next_block
    sf_block *next_block = pp - 16 + (curr_block->header & BLOCK_SIZE_MASK); 

    // get the prev_block allocated
    size_t prev_allocated = curr_block->header & PREV_BLOCK_ALLOCATED;
    
    if(prev_allocated == 2)
        prev_allocated = 1;
    else
        prev_allocated = 0;
    // get the next_block allocated
    size_t next_allocated = next_block->header & THIS_BLOCK_ALLOCATED;
    // get the size of the current block
    //size_t current_size = (curr_block->header & BLOCK_SIZE_MASK);

    //debug("current size %lu", curr_block->header & BLOCK_SIZE_MASK);
    debug("next size is %lu", next_block->header & BLOCK_SIZE_MASK);
    //debug("size of prev_block %lu", prev_block->header & BLOCK_SIZE_MASK);
    //debug("prev allocated is %lu", prev_allocated);
    //debug("next allocated ia %lu", next_allocated);

    // case 1
    if (prev_allocated && next_allocated)
    {
        debug("prev and next are both allocated");
        curr_block->header = curr_block->header - 1;
        next_block->prev_footer = curr_block->header;
        if((next_block->header & BLOCK_SIZE_MASK) > 2)
        next_block->header = next_block->header - 2;
        return curr_block;
    } else if(prev_allocated && !next_allocated)
    // case 2
    {
        // coalesce the next block
        debug("next block is also free");
        size_t next_block_size = next_block->header & BLOCK_SIZE_MASK;
        debug("next block size is %lu", next_block_size);
        curr_block->header = curr_block->header + next_block_size - 1;
        sf_block *next_next_block = pp - 16 + (curr_block->header & BLOCK_SIZE_MASK);
        next_next_block->prev_footer = curr_block->header;
        debug("new block size is %lu", curr_block->header & BLOCK_SIZE_MASK);

        // remove the next block from the free list
        // sf_block *temp = next_block->body.links.prev;
        // temp->body.links.next = next_block->body.links.next;
        // next_block->body.links.next->body.links.prev = temp;
        next_block->body.links.prev->body.links.next = next_block->body.links.next;
        next_block->body.links.next->body.links.prev = next_block->body.links.prev;

        return curr_block;

    } else if(!prev_allocated && next_allocated)
    // case 3
    {
        // coalesce the prev block
        debug("prev block is free");
        sf_block *prev_block = (sf_block*) pp - 16 - (curr_block->prev_footer & BLOCK_SIZE_MASK);

        if((prev_block->header & PREV_BLOCK_ALLOCATED) == 2)
        prev_block->header = (prev_block->header & BLOCK_SIZE_MASK) + (curr_block->header & BLOCK_SIZE_MASK) + 2;
        else
        prev_block->header = prev_block->header + curr_block->header;
        next_block->prev_footer = prev_block->header;
        debug("prev block is also free %lu", prev_block->header & BLOCK_SIZE_MASK);

        // remove the prev block from the free list
        // sf_block *temp = prev_block->body.links.prev;
        // temp->body.links.next = prev_block->body.links.next;
        // prev_block->body.links.next->body.links.prev = temp;
        prev_block->body.links.prev->body.links.next = prev_block->body.links.next;
        prev_block->body.links.next->body.links.prev = prev_block->body.links.prev;

        return prev_block;
    } else 
    // case 4
    {
        // coalesce previous and next block
        debug("prev and next block free");
        sf_block *prev_block = pp - 16 - (curr_block->prev_footer & BLOCK_SIZE_MASK);
        debug("found prev block");
        prev_block->header = prev_block->header + (curr_block->header & BLOCK_SIZE_MASK) + (next_block->header);
        debug("changed prev header");
        sf_block *next_next_block = pp - 16 + (curr_block->header & BLOCK_SIZE_MASK) + (next_block->header & BLOCK_SIZE_MASK);
        debug("found next to next ");
        next_next_block->prev_footer = prev_block->header;
        debug("changed prev footer for next to next");
        debug("value of prev footer is %lu", next_next_block->prev_footer);
        
        debug("removing next block from list");
        next_block->body.links.prev->body.links.next = next_block->body.links.next;
        next_block->body.links.next->body.links.prev = next_block->body.links.prev;


        debug("removing prev block from the list");
        prev_block->body.links.prev->body.links.next = prev_block->body.links.next;
        prev_block->body.links.next->body.links.prev = prev_block->body.links.prev;

        debug("returning prev block");
        return prev_block;
    }

    return NULL;
}


/**
End of helper functions for the dynamic memory allocator
*************************************************************************************************************
*/

void *sf_malloc(size_t size) {
    
    if(sf_mem_start() == sf_mem_end())
    {
        if(initHeap() == 0)
        return NULL;
    }

    if(size <= 0)
    return NULL;

    size_t correct_size = roundTo64(size + 8);
    int indexToStart = getIndexInList(correct_size);

    //debug("index to start looking from %d", indexToStart);
    //debug("correct size requested %ld ", correct_size);
    sf_block *block = searchFreeLists(correct_size, indexToStart);

    if(block == NULL)
    {

        while(checkWildernessBlock(correct_size) == NULL)
        {
            int value = grow_heap();
            //debug("growing heap");
            if(value == 0)
            {
                debug("no more memory, malloc too large ********************");
                sf_errno = ENOMEM;
                debug("returning after setting sf_errno ");
                return NULL;
            }
        }
        //debug("need to use wilderness block");
        block = checkWildernessBlock(correct_size);
        {
           // debug("%p", block);
            sf_block *allocated = allocateWilderness(correct_size);   
            //debug("allocated %lu", (allocated->header & BLOCK_SIZE_MASK));
            // sf_show_free_lists();
            //sf_show_heap(); 
            return allocated->body.payload;
        }
    }
    else {
        //debug("found a good enough block which is stored in block variable");
        sf_block *allocated = allocateBlock(block, correct_size);
        return allocated->body.payload;
    }

    //debug("done with show heap");
    return NULL;
}

void sf_free(void *pp) {
    int valid = validate_pointer(pp);
    //debug("pointer is valid : %d", valid);
    if(valid == -1)
    abort();
    sf_block *new_free_block = coalesce_blocks(pp);
    //debug("new free block needs to be added to the appropriate free lists");

    // check if this block is the new wilderness 
    sf_block *sf_epilogue = sf_mem_end() - 16;
    debug("%lu", sf_epilogue->prev_footer & BLOCK_SIZE_MASK);
    void *start_wilderness = sf_mem_end() - 16 - (sf_epilogue->prev_footer & BLOCK_SIZE_MASK);
    debug("%p", start_wilderness);
    debug("%p", new_free_block);
    if((size_t)start_wilderness == (size_t)new_free_block) 
    { debug("freed wilderness block");
    new_free_block->body.links.next = &sf_free_list_heads[9];
    new_free_block->body.links.prev = &sf_free_list_heads[9];
    sf_free_list_heads[9].body.links.next = new_free_block;
    sf_free_list_heads[9].body.links.prev = new_free_block;
    }
    else 
    {
        debug("freed regular block");
        int index = getIndexInList(new_free_block->header & BLOCK_SIZE_MASK);
        addNewBlockToList(new_free_block, index);
    }
    debug("%p", new_free_block);
    sf_show_heap();

    return;
}

void *sf_realloc(void *pp, size_t rsize) {
    return NULL;
}

void *sf_memalign(size_t size, size_t align) {
    return NULL;
}
