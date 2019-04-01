// Created by Antonis Karvelas.
#ifndef ERGASIA_2_VALIDATION_H
#define ERGASIA_2_VALIDATION_H
#include <stdio.h>
#include <sys/stat.h>

int validateParameters(char* commonDirectory, char* inputDirectory,
    char* mirrorDirectory, unsigned int bufferSize, char* logFile);

#endif //ERGASIA_2_VALIDATION_H