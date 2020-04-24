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
   // debug("changing the value in sighup");
    cancel_solution = 1;
  //  debug("recieved the sigcont after the sighup sigstop********************************");
}

void sigterm_handler(int sig)
{
   // debug("changing the value in sigterm");
    cancel_solution = 1;
    _exit(0);
}

void sigcont_handler(int sig)
{
  //  debug("this is a sigcont signal and was triggered");
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
    signal(SIGCONT, sigcont_handler);

    // pid_t ipid = getpid();

    sigset_t mask_child;

    sigemptyset(&mask_child);
    sigaddset(&mask_child, SIGHUP);

    // if(initalized == 0)
    // {
    //     // kill(ipid, SIGSTOP);
    //     initalized = 1;
    // }

    // raise(SIGSTOP);

    while(1)
    {    
       // debug("Sending a sigstop signal to master");
        kill(getpid(), SIGSTOP);
        struct problem *header = (struct problem *)(malloc(sizeof(struct problem)));

        sigprocmask(SIG_BLOCK, &mask_child, NULL);

      //  sigsuspend(&prev_one);

       // debug("about to read the problem");

        //fread(header, sizeof(struct problem), 1, stdin);

        read(0, header, sizeof(struct problem));

      //  debug("read the problem from the master");

        problem_actual = (struct problem *)(malloc(header->size));
        memcpy(problem_actual, header, sizeof(struct problem));

        //fread(problem_actual->data, problem_actual->size - sizeof(*header), 1, stdin);
        read(0, problem_actual->data, problem_actual->size - sizeof(struct problem));

       // debug("read the problem data from the master");

        sigprocmask(SIG_UNBLOCK, &mask_child, NULL);

       // debug("read the problem from the master %lu ", problem_actual->size);

        solution = solvers[problem_actual->type].solve(problem_actual, &cancel_solution);

       // debug("done with the solve part");

        sigprocmask(SIG_BLOCK, &mask_child, NULL);
       // sigsuspend(&prev_one);
        if(cancel_solution == 1 || solution == NULL)
        {
            struct result fake_solution = {sizeof(struct result), problem_actual->id, 1, "", ""};
            write(1, &fake_solution, sizeof(struct result));
            cancel_solution = 0;
        } else {
            write(1, solution, sizeof(struct result));
            write(1, solution->data, solution->size - sizeof(struct result));
        }

        fflush(stdout);
        sigprocmask(SIG_UNBLOCK, &mask_child, NULL);

        debug("done with the solution in the worker");
    }

    return EXIT_SUCCESS;
}
