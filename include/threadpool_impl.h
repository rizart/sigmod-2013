#ifndef THREADPOOL_IMPL_H
#define THREADPOOL_IMPL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "defn.h"
#include "threadpool.h"

struct Job {
    void *(*function)(void *args); // function to be called from worker
    void *args;                    // functions arguments
    Job *next;                     // pointer to next job
    Job *prev;                     // pointer to previous job
};

struct Job_Queue {
    Job *head;           // head of jobqueue
    Job *tail;           // tail of jobqueue
    sem_t *jobqueue_sem; // sem used for checking if queue is busy or not
    int num_of_jobs;     // number of current jobs in queue
};

struct Thread_Pool {
    pthread_t *threads;  // thread pool threads
    Job_Queue *jobqueue; // thread pool jobqueue
    int num_of_threads;  // number of thread pool threads
};

struct thread_data {
    Thread_Pool *threadpool; // each thread needs to know it's thread pool
    int c_index;             // each thread needs to know which cache can access
};

typedef struct match_data {
    DocID doc_id;
    char *doc_str;
    int c_index;

} match_data;

#ifdef __cplusplus
}
#endif

#endif /* THREADPOOL_IMPL_H */