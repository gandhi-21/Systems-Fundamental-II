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
Function to find number of bytes needed to encode
*/
int get_bytes_encode(int num)
{

  //  debug("%d ", num);
    if(num > 0xFFFF)
    return -1;
    if(num > 0xFFFF)
    return 4;
    else if(num > 0x07FF)
    return 3;
    else if(num > 0x007F)
    return 2;
    else return 1;
}

/**
 Function to encode a unsigned char to utf8 encoded char
*/
void get_encoded(int num, FILE *out)
{
    int bytes = get_bytes_encode(num);

   // debug("%d ", bytes);

    unsigned char first_byte = 0;

    if(bytes == -1)
    return;

    if(bytes == 4)
        first_byte = 0xF0;
    else if(bytes == 3)
        first_byte = 0xE0;
    else if(bytes == 2)
        first_byte = 0xC0;
    else
        {
        //    debug("%x ",num);
            fputc(num, out);
            return;}

    first_byte |= num >> (bytes - 1) * 6;
    fputc(first_byte, out);

    for(int i=bytes - 2; i>=0; i--)
    {
        int new_num = num >> (6 * i);
        unsigned char value = 0x80 | (new_num & 0x3F);
      //  debug("%x ",value);
        fputc(value, out);
    }
    return ;
}

int block_count = 0;

void process_compression(FILE *out, int last_block)
{

    debug("in process compression");

    int count = 0;

    SYMBOL *temp = main_rule;
    int stopRule = 1;
    fputc(0x83, out);    

    do{

        SYMBOL *rule = temp;

        do {
         //   debug("encoding value %d ", rule->value);
            get_encoded(rule->value, out);
            rule = rule->next;
        }while(rule != temp);

        
        temp = temp->nextr;

        if(last_block == 0 && temp == main_rule)
            break;
        else if (temp == main_rule)
        {
            break;
        } else
           {
                debug("putting a rule delimiter");
                fputc(0x85, out);}

    }while(temp != main_rule);

    block_count++;
    fputc(0x84, out);
    debug("bytes read %d ", count);

}

// void print_rule_main(FILE *out)
// {
//     SYMBOL *temp = main_rule;
//     int stopRule = 1;
//     while(stopRule == 1){
        
//       //  debug("Main Rule: ");
//      //   debug("[%d]", temp->value);
//         SYMBOL *temp2 = temp;
//         int stopRuleBody = 1;
//         while(stopRuleBody){
//         //    debug("->%d", temp2->value);
//            // get_encoded(temp2->value, out);
//             temp2 = temp2 -> next;
//             if(temp2 == temp){
//                 break;
//             }
//         }   
//         temp = temp->nextr;
//         if(temp == main_rule){
//             break;
//         }
//     }
//  }


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

    bsize = bsize * 1024;

    debug("start compress");
    int bytes_read = 0;
    init_rules();
    init_symbols();
    init_digram_hash();
    debug("init rules and symbols");
    add_rule(new_rule(next_nonterminal_value++));
    fputc(0x81, out);

    while(!feof(in))
    {
        int byte = fgetc(in);
        bytes_read++;

        if(byte == EOF)
        break;
        SYMBOL *byte_symbol = new_symbol(byte, NULL);

        debug("at insert after");
        insert_after(main_rule->prev, byte_symbol);
        debug("at check diagram");
        check_digram(main_rule->prev->prev);

        if (bytes_read == bsize)
        {
            bytes_read = 0;
            debug("bytes_read is bsize");
            process_compression(out, 1);
            init_rules();
            init_symbols();
            init_digram_hash();
            add_rule(new_rule(next_nonterminal_value++));
            debug("end of block");
        }
    }

    process_compression(out, 0);
    fputc(0x82, out);
    debug("ended compress");
    debug("blocks encoded %d", block_count);
    return EOF;
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


void print_rule_main()
{
    SYMBOL *temp = main_rule;

    do{

        SYMBOL *rule = temp;


        debug("new rule starts here");
        debug("rule head %d ", rule->value);

        do {

            debug("symbol in rule %d ", rule->value);
            rule = rule->next;

        }while(rule != temp);

        temp = temp->nextr;

    }while(temp != main_rule);

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
            debug("terminal value %d", temp_next->value);
            fputc(temp_next->value, out);
        } else {
            debug("non terminal value %d", temp_next->value);
            int value = temp_next->value;
            process_decompression_new((*(rule_map + value)), out);
        } 
        temp_next = temp_next->next;
    }

}


/**
 function to add a symbol to a rule body
*/
void add_symbol_rule(SYMBOL *rule, SYMBOL *symbol)
{
    SYMBOL *last = rule->prev;
    rule->prev = symbol;
    symbol->next = rule;
    symbol->prev = last;
    last->next = symbol;
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
  

    unsigned char curr, prev;
    SYMBOL *curr_rule = NULL;
    prev = fgetc(in);

     while(!feof(in))
    {
        curr = fgetc(in);

        if(prev == 0x81 && curr == 0x83)
        {
            // start of transmission
            debug("star of transmission");
            init_rules();
            init_symbols();
            curr_rule = NULL;
        } else if(prev == 0x85 && curr == 0x83)
        {
            // start and end of the block
            debug("start/end of a block");
            if(curr_rule != NULL)
            {
                int value = curr_rule->value;
                *(rule_map + value) = curr_rule;
                add_rule(curr_rule);
                curr_rule = NULL;
            }

            process_decompression_new(main_rule, out);
            fflush(out);
            init_rules();
            init_symbols();
            curr_rule = NULL;
        } else if(prev == 0x84 && curr == 0x82)
        {
            // end of transmission
            debug("end of transmission");
            int value = curr_rule->value;
            *(rule_map + value) = curr_rule;
            add_rule(curr_rule);
            curr_rule = NULL;
         //   print_rule_main();
            process_decompression_new(main_rule, out);
            fflush(out);
            return EOF;
        } else {
            // normal in block
            debug("in normal block");
            int curr_decoded = get_decoded(curr, in);
            if(curr_rule == NULL)
            {
                curr_rule = new_rule(curr_decoded);
            }  else {
                SYMBOL *new_sym = new_symbol(curr_decoded, NULL);
                add_symbol_rule(curr_rule, new_sym);
            }
        }

        prev = curr;

    }
   
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
            debug("-c used here");
                global_options |= (1 << 1);
                return 0;
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

