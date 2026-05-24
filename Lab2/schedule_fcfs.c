#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"
#include "list.h"
#include "cpu.h"
#include "schedulers.h"

struct node *head = NULL;

void reverseList()
{
    struct node *prev = NULL;
    struct node *current = head;
    struct node *next;

    while (current != NULL) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }

    head = prev;
}

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
    if (head == NULL)
        return NULL;

    return head->task;
}

void schedule()
{
    Task *nextTask;

    reverseList();

    while (head != NULL) {
        nextTask = pickNextTask();
        run(nextTask, nextTask->burst);
        delete(&head, nextTask);
    }
}
