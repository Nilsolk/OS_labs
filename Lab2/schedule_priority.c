#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"
#include "list.h"
#include "cpu.h"
#include "schedulers.h"

struct node *head = NULL;

void add(char *name, int priority, int burst)
{
    Task *newTask = malloc(sizeof(Task));

    newTask->name = malloc(strlen(name) + 1);
    strcpy(newTask->name, name);
    newTask->priority = priority;
    newTask->burst = burst;
    newTask->tid = 0;

    insert(&head, newTask);
}

Task *pickNextTask()
{
    struct node *temp = head;
    Task *best = temp->task;

    while (temp != NULL) {
        if (temp->task->priority >= best->priority) {
            best = temp->task;
        }
        temp = temp->next;
    }

    return best;
}

void schedule()
{
    Task *nextTask;

    while (head != NULL) {
        nextTask = pickNextTask();
        run(nextTask, nextTask->burst);
        delete(&head, nextTask);
    }
}
