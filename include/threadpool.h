#ifndef THREADPOOL_H
#define THREADPOOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <semaphore.h>

// number of threads to be created
#define NUM_OF_THREADS 4

// the job structure
typedef struct Job Job;

// the jobqueue structure
typedef struct Job_Queue Job_Queue;

// the thread pool structure
typedef struct Thread_Pool Thread_Pool;

// the thread data
typedef struct thread_data thread_data;

// THREAD POOL

// initialize thread pool
Thread_Pool *Initialize_ThPool(int num_of_threads);

// this is the worker thread who will take jobs from pool
void Worker_Thread(thread_data *tdata);

// when called from a thread, thread will remain idle until
// all jobs in thread pool job queue have finished
void ThPool_Wait(Thread_Pool *threadpool);

// add job to thread pool
void ThPool_AddJob(Thread_Pool *threadpool, void *(*function)(void *),
                   void *args);

// destroy thread pool
void ThPool_Destroy(Thread_Pool *threadpool);

// JOB QUEUE

// initialize job queue
void Initialize_JobQueue(Thread_Pool *threadpool);

// add job to queue
void JobQueue_Add(Thread_Pool *threadpool, Job *job);

// remove first job from queue
void JobQueue_Removefirst(Thread_Pool *threadpool);

// returns the first job from the job queue
Job *JobQueue_Getfirst(Thread_Pool *threadpool);

// destroy job queue
void JobQueue_Destroy(Thread_Pool *threadpool);

#ifdef __cplusplus
}
#endif

#endif /* THREADPOOL_H */