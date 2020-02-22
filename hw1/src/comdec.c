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

int bytes_compresses = 0;

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
            bytes_compresses++;
            return;}

    first_byte |= num >> (bytes - 1) * 6;
    fputc(first_byte, out);
    bytes_compresses++;

    for(int i=bytes - 2; i>=0; i--)
    {
        int new_num = num >> (6 * i);
        unsigned char value = 0x80 | (new_num & 0x3F);
      //  debug("%x ",value);
        fputc(value, out);
        bytes_compresses++;
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
    bytes_compresses++;
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
                fputc(0x85, out);
                bytes_compresses++;}

    }while(temp != main_rule);

    block_count++;
    fputc(0x84, out);
    bytes_compresses++;
    debug("bytes read %d ", count);

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

    debug("bsize is %d ", bsize);

    bsize = bsize * 1024;

    debug("start compress");
    int bytes_read = 0;
    init_rules();
    init_symbols();
    init_digram_hash();
    debug("init rules and symbols");
    add_rule(new_rule(next_nonterminal_value++));
    fputc(0x81, out);
    bytes_compresses++;
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
    bytes_compresses++;
    debug("ended compress");
    debug("blocks encoded %d", block_count);
    return bytes_compresses;
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

int bytes_decompresses = 0;

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
            bytes_decompresses++;
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


unsigned char curr = 0;
unsigned char prev = 0;
int end_rule = 0;
SYMBOL *curr_rule = NULL;

int check_special(unsigned char curr, FILE *out)
{

    if(curr == 0x81 || curr == 0x84)
    {
        return -1;
    }

    if(curr == 0x83 && prev == 0x81)
    {
        // start of a transmission
        debug("start of transmission");
        init_rules();
        init_symbols();
        curr_rule = NULL;
        return -1;
    } else if(curr == 0x83 && prev == 0x84)
    {
        // start of a block
        debug("end/start of a block");
        int value = curr_rule->value;
        *(rule_map + value) = curr_rule;
        process_decompression_new(main_rule, out);
        fflush(out);
        init_rules();
        init_symbols();
        curr_rule = NULL;
        return -1;
    } else if(curr == 0x82 && prev == 0x84)
    {
        // end of transmission
        debug("end of transmission");
        int value = curr_rule->value;
        *(rule_map + value) = curr_rule;
        process_decompression_new(main_rule, out);
        fflush(out);
        return bytes_decompresses;
    } else if(curr == 0x85)
    {
        end_rule = 1;
        return 0;
    }   else {
        return 0;
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
  
    while(!feof(in))
    {
        curr = fgetc(in);   
        if(check_special(curr, out) == 0)
        {
            
            if(end_rule == 1)
            {
                // end of a rule
                int value = curr_rule->value;
                *(rule_map + value) = curr_rule;
                add_rule(curr_rule);
                curr_rule = NULL;
                end_rule = 0;
            } else {
                // in block
                int decoded_value = get_decoded(curr, in);
                if(curr_rule == NULL)
                    curr_rule = new_rule(decoded_value);
                else {
                    SYMBOL *new_sym = new_symbol(decoded_value, NULL);
                    add_symbol_rule(curr_rule, new_sym);
                }
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
 Convert string to int
*/
int convert_string_int(char *string1)
{
    
    debug("converting int ");
    // if(*(string1) == '-')
    // return -1;
    int n = 0;

    while(*string1 != '\0')
    {
        char c = *(string1) - 48;
        debug("value %d ", c);
        if(c > 9 || c < 0)
        return -1;
        n = (n*10 + c);
        string1 = string1 + 1;
    }

    debug("bsize is %d ", n);

    return n;

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

    debug("in valid args");

    if(argc < 2)
    {
        return -1;
    }

    // check for -h
    if(checkStrings(*(argv + 1), helpArg) == 0)
    {
        debug("sending back to help");
        // set global options for help and return
        global_options = 0x1;
        return 0;
    }


    // check for -c
    if(argc >= 2)
    {
        if(checkStrings(*(argv + 1), compressArg) == 0)
        {
                debug("in valid args 2 c");
            if(argc > 2)
            {
            // check for -b
                if(checkStrings(*(argv + 2), blocksizeArg) == 0)
                {
                        debug("in valid args 3 bsize");
                    // check the blocksize
                    int block = convert_string_int(*(argv + 3));
                    debug("block size is %d ", block);
                    if(block == -1 || block > 1024)
                    {
                        return -1;
                    } else {
                        // set the block size here
                        debug("got a block sizze here");
                        global_options = block;
                        global_options = (global_options << 16) + 2; 
                        return 0;
                    }

                } else {
                    return -1;
                }
            } else {
                global_options = 1024;
                global_options = (global_options << 16) + 2;
                return 0;
            }
        } else if(checkStrings(*(argv + 1), decompressArg) == 0)
        {

            global_options = ((1 << 2) | global_options);
            return 0;

        } else {
            return -1;
        }
        
    }
    return -1;
}

