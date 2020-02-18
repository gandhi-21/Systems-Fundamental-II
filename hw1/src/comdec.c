#include "const.h"
#include "sequitur.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/*
 * You may modify this file and/or move the functions contained here
 * to other source files (except for main.c) as you wish.
 *
 * IMPORTANT: You MAY NOT use any array brackets (i.e. [ and ]) and
 * you MAY NOT declare any arrays or allocate any storage with malloc().
 * The purpose of this restriction is to force you to use pointers.
 * Variables to hold the pathname of the current file or directory
 * as well as other data have been pre-declared for you in const.h.
 * You must use those variables, rather than declaring your own.
 * IF YOU VIOLATE THIS RESTRICTION, YOU WILL GET A ZERO!
 *
 * IMPORTANT: You MAY NOT use floating point arithmetic or declare
 * any "float" or "double" variables.  IF YOU VIOLATE THIS RESTRICTION,
 * YOU WILL GET A ZERO!
 */

/**
 * Main compression function.
 * Reads a sequence of bytes from a specified input stream, segments the
 * input data into blocks of a specified maximum number of bytes,
 * uses the Sequitur algorithm to compress each block of input to a list
 * of rules, and outputs the resulting compressed data transmission to a
 * specified output stream in the format detailed in the header files and
 * assignment handout.  The output stream is flushed once the transmission
 * is complete.
 *
 * The maximum number of bytes of uncompressed data represented by each
 * block of the compressed transmission is limited to the specified value
 * "bsize".  Each compressed block except for the last one represents exactly
 * "bsize" bytes of uncompressed data and the last compressed block represents
 * at most "bsize" bytes.
 *
 * @param in  The stream from which input is to be read.
 * @param out  The stream to which the block is to be written.
 * @param bsize  The maximum number of bytes read per block.
 * @return  The number of bytes written, in case of success,
 * otherwise EOF.
 */
int compress(FILE *in, FILE *out, int bsize) {
    // To be implemented.
    return EOF;
}


/**
 * Add to the .h file custom
@brief Compares two strings using pointers
@param string1 The string 1 to be compared
@param string2 The string 2 to be compared
@return 0 if strings are equal and -1 if not equal
*/
int checkStrings(char* string1, char* string2)
{
    while(*string1 == *string2) {
        if(*string1 == '\0' || *string2 == '\0')
            break;
        string1++;
        string2++;
    }

    if( *string1 == '\0' && *string2 == '\0' )
      return 0;
   else
      return -1;
}


/**
 Get the number of bytes to be read to decode the utf8

*/
int get_bytes(unsigned int c)
{
    if ((c & 0x80) == 0)
    {
        return 0;
    } else if ((c & 0xE0) == 0xC0)
    {
        return 1;
    } else if ((c & 0xF0) == 0xE0)
    {
        return 2;
    } else if ((c & 0xF8) == 0xF0)
    {
        return 3;
    }

    return 0;
}

/**
Get the decoded value of the utf8 char in int
*/

int get_decoded(char c, FILE *in)
{
        unsigned int ch = (unsigned int) c & 0xFF;

        printf("%x ", ch);

        int bytes_required = get_bytes(ch);

        if(bytes_required == 0){
            ch = ch;
        } else if(bytes_required == 1){
            ch = ch & 0x1F;
        } else if (bytes_required == 2) {
            ch = ch & 0xF;
        } else {
            ch = ch & 0x7;
        }

        while(bytes_required > 0)
        {
            char c_next = fgetc(in);
            ch = (ch << 6);
            ch |= (c_next & 0x3F);
            bytes_required -= 1;
        }

        return ch;
}

/**

    1.  84 82
    2.  84 83
    3.  81 83

*/

int rule_exists = 0;

int check_special(FILE *in,unsigned char prevChar)
{
    if (prevChar == 0x84)
    {
        unsigned char currChar = fgetc(in);
        if(currChar == 0x82)
        {
            debug("end of transmission");
            return -1;
        } else if(currChar == 0x83)
        {
            debug("start of new block");
            init_rules();
            init_symbols();
            debug("initialized rules and symbols");
            return 0;
        } else{
            return currChar;
        }
    } else if(prevChar == 0x81)
    {
        unsigned char currChar = fgetc(in);
        if(currChar == 0x83)
        {
            debug("start of transmission");
            debug("start of new block");
            init_rules();
            init_symbols();
            debug("initialized rules and symbols");
            return 0;
        } else {
            return -1;
        }
    }
    return -1;
}

/**
Function to print a linked list
*/
void print_linked()
{
//     SYMBOL *temp = main_rule;
//  // debug("Entered the end of a rule");
//     while(temp != NULL)
//     {
//         debug("%d ", temp->value);
//         temp = temp->next;
//     }
//     return;
}

void print_rule(SYMBOL *curr_rule)
{
    SYMBOL *temp = curr_rule;
    debug("current rule");
    debug("current rule start value: %d ", curr_rule->value);
    while(temp->next != curr_rule)
    {debug("value %d ", temp->value);
    temp = temp->next;}
}


/**
 function to add a symbol to a rule body
*/
void add_symbol_rule(SYMBOL *rule, SYMBOL *symbol)
{
    //debug("adding the symbol to the rule body"); 
    //debug("value at current rule %d ",  rule->value);
    //debug("value at current last %d ", rule->prev->value);
    SYMBOL *last = rule->prev;
    rule->prev = symbol;
    symbol->next = rule;
    symbol->prev = last;
    last->next = symbol;
    // debug("value of rule head %d ", temp->value);
    //debug("added symbol to current rule");
    return;
}


/**
 * Main decompression function.
 * Reads a compressed data transmission from an input stream, expands it,
 * and and writes the resulting decompressed data to an output stream.
 * The output stream is flushed once writing is complete.
 *
 * @param in  The stream from which the compressed block is to be read.
 * @param out  The stream to which the uncompressed data is to be written.
 * @return  The number of bytes written, in case of success, otherwise EOF.
 */
int decompress(FILE *in, FILE *out) {
    
    int in_decompress = 0;
    int in_block = 0;
    int no_block = 0; 
    SYMBOL *curr_rule = NULL;
    int no_rules = 0;
    while(!feof(in))
    {
        unsigned char ch = fgetc(in);
        in_block = check_special(in, ch);
        while(in_block == 0)
        {
            unsigned char nextChar = fgetc(in);
            if(nextChar == 0x84)
                {
                    in_block = check_special(in, nextChar);
                }
            if(nextChar == 0x85)
            {
                // Only create a new rule over here
              //   debug("end of a rule");
                //debug("curr rule value %d ", curr_rule->value);
                print_rule(curr_rule);
                add_rule(curr_rule);
                debug("added curr rule to main rule");
                curr_rule = NULL;
               // print_linked();
               nextChar = fgetc(in);
            }
            int decoded_char = get_decoded(nextChar, in);
            //  Make the linked list over here
            //  debug("started new rule returning a new symbol");
            //  debug("decoded char is %d ", decoded_char);
            if(curr_rule == NULL)
            {
               // debug("making a new rule here");
            //    debug("CURR_RULE FUCK OFF");
                curr_rule = new_rule(decoded_char);
                // debug("curr rule -> prev ->value %d ", curr_rule->prev->value);
                // debug("curr rule -> next ->value %d ", curr_rule->next->value);
                // next and prev of the rule point to itself
               // debug("new rule value created %d ", curr_rule->value);
              // debug("no of rules %d ", no_rules);
            } else {
                 //    debug("rule exists %d ", rule_exists);
                  // debug("making a new symbol here");
                //debug("newly created rule value I %d ", curr_rule->value);
                    SYMBOL *new_sym = new_symbol(decoded_char, NULL);
                  // debug("got a new symbol");
                //    debug("newly got symbol value %d ", new_sym->value);
                 //   debug("newly created rule value II %d ", curr_rule->value);
                    add_symbol_rule(curr_rule, new_sym);
                }
            //  debug("ended new rule returning a new symbol");
            //  debug("started add new rule");
            //  debug("new_sym -> prevr -> value: %d", new_sym->prevr->value);
            //  add the symbol to the rule body
            //  param 1 is new rule, param2 is new symbol
            //  Create a symbol and add it to the body of the current rule
            // add_rule(new_sym);
            // debug("ended add new rule ");
        }
        
    }
    // debug("no of block: %d", no_block);
    return EOF;
}


/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the selected program options will be set in the
 * global variable "global_options", where they will be accessible
 * elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * Refer to the homework document for the effects of this function on
 * global variables.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected options.
 */
int validargs(int argc, char **argv)
{
    // To be implemented.
    char* helpArg = "-h";
    char* compressArg = "-c";
    char* decompressArg = "-d";
    char* blocksizeArg = "-b";

    // Check for the number of arguments
    if(argc <= 1) {
        return -1;
    }
    // Check for the -h flag over here
    else if(checkStrings(*(argv+1), helpArg) == 0) {
        
        // Make the least significant bit 1

        global_options |= (1 << 0); 

        return 0;
    }
    // Check for the -c/-d flag over here
    else if(checkStrings(*(argv+1), compressArg) == 0 || checkStrings(*(argv+1), decompressArg) == 0) {
        // Check for the -c flag here
        //printf("-c/-d flag usef\n");
        if(checkStrings(*(argv+1), compressArg) == 0) {
          //  printf("-c used here");
        // Check for the optional -b here
            if(argc >=2) {
                if(checkStrings(*(argv+2), blocksizeArg)) {
                // Check for the block size here
                 // printf("optional -b used here");
                    // check for block size argument here
                    if(atoi(*(argv+3)) < 1 || atoi(*(argv+3)) > 1024) {
                        // return error
                    } else {
                        // Work with -c and block size

                        global_options |= (1 << 1);

                        return 0;

                    }
                }
            } else {
                // check if there are no more argmemts
                global_options |= (1 << 1);

                return 0;
            }
        }
        // Make sure that -b is not used here
        else {
            // printf("-d used here");
                // work with d here
                global_options = ((1 << 2) | global_options);
                return 0;
        }
    }
    // Return failure here
    else
        return -1;
        
    return -1;
}

