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

SYMBOL *recycled_symbol = NULL;


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
    recycled_symbol = NULL;
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

    debug("Creating symbol [%d]", value);

    SYMBOL *new_sym = NULL;

    if(recycled_symbol != NULL)
    {
        //  use the recycled symbol here
        new_sym = recycled_symbol;
        if(recycled_symbol->next != NULL)
            recycled_symbol = recycled_symbol->next;
        else
            recycled_symbol = NULL;
    } else if(num_symbols < MAX_SYMBOLS){
        // use the symbol storage here

        debug("using symbol storage");
        new_sym = (symbol_storage + num_symbols);
        num_symbols++;
        debug("used symbol storage");
    } else {
        fprintf(stderr, "Symbol storage exhausted");
        abort();
    }

    new_sym->value = value;
    new_sym->refcnt = 0;
    new_sym->next = NULL;
    new_sym->prev = NULL;
    new_sym->nextr = NULL;
    new_sym->prevr = NULL;
    new_sym->rule = NULL;

    debug("set the initial values");

    if(value >= FIRST_NONTERMINAL)
    {
        new_sym->rule = rule;
        if(rule != NULL)
            rule->refcnt += 1;
    }
    debug("returning from new symbol");
    return new_sym;
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
  
    if(recycled_symbol == NULL)
   { recycled_symbol = s;
    recycled_symbol->next = NULL;
   }
    else 
    {
        s->next = recycled_symbol;
        recycled_symbol = s;
    }
    return;
}
