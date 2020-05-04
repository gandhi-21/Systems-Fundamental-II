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
    // currently connected to what extension
    int connected;
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
    // increase the count of registered tu
    pbx->registered_tu += 1;

    debug("New telephone unit registered at %d", new_extension->extension_number);

    // Return the new extension
    dprintf(new_extension->file_descriptor, "ON HOOK %d%s", new_extension->extension_number, EOL);

    return new_extension;
}

int pbx_unregister(PBX *pbx, TU *tu)
{

    int index = tu->extension_number;

    // remove the tu from the pbx array
    pbx->telephone_units[tu->extension_number] = NULL;

    // pbx->isFree set to 0
    pbx->isFree[tu->extension_number] = 0;

    // free that tu pointer
    free(tu);

    // decrease the number of registered tu
    pbx->registered_tu -= 1;

    debug("Unregistered the telephone unit at %d", index);

    return 0;
}

int tu_fileno(TU *tu)
{
    return tu->file_descriptor;
}

int tu_extension(TU *tu)
{
    return tu->extension_number;
}

int tu_pickup(TU *tu)
{
    if(tu->state == TU_RINGING)
    {
        // change the state to connected 
        tu->state = TU_CONNECTED;
        pbx->telephone_units[tu->connected]->state = TU_CONNECTED;
        debug("Telephone unit %d connected to %d", tu->extension_number, tu->connected);
        dprintf(tu->file_descriptor, "CONNECTED %d%s", tu->connected, EOL);
        dprintf(pbx->telephone_units[tu->connected]->file_descriptor, "CONNECTED %d%s", tu->extension_number, EOL);
    } else {
        tu->state = TU_DIAL_TONE;
        debug("Telephone unit %d now in dial tone", tu->extension_number);
        dprintf(tu->file_descriptor, "DIAL TONE%s", EOL);
    }
    return 0;
}

int tu_hangup(TU *tu)
{
    
    // if tu is connected state then the tu goes to the on hook state
        // the tu it is connected to also goes to the tu dial tone state
    if(tu->state == TU_CONNECTED)
    {
        tu->state = TU_ON_HOOK;
        pbx->telephone_units[tu->connected]->state = TU_DIAL_TONE;
        dprintf(pbx->telephone_units[tu->connected]->file_descriptor, "DIAL TONE%s", EOL);
        debug("sent a dial tone status to %d extension", pbx->telephone_units[tu->connected]->extension_number);
    }
    // if tu is in ring back state then the tu goes to the on hook state
        // the tu in the ringing state goes to the tu on hook state
    else if (tu->state == TU_RING_BACK)
    {
        tu->state = TU_ON_HOOK;
        pbx->telephone_units[tu->connected]->state = TU_ON_HOOK;
        dprintf(pbx->telephone_units[tu->connected]->file_descriptor, "ON HOOK %d%s", pbx->telephone_units[tu->connected]->extension_number, EOL);
    }
    // if tu is in the ringing state then it goes to tu on hook state
        // the tu connected goes to the dial tone state
    else if(tu->state == TU_RINGING)
    {
        tu->state = TU_ON_HOOK;
        pbx->telephone_units[tu->connected]->state = TU_DIAL_TONE;
        dprintf(pbx->telephone_units[tu->connected]->file_descriptor, "DIAL TONE%s", EOL);
    }
    // if tu is in dial tone, busy signal or error state it goes to the on hook state
    else if(tu->state == TU_DIAL_TONE || tu->state == TU_BUSY_SIGNAL || tu->state == TU_ERROR)
    {
        tu->state = TU_ON_HOOK;
    }

    // send the tu client its new state
    dprintf(tu->file_descriptor, "ON HOOK %d%s", tu->extension_number, EOL);
    // remove the connected variable (i don't think this matters since pickup is based on state and not on the connected variable)
    // send the other client its new state in the if statements
    return 0;
}

int tu_dial(TU *tu, int ext)
{

    // if the extension being called does not exist move tu to tu_error state
    if (pbx->isFree[ext] == 0)
    {
        // the ext does not exist
        tu->state = TU_ERROR;
    }
    // if the tu is in the dial tone state
        // if ext is in the on hook state, move tu to ring back state and ext status moves to the ringing state
        // if ext is not in on_hook state, move tu to tu busy signal and no change to the ext state
    // if the tu was not in the dial tone state, do nothing
    if (tu->state == TU_DIAL_TONE)
    {
        TU *extension = pbx->telephone_units[ext];
        if(extension->state == TU_ON_HOOK)
        {
            // move tu to ring back state and ext status to ringing state
            extension->state = TU_RINGING;
            tu->state = TU_RING_BACK;
        } else {
            // move tu to the busy signal state
            tu->state = TU_BUSY_SIGNAL;
        }
    } else {
        // do nothing 
    }


    // a notification is sent to the client about the new state 
    // if the new state is tu ring back , then the ext is also notified of its' new state tu ringing
    if(tu->state == TU_RING_BACK)
    {
        // put the new extension connected value
        TU *extension = pbx->telephone_units[ext];
        extension->connected = tu->extension_number;
        tu->connected = extension->extension_number;
        // send ring back to tu client
        dprintf(tu->file_descriptor, "RING BACK%s", EOL);
        dprintf(extension->file_descriptor, "RINGING%s", EOL);
        // send ringing to the ext client
    } else if(tu->state == TU_ERROR) {
        // send the error to the tu client
        dprintf(tu->file_descriptor, "ERROR%s", EOL);
    } else if(tu->state == TU_BUSY_SIGNAL) {
        // send the busy siganl to the ru client
        dprintf(tu->file_descriptor, "BUSY SIGNAL%s", EOL);
    } else {
        dprintf(tu->file_descriptor, "ON HOOK %d%s", tu->extension_number, EOL);
    }

    return 0;
}

int tu_chat(TU *tu, char *msg)
{
    return 0;
}