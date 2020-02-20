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
 Function to encode a unsigned char to utf8 encoded char
*/
unsigned char get_encoded(unsigned char ch)
{
    return 0;
}


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

    // int bytes_read = 0;
    // init_rules();
    // init_symbols();

    // // Read in data from the input, segment into bytes 
    // // Use this algorithm to compress the input
    // // while(input data remains)
    // // {
    // //  read a byte of input
    // //  create terminal symbol from input byte
    // //  use insert_after() to append symbol to main rule
    // //  call check_diagram() on second-to-last symbol in main rule  
    // // }
    // // after every block size is read, put everything to output
    // // 
    int bytes_read = 0;

    // while(!feof(in))
    //{
    //     int byte = fgetc(in);
    //     bytes_read++;

    //     SYMBOL *byte_symbol = new_symbol((unsigned char)byte, NULL);

    //     // insert_after(main_rule, byte_symbol);

    //     // check_digram(main_rule->prev->prev);

    //     if (bytes_read == bsize)
    //     {
    //         bytes_read = 0;

    //         // convert int to utf8
    //         // output the bytes
    //         // initialize the rules and symbols again

    //     }

    // }


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

       // printf("%x ", ch);

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


SYMBOL *curr_rule = NULL;

void print_rule(SYMBOL *curr_rule)
{
    SYMBOL *temp = curr_rule;
    // debug("current rule");
    // debug("current rule start value: %d ", curr_rule->value);
    while(temp->next != curr_rule)
    {debug("value %d ", temp->value);
    temp = temp->next;}
    debug("value extra %d ", temp->value);
    debug("\n");
}


/**
Function to decompress a rule
*/
void decompress_rule(SYMBOL *rule, FILE *out)
{
 //   debug("decompressing rule");

    SYMBOL *symbol_mover = rule;
    
    do{
        if(symbol_mover->value < FIRST_NONTERMINAL)
        {
           // debug("terminal symbol %c", symbol_mover->value);
            fputc(symbol_mover->value, out);
            symbol_mover = symbol_mover->next;
        }   else {
         //   debug("non terminal rule");
         //   debug("value of non-terminal %d", symbol_mover->value);
            int value = symbol_mover->value;
            decompress_rule((*(rule_map + value))->next  , out);
            symbol_mover = symbol_mover->next;
        }
    }while(symbol_mover->next != rule);
}


/**
Function to process the decompression
Make another function to decompress a rule
call that function as you traverse the list of rules in the main rule
if you get a non terminal symbol call the function to decompress the rule
*/

void process_decompression_new(SYMBOL *rule, FILE *out)
{
    SYMBOL *temp = rule;
    SYMBOL *temp_next = rule->next;

    while(temp_next != temp)
    {
        if(temp_next->value < FIRST_NONTERMINAL)
        {
            debug("terminal value ");
            fputc(temp_next->value, out);
        } else {
            debug("non terminal value");
            int value = temp_next->value;
            process_decompression_new((*(rule_map + value)), out);
        } 
        temp_next = temp_next->next;
    }

}

int check_special(FILE *in,unsigned char prevChar, FILE *out)
{
    if (prevChar == 0x84)
    {
        unsigned char currChar = fgetc(in);
        if(currChar == 0x82)
        {
            // debug("end of block");
       //     process_decompression(main_rule, out);
            return -1;
        } else if(currChar == 0x83)
        {
            //debug("end of a block");
            //process_decompression(main_rule, out);
            debug("start of new block");
            init_rules();
            init_symbols();
            curr_rule = NULL;
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
            // init_rules();
            // init_symbols();
            debug("initialized rules and symbols");
            return 0;
        } else {
            return -1;
        }
    }
    return -1;
}



void print_rule_main()
{
    SYMBOL *temp = main_rule;
    debug("main rule");
    // debug("main rule last value: %d ", temp->value);
    while(temp->nextr != main_rule)
    {
    debug("head of the rule %d ", temp->value);
    temp = temp->nextr;}
    //print_rule(temp);
    debug("end of main rule");
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
Function to print the values in the rule map
*/
void print_rule_map()
{
    debug("printing rule map");
    for(int i = 0; i < MAX_SYMBOLS; i++)
    {
        if(*(rule_map + i) != NULL){
            debug("I: %d, Value: %d", i, (*(rule_map + i))->value);
          //  print_rule(*(rule_map + i));
        } 
    }
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
  
    // maintain previous and current
    // if 81 and 83
    // init rules and symbols
    // start reading the blocks
    // if previous 84 and current 83
    // decompress
    // init symbols and rules
    // start reading
    // if prev is 84 and current 82
    // decompress and return

    unsigned char prev, curr;
    prev = fgetc(in);
    SYMBOL *curr_rule = NULL;

    while(!feof(in))
    {
        curr = fgetc(in);
        if(curr == 0x83 && prev == 0x81)
        {
          //  debug("start of transmission");
            init_rules();
            init_symbols();
        } else if(curr == 0x83 && prev == 0x84)
        {
          //  debug("start of a block");
           // print_rule_map();
            if(curr_rule != NULL)
            {
                int value = curr_rule->value;
          //      debug("head of last rule block %d ", value);
                *(rule_map + value) = curr_rule;
                add_rule(curr_rule);
                curr_rule = NULL;
           //     print_rule_map();
            }
            debug("called process decompress");
            process_decompression_new(main_rule, out);
            fflush(out);
            init_rules();
            init_symbols();
            curr_rule = NULL;
        } else if(curr == 0x82 && prev == 0x84)
        {
         //   debug("end of transmission");
            //debug("printing rule map");
            int value = curr_rule->value;
          //  debug("value of last rule transmission %d ", value);
            *(rule_map + value) = curr_rule;
            add_rule(curr_rule);
            curr_rule = NULL;
        // //    print_rule_map();
             debug("called process decompression");
            process_decompression_new(main_rule, out);
            fflush(out);
            return EOF;
        } else if(curr_rule != NULL && curr == 0x85)
        {
          //  debug("end of a rule");
            int value = curr_rule->value;
          //  debug("head of current rule %d ", value);
            *(rule_map + value) = curr_rule;
            add_rule(curr_rule);
            curr_rule = NULL;
        }
        else {
         //   debug("in block");
            int curr_decoded = get_decoded(curr, in);
           // debug("Decoded char %d ", curr_decoded);
            if(curr_rule == NULL)
            {
               // debug("rule with value %d ", curr_decoded);
                curr_rule = new_rule(curr_decoded);
            } else {
            //    debug("symbol with value %d ", curr_decoded);
                SYMBOL * new_sym = new_symbol(curr_decoded, NULL);
                add_symbol_rule(curr_rule, new_sym);
            }

        }
        prev = curr;
    }
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

