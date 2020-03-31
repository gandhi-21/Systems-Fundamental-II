#include <stdio.h>
#include "sfmm.h"
#include <debug.h>

int main(int argc, char const *argv[]) {
    sf_mem_init();

    double* ptr = sf_malloc(3960);
    int* ptr2 = sf_malloc(sizeof(int));
    *ptr = 30;
    *ptr2 = 3;
    //printf("%d\n", *ptr);
    sf_show_heap();
    debug("size of pointer is %ld", sizeof(ptr));

    sf_free(ptr);
    debug("back from free 1");
    sf_show_heap();
    sf_free(ptr2);
   // debug("back from free 2");
    sf_show_heap();

    sf_mem_fini();

    return EXIT_SUCCESS;
}
