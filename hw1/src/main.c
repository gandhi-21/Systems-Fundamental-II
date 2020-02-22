#include <stdio.h>
#include <stdlib.h>

#include "const.h"
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

int main(int argc, char **argv)
{
    int ret;
    if(validargs(argc, argv))
        USAGE(*argv, EXIT_FAILURE);
    debug("Options: 0x%x", global_options);
    if(global_options & 1)
        USAGE(*argv, EXIT_SUCCESS);

    int bytes = 0;

    if(global_options & (1 << 2)) {
       // debug("decompressing here");
        FILE *in;
        FILE *out;
        in = stdin;
        out = stdout;
        bytes = decompress(in, out);
    }

    if(global_options & (1 << 1)) {
        debug("compressing here");
        FILE *in;
        FILE *out;
        in = stdin;
        out = stdout;
        bytes = compress(in, out, (global_options >> 16));
    }

    if (bytes != EOF)
        {
            return EXIT_SUCCESS;
        } else {
            return EXIT_FAILURE;
        }

}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */