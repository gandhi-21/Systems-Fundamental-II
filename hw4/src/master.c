#include <stdlib.h>
#include <unistd.h>
#include "debug.h"
#include "polya.h"
#include <string.h>

/**
    Helper stuff for master
*/

struct base_worker{
    pid_t id; // pid of a worker process
    int status; // current worker status
    int fd[2]; //
    int problem_id; 
};

struct base_worker workers_array[MAX_WORKERS];

// ********************************************************************************

/*
 * master
 * (See polya.h for specification.)
 */
int master(int workers) {
    // TO BE IMPLEMENTED
    sf_start();
    debug("Starting the master");

    debug("need to fork and initialize each worker in the array");

    pid_t p;

    for(int i=0;i<workers;i++)
    {
        debug("initialize worker no %d", i+1);

        int fd1_parent[2]; // first pipe
        int fd2_child[2]; // second pipe

        if(pipe(fd1_parent) == -1)
        {
            debug("pipe failed");
        }
        if(pipe(fd2_child) == -1)
        {
            debug("pipe failed");
        }

        int second = fd2_child[0];
        int first = fd1_parent[1];

        p = fork();

        if(p < 0){
            debug("fork failed.");
        } else if (p == 0)
        {
            // child process
            close(fd1_parent[1]);
            close(fd2_child[0]);

            dup2(fd2_child[1], STDOUT_FILENO);
            dup2(fd1_parent[0], STDIN_FILENO);
            debug("child process pid %u", getpid());
            struct base_worker current_worker = {getpid(), 0, {first,second}};
            workers_array[i] = current_worker;

            char *execv_args[] = {"./bin/polya_worker", NULL};

            execv(execv_args[0], execv_args);

        } else {
            // parent process
            close(fd2_child[1]);
            close(fd1_parent[0]);
            debug("parent process pid %u", getpid());
        }
    }

    debug("done initializing all workers and pipes");

    // for(int i=0; i<workers; i++)
    // {
    //     debug("worker number %d", i+1);
    //     debug("worker pid is %u", workers_array[i].id);
    // }



    return EXIT_FAILURE;
}
