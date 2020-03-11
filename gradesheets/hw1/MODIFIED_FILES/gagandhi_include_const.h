--- hw1/dev_repo/solution/include/const.h

+++ hw1/repos/gagandhi/hw1/include/const.h

@@ -17,10 +17,10 @@

 

 #define USAGE(program_name, retcode) do { \

 fprintf(stderr, "USAGE: %s %s\n", program_name, \

-"[-h] -c|-d [-b]\n" \

+"[-h] -s|-d [-c]\n" \

 "   -h       Help: displays this help menu.\n" \

 "   -c       Compress: read bytes from standard input, output compressed data to standard output.\n" \

-"   -d       Decompress: read compressed data from standard input, output raw data to standard output.\n" \

+"   -d       Deserialize: read compressed data from standard input, uncompress to standard output.\n" \

 "            Optional additional parameter for -c (not permitted with -d):\n" \

 "               -b           BLOCKSIZE is the blocksize (in Kbytes, range [1, 1024])\n" \

 "                            to be used in compression.\n"); \

@@ -56,7 +56,7 @@

 /*

  * Array, used during decompression, that maps symbol values to nonterminal symbols.

  */

-SYMBOL *rule_map[SYMBOL_VALUE_MAX];

+SYMBOL *rule_map[MAX_SYMBOLS];

 

 /*

  * Below this line are prototypes for functions that MUST occur in your program.
