// Created by Antonis Karvelas.
#ifndef ERGASIA_2_VALIDATION_H
#define ERGASIA_2_VALIDATION_H
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

int validateParameters(char* commonDirectory, char* inputDirectory,
    char* mirrorDirectory, unsigned int bufferSize, char* logFile);
unsigned long int getClientIDFromFilename(char* filename);
int isIDFile(char* filename);

#endif //ERGASIA_2_VALIDATION_H