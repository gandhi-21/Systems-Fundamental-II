#include <stdlib.h>
#include <unistd.h>
#include "debug.h"
#include "polya.h"
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
    Helper stuff for master
*/

struct base_worker{
    pid_t id; // pid of a worker process
    int status; // current worker status
    int fd[2]; //
    int problem_id; 
    struct problem *currently_solving;
};

struct base_worker *workers_array[MAX_WORKERS];

int all_workers_idle(int workers)
{
    for(int i=0;i<workers;i++)
    {
        if(workers_array[i]->status != WORKER_IDLE)
        return -1;
    }

    return 0;
}

int getIndex(pid_t pidd)
{
  //  debug("pid looking for is %u", pidd);
    for(int i=0;i<MAX_WORKERS;i++)
    {
        // debug("for array index %d pid is %u", i, workers_array[i]->id);
     //   debug("%d address of worker array in function %p ", i, workers_array[i]);
        if((workers_array[i] != NULL) && workers_array[i]->id == pidd)
        {
            return i;
        }
    }
    return -1;
}

void sigchld_handler(int sig)
{
    //debug("in the sigchild handler");

    int worker_status;
    pid_t child_pid;

    while((child_pid =  waitpid(-1, &worker_status, WNOHANG|WUNTRACED|WCONTINUED|WSTOPPED)) > 0)
    {
       // int returned = WIFSTOPPED(worker_status);
        int did_exit = WIFEXITED(worker_status);
      //  int did_no = WIFSIGNALED(worker_status);
      //  int did_continue = WIFCONTINUED(worker_status);

        //debug("child pid %u and returned is %d and did it exit %d and unkown signal %d and did it continue %d", child_pid, returned, did_exit, did_no, did_continue);
        int index = getIndex(child_pid);
        //debug("index of pid is %d ", index);
        //debug("current state of worker is %d ", workers_array[index]->status);

        if(did_exit && workers_array[index]->status == WORKER_IDLE)
        {
            workers_array[index]->status = WORKER_RUNNING;
            sf_change_state(child_pid, WORKER_IDLE, WORKER_RUNNING);
        }

        if(workers_array[index]->status == WORKER_STARTED)
        {
           // debug("changed from started to idle");
            sf_change_state(child_pid, WORKER_STARTED, WORKER_IDLE);
            workers_array[index]->status = WORKER_IDLE;
        }

        else if(workers_array[index]->status == WORKER_RUNNING)
        {
          //  debug("changed from running to stoppepd");
            if(did_exit != 0) 
            {
                sf_change_state(child_pid, WORKER_RUNNING, WORKER_EXITED);
                workers_array[index]->status = WORKER_EXITED;
            }
            else
            {sf_change_state(child_pid, WORKER_RUNNING, WORKER_STOPPED);
            workers_array[index]->status = WORKER_STOPPED;}
        }

        else if(workers_array[index]->status == WORKER_CONTINUED)
        {
          //  debug("changed from continue to running"); 
                sf_change_state(child_pid, WORKER_CONTINUED, WORKER_RUNNING);
                workers_array[index]->status = WORKER_RUNNING;
        } else {
          //  debug("done nothing");
        }
       // debug("new state of the worker is %d", workers_array[index]->status);
    }
    

  //  debug("end of the sigchld handler");
}

// ********************************************************************************

/*
 * master
 * (See polya.h for specification.)
 */
int master(int workers) {
    // TO BE IMPLEMENTED
    sf_start();
    debug("Starting the master");

    signal(SIGCHLD, sigchld_handler);

    sigset_t mask_child;
    sigemptyset(&mask_child);
    sigaddset(&mask_child, SIGCHLD);


  //  debug("need to fork and initialize each worker in the array");

    pid_t p;

    // for(int i=0;i<MAX_WORKERS;i++)
    // {
    //     workers_array[i] = NULL;
    // }


    for(int i=0;i<workers;i++)
    {
        debug("initialize worker no %d", i+1);

        // block all sigchld for now      
       // sigprocmask(SIG_BLOCK, &mask_child, NULL);
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

            dup2(fd2_child[1], 1);
            dup2(fd1_parent[0], 0);
            debug("child process pid %u", getpid());

            char *execv_args[] = {"./bin/polya_worker", NULL};

            execv(execv_args[0], execv_args);

        } else {
            // parent process
            close(fd2_child[1]);
            close(fd1_parent[0]);
            debug("parent process pid %u", getpid());
            //waitpid(-1, NULL, WUNTRACED);
            struct base_worker *current_worker = (struct base_worker *)malloc(sizeof(struct base_worker));
            current_worker->id = p;
            current_worker->status = WORKER_STARTED;
            current_worker->fd[0] = second;
            current_worker->fd[1] = first;
            workers_array[i] = current_worker;
         //   debug("%p address of the workers array %d", workers_array[i], i);
         //   debug("worker struct pid is %u", workers_array[i]->id);

        }
       /// sigprocmask(SIG_UNBLOCK, &mask_child, NULL);
    }

    debug("done initializing all workers and pipes");

    sigset_t child;
    sigemptyset(&child);
    sigsuspend(&child);
    


    struct problem *current_problem = (struct problem *) (malloc(sizeof(struct problem)));
    struct result *current_result = (struct result *) (malloc(sizeof(struct result)));
    // int solved=0;
    int i=0;
    while(i<workers)
    {
        //debug("workers %u", workers_array[i]->id);
        //sleep(1);

        if((current_problem = get_problem_variant(workers, i)) == NULL)
        {
            debug("done with all the problems. exiting the loop now");
            break;
        }

        if(workers_array[i]->status == WORKER_IDLE)
        {
            // assign the problem to the worker
        //    debug("assigning problem to worker of size %lu", current_problem->size);
            sigprocmask(SIG_BLOCK, &mask_child, NULL);
            
            write(workers_array[i]->fd[1],current_problem, sizeof(struct problem));
         //   debug("sent the header of the problem");
            write(workers_array[i]->fd[1],current_problem->data, current_problem->size - sizeof(struct problem));
            //fwrite(current_problem, sizeof(struct problem), 1, &workers_array[i]->fd[1]);
            //fwrite(current_problem,current_problem->size - sizeof(struct problem), 1, &workers_array[i]->fd[1]);
            fflush(stdout);
            sf_send_problem(workers_array[i]->id, current_problem);
       //     debug("sent the whole problem");
            
            // assign the problem id to the worker
            workers_array[i]->problem_id = current_problem->id;
            workers_array[i]->currently_solving = current_problem;

            sf_change_state(workers_array[i]->id, WORKER_IDLE, WORKER_CONTINUED);
            workers_array[i]->status = WORKER_CONTINUED;

        //    debug("pid is and i is %d and %d ", workers_array[i]->id, i);
         //   debug("sent a problem to the worker with current state %d", workers_array[i]->status);

            sigprocmask(SIG_UNBLOCK, &mask_child, NULL);

            // send a sigcont signal
            kill(workers_array[i]->id, SIGCONT);
            // change the state of the worker

            // sf_change_state(workers_array[i]->id, WORKER_CONTINUED, WORKER_RUNNING);
            // workers_array[i]->status = WORKER_RUNNING;
            
        } 
        
        //sigsuspend(&child);

        if(workers_array[i]->status == WORKER_STOPPED)
        {

          //  debug("worker has stopped now and is reading the result");
            // read the result from the worker
            sigprocmask(SIG_BLOCK, &mask_child, NULL);
            
            read(workers_array[i]->fd[0], current_result, sizeof(struct result));
         //   debug("read the solution header, and the size of %ld and byte read is %d", current_result->size, byte_read);
            struct result *correct_result = (struct result *)(malloc(current_result->size));
            memcpy(correct_result, current_result, sizeof(struct result));
            read(workers_array[i]->fd[0], correct_result->data, correct_result->size - sizeof(struct result));
            //debug("read the whole solution %ld", sizeof(current_result));

            if(correct_result->failed)
            {
                debug("received a failed result possibly from sighup handler");
            } else {
                sf_recv_result(workers_array[i]->id, correct_result);   
                post_result(correct_result, workers_array[i]->currently_solving);   
            }
            sf_change_state(workers_array[i]->id, WORKER_STOPPED, WORKER_IDLE);
            workers_array[i]->status = WORKER_IDLE; 
            sigprocmask(SIG_UNBLOCK, &mask_child, NULL);
            // post the result
            // send sighup to all the workers working on the same problem
            if(!correct_result->failed)
            {
                for(int j=0;j<workers;j++)
                {
                    if(j==i)
                    continue;
                    if(workers_array[j]->problem_id == correct_result->id)
                    {
                        debug("probem was solved and sighup was sent to the worker");
                        sf_cancel(workers_array[j]->id);
                        kill(workers_array[j]->id, SIGHUP);
                    }
                }
            }

        } 

        i++;
        if(i == workers)
        i = 0;
      //  debug("%d ", i);
    }

    for(int i=0;i<workers;i++)
    {
            kill(workers_array[i]->id, SIGCONT);
            kill(workers_array[i]->id, SIGTERM);
    }
    debug("Terminated all workers and ended the program");
    sf_end();

    return EXIT_SUCCESS;
}
