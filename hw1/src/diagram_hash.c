#include "const.h"
#include "sequitur.h"

/*
 * Digram hash table.
 *
 * Maps pairs of symbol values to first symbol of digram.
 * Uses open addressing with linear probing.
 * See, e.g. https://en.wikipedia.org/wiki/Open_addressing
 */

/**
 * Clear the digram hash table.
 */
void init_digram_hash(void) {
    // To be implemented.
    for(int i=0;i<MAX_DIGRAMS;i++)
    {
        *(digram_table + i) = NULL;
    }

}

/**
 * Look up a digram in the hash table.
 *
 * @param v1  The symbol value of the first symbol of the digram.
 * @param v2  The symbol value of the second symbol of the digram.
 * @return  A pointer to a matching digram (i.e. one having the same two
 * symbol values) in the hash table, if there is one, otherwise NULL.
 */
SYMBOL *digram_get(int v1, int v2) {
    // To be implemented.

   // debug("get dig value");
    int value = DIGRAM_HASH(v1, v2);
   // debug("got diag value %d ", value);

    for(int i=value;i<MAX_DIGRAMS;i++)
    {
        SYMBOL *daig = *(digram_table + i);
     // debug("found index in digram table values %d and %d ", daig->value, daig->next->value);

        if(*(digram_table + i) == TOMBSTONE)
            continue;

        if(*(digram_table + i) == NULL)
        {
        //    debug("daig is null");
            return NULL;
        }

        if(daig->value == v1 && daig->next->value == v2)
        { 
        //     debug("found diag value %d", i);
            return *(digram_table + i);
        }
    
    } 

    // Did not find it till the end
    for(int i=0;i<value;i++)
    {
        SYMBOL *daig = *(digram_table + i);

        if(*(digram_table + i) == TOMBSTONE)
            continue;


        if(daig == NULL)
            {
           //     debug("diag is null from 0");
                return NULL;}
        if(daig->value == v1 && daig->next->value == v2)
            {
           //     debug("found diag value %d", i);
                return *(digram_table + i);}
    }

    return NULL;
}

/**
 * Delete a specified digram from the hash table.
 *
 * @param digram  The digram to be deleted.
 * @return 0 if the digram was found and deleted, -1 if the digram did
 * not exist in the table.
 *
 * Note that deletion in an open-addressed hash table requires that a
 * special "tombstone" value be left as a replacement for the value being
 * deleted.  Tombstones are treated as vacant for the purposes of insertion,
 * but as filled for the purpose of lookups.
 *
 * Note also that this function will only delete the specific digram that is
 * passed as the argument, not some other matching digram that happens
 * to be in the table.  The reason for this is that if we were to delete
 * some other digram, then somebody would have to be responsible for
 * recycling the symbols contained in it, and we do not have the information
 * at this point that would allow us to be able to decide whether it makes
 * sense to do it here.
 */
int digram_delete(SYMBOL *digram) {
    // To be implemented.

   // debug("getting digram");

    debug("delete digram %d %d ", digram->value, digram->next->value);

    int value = DIGRAM_HASH(digram->value, digram->next->value);
    
    for(int i=value;i<MAX_DIGRAMS;i++)
    {
     //   SYMBOL *daig = *(digram_table + i);

        if(*(digram_table + i) == TOMBSTONE)
            continue;


        if(*(digram_table + i) == NULL)
            {
                debug("entry not found");
                return -1;}
        if(*(digram_table + i) == digram)
        {

                    debug("delete digram at %d ", i);
                    *(digram_table + i) = TOMBSTONE;
                    return 0;
        }
    }

    for(int i=0;i<value;i++)
    {
    //    SYMBOL *daig = *(digram_table + 1);

        if(*(digram_table + i) == TOMBSTONE)
            continue;

        if(*(digram_table + i) == NULL)
            {
                debug("entry not found");
                return -1;}
        if(*(digram_table + i) == digram)
        {
            debug("delete digram at %d ", i);
            *(digram_table + i) = TOMBSTONE;
            return 0;  
        }
    }
    return -1;
}

/**
 * Attempt to insert a digram into the hash table.
 *
 * @param digram  The digram to be inserted.
 * @return  0 in case the digram did not previously exist in the table and
 * insertion was successful, 1 if a matching digram already existed in the
 * table and no change was made, and -1 in case of an error, such as the hash
 * table being full or the given digram not being well-formed.
 */
int digram_put(SYMBOL *digram) {
     // To be implemented.

    // Check if the diagram is well formed

    int v1 = digram->value;
    int v2 = digram->next->value;

    debug("add digram %d %d ", v1, v2);

    int value = DIGRAM_HASH(v1, v2);

    for(int i=value;i<MAX_DIGRAMS;i++)
    {
      //  SYMBOL *diag = *(digram_table + i);

     //   debug("sending to digram get %i", i);
        if((*(digram_table + 1)) == digram)
            {
                debug("diag found at index %d ", i);
                return 1;
            }

        if(*(digram_table + i) == NULL || *(digram_table + i) == TOMBSTONE)
            {
                debug("doing diag put at index %d", i);
                *(digram_table + i) = digram;
                // debug("check diag index value1 %d %d", diag->value, diag->next->value);
                // debug("check diag index value2 %d %d", digram->value, digram->next->value);
                // debug("check diag index value3 %d %d ", (*(digram_table + i))->value, (*(digram_table + i))->next->value);
                return 0;
            }
    }

    // Did not find it till the end
    for(int i=0;i<value;i++)
    {
      //  SYMBOL *diag = *(digram_table + i);
        if(*(digram_table + i) == digram)
            return 1;
 
        if(*(digram_table + i) == NULL || *(digram_table + i) == TOMBSTONE)
        {
            debug("doing diag put");
            *(digram_table + i) = digram;
            return 0;
        }
    }

    return -1;
}