// Created by Antonis Karvelas.
#ifndef ERGASIA_2_CHILD_PROCESSES_H
#define ERGASIA_2_CHILD_PROCESSES_H
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
# include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <sys/select.h>
#include <sys/time.h>

#include "linked_list.h"
#include "validation.h"

int synchronizeClients(unsigned long int client1, unsigned long int client2,
    char* commonDir, char* inputDir, char* mirrorDir, char* logFile, unsigned int bufferSize);
int synchronizeExistingClients(unsigned long int ID, char* commonDir,
    char* inputDir, char* mirrorDir, char* logFile, LinkedList* clientList, unsigned int bufferSize);
#endif //ERGASIA_2_CHILD_PROCESSES_H