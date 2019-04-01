// Created by Antonis Karvelas.
// Let there be light...
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    // Initialize command line parameters:
    unsigned long int ID = 0;
    char* commonDirectory = NULL;
    char* inputDirectory = NULL;
    char* mirrorDirectory = NULL;
    unsigned int bufferSize = 0;
    char* logFile = NULL;

    // Read through the command line arguments:
    for(int i = 0; i < argc; i++)
    {
        // Get client ID:
        if(strcmp(argv[i], "-n") == 0)
        {
            i++;
            ID = strtoul(argv[i], NULL, 10);
        }
        // Get common directory:
        else if(strcmp(argv[i], "-c") == 0)
        {
            i++;
            commonDirectory = malloc(strlen(argv[i]) * sizeof(char));
            strcpy(commonDirectory, argv[i]);
        }
        // Get input directory:
        else if(strcmp(argv[i], "-i") == 0)
        {
            i++;
            inputDirectory = malloc(strlen(argv[i]) * sizeof(char));
            strcpy(inputDirectory, argv[i]);
        }
        // Get mirror directory:
        else if(strcmp(argv[i], "-m") == 0)
        {
            i++;
            mirrorDirectory = malloc(strlen(argv[i]) * sizeof(char));
            strcpy(mirrorDirectory, argv[i]);
        }
        // Get buffer size:
        else if(strcmp(argv[i], "-b") == 0)
        {
            i++;
            bufferSize = strtoul(argv[i], NULL, 10);
        }
        // Get log file:
        else if(strcmp(argv[i], "-b") == 0)
        {
            i++;
            logFile = malloc(strlen(argv[i]) * sizeof(char));
            strcpy(logFile, argv[i]);
        }
    }
}