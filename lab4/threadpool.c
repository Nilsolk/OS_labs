/**
 * Implementation of thread pool.
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include "threadpool.h"

#define QUEUE_SIZE 10
#define NUMBER_OF_THREADS 3

typedef struct
{
    void (*function)(void *p);
    void *data;
} task;

// обычная кольцевая очередь
static task queue[QUEUE_SIZE];
static int head = 0;
static int tail = 0;
static int count = 0;

// сколько задач сейчас реально выполняется, а не просто лежит в очереди
static int busy = 0;
static int shutting_down = 0;

static pthread_t bees[NUMBER_OF_THREADS];
static pthread_mutex_t queue_lock;
static sem_t work_sem;
static sem_t empty_sem;

// insert a task into the queue
// returns 0 if successful or 1 otherwise
int enqueue(task t)
{
    if (count == QUEUE_SIZE)
        return 1;

    queue[tail] = t;
    tail = (tail + 1) % QUEUE_SIZE;
    count++;

    return 0;
}

// remove a task from the queue
task dequeue()
{
    task t = queue[head];

    head = (head + 1) % QUEUE_SIZE;
    count--;

    return t;
}

static void notify_if_all_tasks_done()
{
    // Если shutdown уже попросили и работы больше нет, будим главный поток.
    if (shutting_down && count == 0 && busy == 0)
        sem_post(&empty_sem);
}

// the worker thread in the thread pool
void *worker(void *param)
{
    (void)param;

    while (1)
    {
        sem_wait(&work_sem);
        pthread_mutex_lock(&queue_lock);
        task t = dequeue();
        busy++;
        pthread_mutex_unlock(&queue_lock);

        execute(t.function, t.data);

        pthread_mutex_lock(&queue_lock);
        busy--;
        notify_if_all_tasks_done();
        pthread_mutex_unlock(&queue_lock);
    }

    return NULL;
}

/**
 * Executes the task provided to the thread pool
 */
void execute(void (*somefunction)(void *p), void *p)
{
    (*somefunction)(p);
}

/**
 * Submits work to the pool.
 */
int pool_submit(void (*somefunction)(void *p), void *p)
{
    task t;
    int result;

    if (somefunction == NULL)
        return 1;

    t.function = somefunction;
    t.data = p;

    pthread_mutex_lock(&queue_lock);

    if (shutting_down)
    {
        pthread_mutex_unlock(&queue_lock);
        return 1;
    }

    result = enqueue(t);

    pthread_mutex_unlock(&queue_lock);

    if (result == 0)
        sem_post(&work_sem);

    return result;
}

// initialize the thread pool
void pool_init()
{
    int i;

    head = 0;
    tail = 0;
    count = 0;
    busy = 0;
    shutting_down = 0;

    pthread_mutex_init(&queue_lock, NULL);
    sem_init(&work_sem, 0, 0);
    sem_init(&empty_sem, 0, 0);

    for (i = 0; i < NUMBER_OF_THREADS; i++)
    {
        if (pthread_create(&bees[i], NULL, worker, NULL) != 0)
        {
            perror("pthread_create умер");
            exit(1);
        }
    }
}

// shutdown the thread pool
void pool_shutdown()
{
    int i;

    pthread_mutex_lock(&queue_lock);
    shutting_down = 1;
    notify_if_all_tasks_done();
    pthread_mutex_unlock(&queue_lock);

    sem_wait(&empty_sem);
    for (i = 0; i < NUMBER_OF_THREADS; i++)
        pthread_cancel(bees[i]);

    for (i = 0; i < NUMBER_OF_THREADS; i++)
        pthread_join(bees[i], NULL);

    sem_destroy(&work_sem);
    sem_destroy(&empty_sem);
    pthread_mutex_destroy(&queue_lock);
}
