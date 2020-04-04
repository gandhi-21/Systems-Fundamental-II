#include <stdio.h>
#include "sfmm.h"
#include <debug.h>

int main(int argc, char const *argv[]) {
    sf_mem_init();

    //void * mem = sf_memalign(4, 4096);

    void *x = sf_malloc(sizeof(double));
	void *y = sf_realloc(x, PAGE_SZ << 16);
   
    sf_show_heap();

    if(y != NULL)
    printf("23");
    else 
    printf("21");

    //printf("%p", y);
    //debug("%p", mem);

    // sf_malloc(1600);
    // char *x = sf_malloc(200);
    // sf_memalign(300, 512);
    // sf_free(x);
    // char *y = sf_malloc(2000);
    // sf_realloc(y, 1000);
    // sf_free(y);
    // char *z = sf_malloc(2033);
    // sf_realloc(z, 200);
    // sf_memalign(300, 1024);

    // sf_show_heap();

    sf_mem_fini();

    return EXIT_SUCCESS;
}
