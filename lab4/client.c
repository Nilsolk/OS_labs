/**
 * Example client program that uses thread pool.
 */

#include <stdio.h>
#include <pthread.h>
#include "threadpool.h"

struct data
{
    int a;
    int b;
};

void add(void *param)
{
    struct data *temp;
    temp = (struct data*)param;

    printf("Thread %lu: %d + %d = %d\n",
           pthread_self(), temp->a, temp->b, temp->a + temp->b);
}

int main(void)
{
    int i;
    struct data work[8];

    pool_init();

    for (i = 0; i < 8; i++)
    {
        work[i].a = i;
        work[i].b = i + 10;

        if (pool_submit(&add, &work[i]) != 0)
        {
            printf("task was not added\n");
        }
    }

    pool_shutdown();

    return 0;
}