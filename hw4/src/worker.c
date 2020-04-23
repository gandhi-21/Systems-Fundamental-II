#include <stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>
#include "debug.h"
#include "polya.h"


struct problem *problem_actual;
struct result *solution;

/*
    Signal handlers for the child process
*/

volatile sig_atomic_t cancel_solution = 0;

// int initalized = 0;
int sigterm = 0;

void sighup_handler(int sig)
{
    debug("changing the value in sighup");
    cancel_solution = 1;
    debug("recieved the sigcont after the sighup sigstop********************************");
}

void sigterm_handler(int sig)
{
    debug("changing the value in sigterm");
    cancel_solution = 1;
    _exit(0);
}


// *************************************************************************

/*
 * worker
 * (See polya.h for specification.)
 */
int worker(void) {
    // TO BE IMPLEMENTED

    signal(SIGHUP, sighup_handler);
    signal(SIGTERM, sigterm_handler);

    // pid_t ipid = getpid();

    sigset_t mask_child, prev_one;

    sigemptyset(&mask_child);
    sigaddset(&mask_child, SIGHUP);

    // if(initalized == 0)
    // {
    //     // kill(ipid, SIGSTOP);
    //     initalized = 1;
    // }

    while(1)
    {    
        debug("Sending a sigstop signal to master");
        raise(SIGSTOP);
        struct problem *header = (struct problem *)(malloc(sizeof(struct problem)));

        sigprocmask(SIG_BLOCK, &mask_child, &prev_one);

        fread(header, sizeof(struct problem), 1, stdin);

        problem_actual = (struct problem *)(malloc(header->size));
        memcpy(problem_actual, header, sizeof(struct problem));

        fread(problem_actual->data, problem_actual->size - sizeof(*header), 1, stdin);

        sigprocmask(SIG_SETMASK, &prev_one, NULL);

        solution = solvers[problem_actual->type].solve(problem_actual, &cancel_solution);


        sigprocmask(SIG_BLOCK, &mask_child, &prev_one);
        if(cancel_solution == 1 || solution == NULL)
        {
            struct result fake_solution = {sizeof(struct result), problem_actual->id, 1, "", ""};
            fwrite(&fake_solution, sizeof(struct result), 1, stdout);
            cancel_solution = 0;
        } else {
            fwrite(solution, sizeof(struct result), 1, stdout);
            fwrite(solution->data, solution->size - sizeof(struct result), 1, stdout);
        }

        fflush(stdout);
        sigprocmask(SIG_SETMASK, &prev_one, NULL);

    }

    return EXIT_SUCCESS;
}
