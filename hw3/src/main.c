#include <stdio.h>
#include "sfmm.h"
#include <debug.h>

int main(int argc, char const *argv[]) {
    sf_mem_init();

    double* ptr = sf_malloc(3960);
    sf_show_heap();
    sf_free(ptr);
    //debug("After first free ***************************");
    sf_show_heap();
    int* ptr2 = sf_malloc(sizeof(int));
    sf_free(ptr2);
    //int *ptr3 = sf_malloc(sizeof(int));
    //*ptr2 = 3;
    //*ptr = 4000;
    //*ptr3 = 78;
    //printf("%d\n", *ptr);
    //sf_show_heap();
    //debug("size of pointer is %ld", sizeof(ptr));
    //sf_free(ptr);
    //sf_show_heap();
    debug("back from free 1");
    //sf_free(ptr3);
    //sf_show_heap();
    debug("back from free 2");
    debug("back from free 3");
    //sf_show_heap();
   // sf_free(ptr2);
   // debug("back from free 2");
    sf_show_heap();

    sf_mem_fini();

    return EXIT_SUCCESS;
}
