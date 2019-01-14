#include "threadpool_impl.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

// mutex to solve race conditions in adding/removing  to/from jobqueue
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// barrier
pthread_barrier_t barrier;

// indicator for threadpool activity
static int threadpool_running = 1;

void BarrierJob(void *args) {
    pthread_barrier_wait(&barrier);
}

// ------------------------- THREAD POOL OPERATIONS ------------------------- //

// wait for all jobs in jobqueue to be done
void ThPool_Wait(Thread_Pool *threadpool) {
    int i;
    // set the barrier
    pthread_barrier_init(&barrier, NULL, NUM_OF_THREADS + 1);

    for (i = 0; i != NUM_OF_THREADS; i++)
        ThPool_AddJob(threadpool, (void *)BarrierJob, NULL);

    pthread_barrier_wait(&barrier);
    pthread_barrier_destroy(&barrier);
}

// initialize threadpool
Thread_Pool *Initialize_ThPool(int num_of_threads) {
    Thread_Pool *threadpool;

    // thread pool memory allocation
    threadpool = malloc(sizeof(Thread_Pool));
    if (threadpool == NULL) {
        printf("Error: malloc() at 'Initialize_ThPool' [threadpool]\n");
        return NULL;
    }

    // set number of threads that will be used
    threadpool->num_of_threads = num_of_threads;

    // thread pool threads memory allocation
    threadpool->threads = malloc(num_of_threads * sizeof(pthread_t));
    if (threadpool->threads == NULL) {
        printf("Error: malloc() at 'Initialize_ThPool' [threads]\n");
        return NULL;
    }

    // initialize jobqueue
    Initialize_JobQueue(threadpool);

    // jobqueue semaphore memory allocation
    threadpool->jobqueue->jobqueue_sem = malloc(sizeof(sem_t));
    if (threadpool->jobqueue->jobqueue_sem == NULL)
        printf("Error: malloc() at 'Initialize_ThPool' [jobqueue_sem]\n");

    // jobqueue semaphore initialize to zero
    sem_init(threadpool->jobqueue->jobqueue_sem, 0, 0);

    thread_data *tdata = NULL;
    int i;
    // create threadpool threads
    for (i = 0; i < num_of_threads; i++) {
        tdata = malloc(sizeof(thread_data));
        if (tdata == NULL) {
            printf("Error: malloc() at 'Initialize_ThPool' [tdata]\n");
            return NULL;
        }
        tdata->threadpool = threadpool;
        tdata->c_index = i;
        if (pthread_create(&(threadpool->threads[i]), NULL,
                           (void *)Worker_Thread, (void *)tdata) != 0)
            printf("Error: pthread_create() at 'Initialize_ThPool'\n");
    }

    return threadpool;
}

// worker thread running and executing jobs
void Worker_Thread(thread_data *tdata) {
    while (threadpool_running) {
        // wait for job to show up
        if (sem_wait(tdata->threadpool->jobqueue->jobqueue_sem)) {
            printf("Error: sem_wait() at 'Worker_Thread'\n");
            return;
        }

        if (threadpool_running) {
            void *(*function)(void *args);
            match_data *args;
            Job *job;

            pthread_mutex_lock(&mutex); // lock
            // get job from jobqueue
            job = JobQueue_Getfirst(tdata->threadpool);
            function = job->function;
            args = job->args;
            if (args != NULL) // if this not a barrier job
                args->c_index = tdata->c_index;
            JobQueue_Removefirst(tdata->threadpool);

            pthread_mutex_unlock(&mutex); // unlock
            function(args);               // run job function
            free(job);

        } else
            break; // worker is out of business , need to exit
    }
    free(tdata);
}

// add job to thread pool
void ThPool_AddJob(Thread_Pool *threadpool, void *(*function)(void *),
                   void *args) {
    Job *job;

    // job memory allocation
    job = malloc(sizeof(Job));
    if (job == NULL) {
        printf("Error: malloc() at 'ThPool_AddJob' [job]\n");
        return;
    }

    // set the job function and its arguments
    job->function = function;
    job->args = args;

    pthread_mutex_lock(&mutex);    // lock
    JobQueue_Add(threadpool, job); // add job to jobqueue
    pthread_mutex_unlock(&mutex);  // unlock
}

// destroy thread pool
void ThPool_Destroy(Thread_Pool *threadpool) {
    int t;

    // workers need to stop running
    threadpool_running = 0;

    // if there are workers waiting for job awake them
    for (t = 0; t != (threadpool->num_of_threads); t++)
        if (sem_post(threadpool->jobqueue->jobqueue_sem) != 0)
            printf("Error: sem_post() at 'ThPool_Destroy'\n");

    // destroy jobqueue semaphore
    if (sem_destroy(threadpool->jobqueue->jobqueue_sem) != 0)
        printf("Error: sem_destroy() at 'ThPool_Destroy'\n");

    // wait threads to terminate
    for (t = 0; t != (threadpool->num_of_threads); t++)
        pthread_join(threadpool->threads[t], NULL);

    // destroy jobqueue
    JobQueue_Destroy(threadpool);

    // free all memory
    free(threadpool->threads);
    free(threadpool->jobqueue->jobqueue_sem);
    free(threadpool->jobqueue);
    free(threadpool);
}

// -------------------------- JOB QUEUE OPERATIONS -------------------------- //

void Initialize_JobQueue(Thread_Pool *threadpool) {
    // jobqueue memory allocation
    threadpool->jobqueue = malloc(sizeof(Job_Queue));

    if (threadpool->jobqueue == NULL) {
        printf("Error: malloc() at 'Initialize_JobQueue' [jobqueue]\n");
        return;
    }

    threadpool->jobqueue->head = NULL;     // initialize head
    threadpool->jobqueue->tail = NULL;     // initialize tail
    threadpool->jobqueue->num_of_jobs = 0; // there are no jobs yet
}

void JobQueue_Add(Thread_Pool *threadpool, Job *job) {
    job->next = NULL;
    job->prev = NULL;

    Job *cur_last_job;
    cur_last_job = threadpool->jobqueue->tail;

    int num_of_jobs = threadpool->jobqueue->num_of_jobs;

    if (!num_of_jobs) { // jobqueue is empty
        threadpool->jobqueue->head = job;
        threadpool->jobqueue->tail = job;
    } else { // jobqueue is not empty
        cur_last_job->prev = job;
        job->next = cur_last_job;
        threadpool->jobqueue->tail = job;
    }

    (threadpool->jobqueue->num_of_jobs)++;        // increase num of jobs
    sem_post(threadpool->jobqueue->jobqueue_sem); // increase semaphore
}

void JobQueue_Removefirst(Thread_Pool *threadpool) {
    Job *cur_first_job;
    cur_first_job = threadpool->jobqueue->head;

    int num_of_jobs = threadpool->jobqueue->num_of_jobs;

    if (num_of_jobs == 1) { // jobqueue has only one job
        threadpool->jobqueue->head = NULL;
        threadpool->jobqueue->tail = NULL;
    } else { // jobqueue has more than one jobs
        cur_first_job->prev->next = NULL;
        threadpool->jobqueue->head = cur_first_job->prev;
    }

    (threadpool->jobqueue->num_of_jobs)--; // decrease num of jobs
}

Job *JobQueue_Getfirst(Thread_Pool *threadpool) {
    return threadpool->jobqueue->head;
}

void JobQueue_Destroy(Thread_Pool *threadpool) {
    Job *cur_job;
    cur_job = threadpool->jobqueue->head;

    while (threadpool->jobqueue->num_of_jobs) {
        threadpool->jobqueue->head = cur_job->prev;

        // free job memory
        free(cur_job);

        cur_job = threadpool->jobqueue->head;
        threadpool->jobqueue->num_of_jobs--;
    }

    threadpool->jobqueue->head = NULL;
    threadpool->jobqueue->tail = NULL;
}
