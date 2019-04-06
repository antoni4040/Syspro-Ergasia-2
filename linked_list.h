// Created by Antonis Karvelas.
// Here lies the generic linked list, plus a few client specific things:
#ifndef ERGASIA_1_LINKEDLIST_H
#define ERGASIA_1_LINKEDLIST_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct client
{
    unsigned long int id;
    pid_t process_id;
} client;

typedef struct Node
{
    void* item;
    struct Node* next;
    struct Node* previous;
} Node;

typedef struct LinkedList
{
    Node* head;
    Node* tail;
} LinkedList;

// Generic part:
Node* initializeNode(void* item);
LinkedList* initializeLinkedList();
void appendToLinkedList(LinkedList* linkedList, Node* node);
void freeLinkedList(LinkedList* linkedList);
Node* popStart(LinkedList* linkedList);

// Client part:
client* initializeClient(unsigned long int id, pid_t processID);
int checkClientInLinkedList(unsigned long int id, LinkedList* linkedList);
#endif //ERGASIA_1_LINKEDLIST_H