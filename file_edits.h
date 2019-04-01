// Created by Antonis Karvelas.
#ifndef ERGASIA_2_FILE_EDITS_H
#define ERGASIA_2_FILE_EDITS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

int writeIDFile(unsigned long int id, char* commonDirectory);

#endif //ERGASIA_2_FILE_EDITS_H