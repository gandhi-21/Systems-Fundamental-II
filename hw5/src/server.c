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
            debug("Pickup the phone here.");
            int pickup = tu_pickup(tu_client);
            debug("Return of the pickup is %d", pickup);
        } else if(strcmp(user_input, "hangup") == 0) {
            debug("Hangup the phone here.");
            int hangup = tu_hangup(tu_client);
            debug("Return of the hangup is %d", hangup);
        } else if(strcmp(user_input, "dial") >= 0) {
            debug("Dial a extension here.");
            int extension = atoi(user_input + 4);
            debug("extension is %d", extension);
            int dial = tu_dial(tu_client, extension);
            debug("Return of the dial is %d", dial);
        } else if(strcmp(user_input, "chat") >= 0) {
            debug("Chat started with a number here");
            char *message = user_input + 4;
            debug("Message is %s", message);
            int chat = tu_chat(tu_client, message);
            debug("Return of chat is %d", chat);
        }
    }

    debug("Closing the server since end of file was received");
    int unregister = pbx_unregister(pbx, tu_client);
    debug("Unregistered the tu client with return status %d", unregister);

    return NULL;
}