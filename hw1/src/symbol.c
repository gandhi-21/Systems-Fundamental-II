#include "const.h"
#include "sequitur.h"

/*
 * Symbol management.
 *
 * The functions here manage a statically allocated array of SYMBOL structures,
 * together with a stack of "recycled" symbols.
 */

/*
 * Initialization of this global variable that could not be performed in the header file.
 */
int next_nonterminal_value = FIRST_NONTERMINAL;

/**
 * Declare and initialize the struct stack for the recycled symbols
 * get_symbol returns the current head symbol
 * recycle_symbol adds a symbol to the head of the recycled symbols list
 * add these to custom.h
 */
struct recycled_symbols {
    int number_symbols;
    struct symbol *head;
};

struct recycled_symbols RECYCLED_SYMBOLS = {0, NULL};

/**
 * Initialize the symbols module.
 * Frees all symbols, setting num_symbols to 0, and resets next_nonterminal_value
 * to FIRST_NONTERMINAL;
 */
void init_symbols(void) {
    // To be implemented.
 //   debug("started initial symbols");
    num_symbols = 0;
    next_nonterminal_value = FIRST_NONTERMINAL;
 //   debug("initialized symbols");
    return;
}

/**
 * Get a new symbol.
 *
 * @param value  The value to be used for the symbol.  Whether the symbol is a terminal
 * symbol or a non-terminal symbol is determined by its value: terminal symbols have
 * "small" values (i.e. < FIRST_NONTERMINAL), and nonterminal symbols have "large" values
 * (i.e. >= FIRST_NONTERMINAL).
 * @param rule  For a terminal symbol, this parameter should be NULL.  For a nonterminal
 * symbol, this parameter can be used to specify a rule having that nonterminal at its head.
 * In that case, the reference count of the rule is increased by one and a pointer to the rule
 * is stored in the symbol.  This parameter can also be NULL for a nonterminal symbol if the
 * associated rule is not currently known and will be assigned later.
 * @return  A pointer to the new symbol, whose value and rule fields have been initialized
 * according to the parameters passed, and with other fields zeroed.  If the symbol storage
 * is exhausted and a new symbol cannot be created, then a message is printed to stderr and
 * abort() is called.
 *
 * When this function is called, if there are any recycled symbols, then one of those is removed
 * from the recycling list and used to satisfy the request.
 * Otherwise, if there currently are no recycled symbols, then a new symbol is allocated from
 * the main symbol_storage array and the num_symbols variable is incremented to record the
 * allocation.
 */
SYMBOL *new_symbol(int value, SYMBOL *rule) {
    // To be implemented.

  //  debug("started new symbol ");

    if(RECYCLED_SYMBOLS.number_symbols != 0) {
        // recycle a symbol and return it
    //    debug("taking a recycled symbol");
        SYMBOL *newSymbol = RECYCLED_SYMBOLS.head;

        RECYCLED_SYMBOLS.head = RECYCLED_SYMBOLS.head->next;

        newSymbol->value = value;
        if(value < FIRST_NONTERMINAL){
            newSymbol->rule = NULL;
            newSymbol->refcnt = 0;
        }
        else {
            newSymbol->rule = rule;
            rule->refcnt += 1;
            newSymbol->refcnt = 0;
        }
        newSymbol->next = NULL;
        newSymbol->prev = NULL;
        newSymbol->nextr = NULL;
        newSymbol->prevr = NULL;
    
        return newSymbol;

    } else if(num_symbols < MAX_SYMBOLS) {
        // use main storage to create a new symbol and return it
      //  debug("makeing a new symbol from the symbols storage");
        SYMBOL *ptr = symbol_storage;
       // debug("no of symbols already in the symbols %d ", num_symbols);
        for(int i=0;i<num_symbols; i++) {
            ptr = ptr+1;
        }
        num_symbols++;
       // debug("went through the num of symbols using the loop");
       // debug("created a new symbol pointer");
        ptr->value = value;
    //    debug("added the value to the newly created symbol pointer");
        if(value < FIRST_NONTERMINAL){
      //      debug("adding a non terminal value");
            ptr->rule = NULL;
            ptr->refcnt = 0;
        //    debug("added a non terminal value");
        }
        else {
          //  debug("adding a terminal value");
            ptr->rule = rule;
           // debug("added the value of rule to rule");
            if(rule != NULL)
            rule->refcnt += 1;
           // debug("added the ref count");
            ptr->refcnt = 0;
        //    debug("added a terminal value");
        }
        ptr->next = NULL;
        ptr->prev = NULL;
        ptr->nextr = NULL;
        ptr->prevr = NULL;
        // debug("made a new symbol from the symbol storage and now returning it");
  //      debug("value of new symbol %d", newSymbol->value);
        return ptr;
    } else {
        // stderr and abort
        fprintf(stderr, "Symbols Storage exhausted!\n");
        abort();
    }
    return NULL;
}

/**
 * Recycle a symbol that is no longer being used.
 *
 * @param s  The symbol to be recycled.  The caller must not use this symbol any more
 * once it has been recycled.
 *
 * Symbols being recycled are added to the recycled_symbols list, where they will
 * be made available for re-allocation by a subsequent call to new_symbol.
 * The recycled_symbols list is managed as a LIFO list (i.e. a stack), using the
 * next field of the SYMBOL structure to chain together the entries.
 */
void recycle_symbol(SYMBOL *s) {
    // To be implemented.

    // remove the symbol from the list first
    s->prev->next = s->next;

    // add the symbol to the list of recycled symbols
    RECYCLED_SYMBOLS.number_symbols += 1;
    if(RECYCLED_SYMBOLS.head == NULL) {
        RECYCLED_SYMBOLS.head = s;
    } else {
        s->next = RECYCLED_SYMBOLS.head;
        RECYCLED_SYMBOLS.head = s;
    }
    return;
}
