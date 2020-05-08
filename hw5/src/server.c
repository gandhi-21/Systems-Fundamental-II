#include "server.h"
#include "debug.h"
#include "csapp.h"
#include "pbx.h"

void *pbx_client_service(void *arg)
{
    int connfd = *((int *)arg);
    Pthread_detach(pthread_self());
    Free(arg);
    TU *tu_client = pbx_register(pbx, connfd);
    debug("client connecetd %p", tu_client);
    FILE *file = fdopen(connfd, "r");
    char user_input[MAXBUF];

    while(1)
    {
        debug("Enter a command");
        fscanf(file, " %[^\r\n]s", user_input);
        debug("String read %s", user_input);
        if(feof(file))
        break;

        if(strcmp(user_input, "pickup") == 0) {
       //     debug("[%d] Pickup", tu_client->extension_number);
            int pickup = tu_pickup(tu_client);
            debug("Return of the pickup is %d", pickup);
        } else if(strcmp(user_input, "hangup") == 0) {
        //    debug("[%d] Hangup", tu_client->extension_number);
            int hangup = tu_hangup(tu_client);
            debug("Return of the hangup is %d", hangup);
        } else if(strncmp(user_input, "dial", 4) == 0) {
            int extension = atoi(user_input + 4);
        //    debug("[%d] DIAL [%d]", tu_client->extension_number, extension);
            int dial = tu_dial(tu_client, extension);
            debug("Return of the dial is %d", dial);
        } else if(strncmp(user_input, "chat", 4) == 0) {
            char *message = user_input + 4;
        //    debug("[%d] CHAT [%s]", tu_client->extension_number, message);
            int chat = tu_chat(tu_client, message);
            debug("Return of chat is %d", chat);
        }
    }

    debug("Closing the server since end of file was received for tu");
    int unregister = pbx_unregister(pbx, tu_client);
    debug("Unregistered the tu client with return status %d", unregister);

    return NULL;
}