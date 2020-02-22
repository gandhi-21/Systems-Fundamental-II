/**
This is a filee to define custom  helper functions
*/
#ifndef custom
#define custom


int bytes_compresses;
int block_count;
int bytes_decompresses;
unsigned char curr;
unsigned char prev;
int end_rule;
SYMBOL *curr_rule;



/**
    Functions for compression
*/
void process_compression(FILE *out);
int get_encoded(int num, FILE *out);
int get_bytes_encode(int num);

/**
    Functions for decompression
*/
void process_decompression_new(SYMBOL *rule, FILE *out);
int get_decoded(char ch, FILE *in);
int get_bytes(unsigned int c);
void add_symbol_rule(SYMBOL *rule, SYMBOL *symbol);
int check_special(unsigned char curr, FILE *out);


/**
    Function for valid args
*/
int checkStrings(char *string1, char *string2);
int convert_string_int(char *string1);

#endif