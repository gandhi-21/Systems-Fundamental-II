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
    // semaphore for the tu object
    sem_t mutex_tu;
};

struct pbx {
    // number of tu registered right now
    int registered_tu;
    // TU array 
    TU *telephone_units[PBX_MAX_EXTENSIONS];
    // is the tu free
    int isFree[PBX_MAX_EXTENSIONS];
    // semaphore for the pbx object
    sem_t mutex_pbx;
};

void print_state(TU *tu);

PBX *pbx_init()
{
    // malloc the pbx structure here
    pbx =  malloc(sizeof(PBX));

    debug("malloced the pbx structure in init");

    sem_init(&pbx->mutex_pbx, 0, 1);

    P(&pbx->mutex_pbx);

    pbx->registered_tu = 0;

    for(int i=0;i<PBX_MAX_EXTENSIONS;i++)
    {pbx->isFree[0] = 0;}

    V(&pbx->mutex_pbx);
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
    P(&pbx->mutex_pbx);
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

    sem_init(&new_extension->mutex_tu, 0, 1);
    P(&new_extension->mutex_tu);

    new_extension->extension_number = index;
    new_extension->file_descriptor = fd;
    new_extension->state = TU_ON_HOOK;


    // Put the new extension in the pbx array
    pbx->telephone_units[index] = new_extension;
    // increase the count of registered tu
    pbx->registered_tu += 1;

    debug("New telephone unit registered at %d", new_extension->extension_number);

    // Return the new extension
    int printed = dprintf(new_extension->file_descriptor, "ON HOOK %d%s", new_extension->extension_number, EOL);

    if(printed == -1)
    {
        V(&new_extension->mutex_tu);
        V(&pbx->mutex_pbx);
        return NULL;
    }


    V(&new_extension->mutex_tu);
    V(&pbx->mutex_pbx);

    return new_extension;
}

int pbx_unregister(PBX *pbx, TU *tu)
{

    P(&pbx->mutex_pbx);
    P(&tu->mutex_tu);

    int index = tu->extension_number;

    // remove the tu from the pbx array
    pbx->telephone_units[index] = NULL;

    // pbx->isFree set to 0
    pbx->isFree[index] = 0;

    V(&tu->mutex_tu);
    // free that tu pointer
    free(tu);

    // decrease the number of registered tu
    pbx->registered_tu -= 1;

    debug("Unregistered the telephone unit at %d", index);

    V(&pbx->mutex_pbx);

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
    debug("[%d] PICKUP", tu->extension_number);
    P(&pbx->mutex_pbx);
    P(&tu->mutex_tu);
    if(tu->state == TU_DIAL_TONE || tu->state == TU_RING_BACK || tu->state == TU_BUSY_SIGNAL || tu->state == TU_ERROR)
    {
        print_state(tu);
        V(&tu->mutex_tu);
    } else if (tu->state == TU_CONNECTED) 
        {
            print_state(tu);
            V(&tu->mutex_tu);
        } else {
                if(tu->state == TU_ON_HOOK)
                    {
                        tu->state = TU_DIAL_TONE;
                        print_state(tu);
                        V(&tu->mutex_tu);
                    } else if (tu->state == TU_RINGING)
                        {
                            P(&pbx->telephone_units[tu->connected]->mutex_tu);
                            tu->state = TU_CONNECTED;
                            pbx->telephone_units[tu->connected]->state = TU_CONNECTED;

                            debug("[%d] CONNECTED [%d]", tu->extension_number, pbx->telephone_units[tu->connected]->extension_number);

                            print_state(tu);
                            print_state(pbx->telephone_units[tu->connected]);
                            V(&pbx->telephone_units[tu->connected]->mutex_tu);
                            V(&tu->mutex_tu);
                        }
                }
    V(&pbx->mutex_pbx);
    return 0;
}

int tu_hangup(TU *tu)
{
    debug("[%d] HANGUP", tu->extension_number);

    P(&pbx->mutex_pbx);
    P(&tu->mutex_tu);

    if(tu->state == TU_ON_HOOK || tu->state == TU_DIAL_TONE || tu->state == TU_BUSY_SIGNAL || tu->state == TU_ERROR)
    {
        tu->state = TU_ON_HOOK;
        print_state(tu);
        V(&tu->mutex_tu);

    } else if(tu->state == TU_RINGING)
        {
            P(&pbx->telephone_units[tu->connected]->mutex_tu);

            tu->state = TU_ON_HOOK;
            pbx->telephone_units[tu->connected]->state = TU_DIAL_TONE;

            print_state(tu);
            print_state(pbx->telephone_units[tu->connected]);

            V(&pbx->telephone_units[tu->connected]->mutex_tu);
            V(&tu->mutex_tu);

        } else if(tu->state == TU_RING_BACK)
            {
                P(&pbx->telephone_units[tu->connected]->mutex_tu);

                tu->state = TU_ON_HOOK;
                pbx->telephone_units[tu->connected]->state = TU_ON_HOOK;

                print_state(tu);
                print_state(pbx->telephone_units[tu->connected]);

                V(&pbx->telephone_units[tu->connected]->mutex_tu);
                V(&tu->mutex_tu);

            } else if(tu->state == TU_CONNECTED)
                {
                    P(&pbx->telephone_units[tu->connected]->mutex_tu);

                    tu->state = TU_ON_HOOK;
                    pbx->telephone_units[tu->connected]->state = TU_DIAL_TONE;

                    print_state(tu);
                    print_state(pbx->telephone_units[tu->connected]);

                    V(&pbx->telephone_units[tu->connected]->mutex_tu);
                    V(&tu->mutex_tu);
                }
    V(&pbx->mutex_pbx);
    return 0;
}

int tu_dial(TU *tu, int ext)
{

    P(&pbx->mutex_pbx);
    P(&tu->mutex_tu);

    debug("[%d] DIAL [%d]", tu->extension_number, ext);

    if(tu->state == TU_DIAL_TONE) 
    {
        if(pbx->isFree[ext] == 0)
            {
                tu->state = TU_ERROR;
                print_state(tu);
                
                V(&tu->mutex_tu);

            } else if(pbx->telephone_units[ext]->state == TU_ON_HOOK)
                {

                    P(&pbx->telephone_units[ext]->mutex_tu);

                    tu->state = TU_RING_BACK;
                    pbx->telephone_units[ext]->state = TU_RINGING;

                    tu->connected = ext;
                    pbx->telephone_units[ext]->connected = tu->extension_number;

                    print_state(tu);
                    print_state(pbx->telephone_units[ext]);

                    V(&pbx->telephone_units[ext]->mutex_tu);
                    V(&tu->mutex_tu);

                } else 
                    {
                        tu->state = TU_BUSY_SIGNAL;
                        print_state(tu);

                        V(&tu->mutex_tu);

                    }
    } else
        {
            print_state(tu);
            
            V(&tu->mutex_tu);

        }

    V(&pbx->mutex_pbx);

    return 0;
}

int tu_chat(TU *tu, char *msg)
{
    debug("[%d] CHAT [%d] : [%s]", tu->extension_number, tu->connected, msg);

    P(&pbx->mutex_pbx);
    P(&tu->mutex_tu);

    if(tu->state == TU_CONNECTED)
    {
        P(&pbx->telephone_units[tu->connected]->mutex_tu);

        print_state(tu);
        dprintf(pbx->telephone_units[tu->connected]->file_descriptor, "CHAT %s%s", msg, EOL);

        V(&pbx->telephone_units[tu->connected]->mutex_tu);
        V(&tu->mutex_tu);

    } else 
        {
            print_state(tu);

            V(&tu->mutex_tu);
            V(&pbx->mutex_pbx);
            return -1;
        }

    V(&pbx->mutex_pbx);

    return 0;
}

void print_state(TU *tu) 
{

    // check the state and print accordingly
    if(tu->state == TU_ON_HOOK || tu->state == TU_CONNECTED)
    {
        if(tu->state == TU_ON_HOOK)
        {
          dprintf(tu->file_descriptor, "%s %d%s", tu_state_names[tu->state], tu->extension_number, EOL);
        } else {
            dprintf(tu->file_descriptor, "%s %d%s", tu_state_names[tu->state], tu->connected, EOL);
        }
    } else {
        dprintf(tu->file_descriptor, "%s%s", tu_state_names[tu->state], EOL);
    }
}