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

Task *pickNextTask()
{
    Task *nextTask;

    if (head == NULL)
        return NULL;

    if (nextTaskName[0] != '\0') {
        nextTask = findTaskByName(nextTaskName);
        if (nextTask != NULL)
            return nextTask;
    }

    return head->task;
}

void schedule()
{
    Task *nextTask;
    struct node *current;
    int slice;

    reverseList();

    while (head != NULL) {
        nextTask = pickNextTask();
        current = findNodeByTask(nextTask);

        if (current != NULL) {
            if (current->next != NULL) {
                strcpy(nextTaskName, current->next->task->name);
            }
            else if (head != current) {
                strcpy(nextTaskName, head->task->name);
            }
            else {
                nextTaskName[0] = '\0';
            }
        }
        else {
            nextTaskName[0] = '\0';
        }

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
