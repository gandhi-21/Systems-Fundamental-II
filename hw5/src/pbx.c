#include "server.h"
#include "debug.h"
#include "csapp.h"
#include "pbx.h"

#include <unistd.h>

struct tu {
    // File Descriptors
    int file_descriptor;
    // extension number
    int extension_number;
    // current state
    TU_STATE state;
};

struct pbx {
    // number of tu registered right now
    int registered_tu;
    // TU array 
    TU *telephone_units[PBX_MAX_EXTENSIONS];
    // is the tu free
    int isFree[PBX_MAX_EXTENSIONS];
};


PBX *pbx_init()
{
    // malloc the pbx structure here
    pbx =  malloc(sizeof(PBX));

    debug("malloced the pbx structure in init");

    pbx->registered_tu = 0;

    for(int i=0;i<PBX_MAX_EXTENSIONS;i++)
    {pbx->isFree[0] = 0;}

    debug("initialized the array so all extensions are free");

    return pbx;
}


void pbx_shutdown(PBX *pbx)
{
    free(pbx);
    pbx = NULL;
}


TU *pbx_register(PBX *pbx, int fd)
{
    // Make a new TU here and add it to the pbx array
    TU *new_extension =  malloc(sizeof(TU));

    // Find the first free extension
    int index=0;
    for(int i=0;i<PBX_MAX_EXTENSIONS;i++)
    {
        if(pbx->isFree[i] == 0)
        {
            index = i;
            pbx->isFree[i] = 1;
            break;
        }
    }

    new_extension->extension_number = index;
    new_extension->file_descriptor = fd;
    new_extension->state = TU_ON_HOOK;


    // Put the new extension in the pbx array
    pbx->telephone_units[index] = new_extension;

    debug("New telephone unit registered at %d", new_extension->extension_number);

    // Return the new extension
    return new_extension;
}

int pbx_unregister(PBX *pbx, TU *tu)
{
    return 0;
}

int tu_fileno(TU *tu)
{
    return 0;
}

int tu_extension(TU *tu)
{
    return 0;
}

int tu_pickup(TU *tu)
{
    return 0;
}

int tu_hangup(TU *tu)
{
    return 0;
}

int tu_dial(TU *tu, int ext)
{
    return 0;
}

int tu_chat(TU *tu, char *msg)
{
    return 0;
}