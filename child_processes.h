// Created by Antonis Karvelas.
#ifndef ERGASIA_2_CHILD_PROCESSES_H
#define ERGASIA_2_CHILD_PROCESSES_H
#define _BSD_SOURCE
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
# include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include "linked_list.h"
#include "validation.h"

int synchronizeClients(unsigned long int client1, unsigned long int client2,
    char* commonDir, char* inputDir);
int synchronizeExistingClients(unsigned long int ID, char* commonDir,
    char* inputDir, LinkedList* clientList);
#endif //ERGASIA_2_CHILD_PROCESSES_H