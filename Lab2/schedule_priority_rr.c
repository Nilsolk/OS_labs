#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"
#include "list.h"
#include "cpu.h"
#include "schedulers.h"

#define NAME_SIZE 100

struct node *head = NULL;
char nextTaskName[NAME_SIZE] = "";

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

int getMaxPriority()
{
    struct node *temp = head;
    int maxPriority = temp->task->priority;

    while (temp != NULL) {
        if (temp->task->priority > maxPriority)
            maxPriority = temp->task->priority;
        temp = temp->next;
    }

    return maxPriority;
}

Task *findTaskByName(char *name)
{
    struct node *temp = head;

    while (temp != NULL) {
        if (strcmp(temp->task->name, name) == 0)
            return temp->task;
        temp = temp->next;
    }

    return NULL;
}

struct node *findNodeByTask(Task *task)
{
    struct node *temp = head;

    while (temp != NULL) {
        if (temp->task == task)
            return temp;
        temp = temp->next;
    }

    return NULL;
}

Task *findNextSamePriority(Task *task)
{
    struct node *current;
    struct node *temp;
    int priority;

    current = findNodeByTask(task);
    if (current == NULL)
        return NULL;

    priority = task->priority;

    temp = current->next;
    while (temp != NULL) {
        if (temp->task->priority == priority)
            return temp->task;
        temp = temp->next;
    }

    temp = head;
    while (temp != current) {
        if (temp->task->priority == priority)
            return temp->task;
        temp = temp->next;
    }

    return NULL;
}

Task *pickNextTask()
{
    struct node *temp;
    Task *nextTask;
    int maxPriority;

    if (head == NULL)
        return NULL;

    maxPriority = getMaxPriority();

    if (nextTaskName[0] != '\0') {
        nextTask = findTaskByName(nextTaskName);
        if (nextTask != NULL && nextTask->priority == maxPriority)
            return nextTask;
    }

    temp = head;
    while (temp != NULL) {
        if (temp->task->priority == maxPriority)
            return temp->task;
        temp = temp->next;
    }

    return NULL;
}

void schedule()
{
    Task *nextTask;
    Task *samePriorityNext;
    int slice;

    reverseList();

    while (head != NULL) {
        nextTask = pickNextTask();
        samePriorityNext = findNextSamePriority(nextTask);

        if (samePriorityNext != NULL)
            strcpy(nextTaskName, samePriorityNext->name);
        else
            nextTaskName[0] = '\0';

        if (nextTask->burst > QUANTUM)
            slice = QUANTUM;
        else
            slice = nextTask->burst;

        run(nextTask, slice);
        nextTask->burst = nextTask->burst - slice;

        if (nextTask->burst == 0) {
            delete(&head, nextTask);
        }

        if (head == NULL)
            nextTaskName[0] = '\0';
    }
}
