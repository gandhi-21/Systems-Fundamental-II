--- hw1/dev_repo/solution/include/sequitur.h

+++ hw1/repos/gagandhi/hw1/include/sequitur.h

@@ -79,9 +79,6 @@

 /* The total number of pre-allocated symbols that we have at our disposal. */

 #define MAX_SYMBOLS 1000000

 

-/* The maximum number of nonterminal symbols (limited by 2^21 Unicode code points). */

-#define SYMBOL_VALUE_MAX (1 << 21)

-

 /* Statically allocated storage for symbols (definition is in const.h). */

 extern SYMBOL symbol_storage[/*MAX_SYMBOLS*/];

 
