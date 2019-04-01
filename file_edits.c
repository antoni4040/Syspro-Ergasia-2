#include "file_edits.h"

int writeIDFile(unsigned long int id, char* commonDirectory)
{
    // Filepath for the file we want to create:
    char idString[20];
    char* fileToWrite = malloc(sizeof(char) * (strlen(commonDirectory) + 16));
    strcpy(fileToWrite, commonDirectory);
    strcat(fileToWrite, "/");
    sprintf(idString, "%lu", id);
    strcat(fileToWrite, idString);
    strcat(fileToWrite, ".id");

    // Check if file already exists:
    struct stat fileExists;
    if (stat(fileToWrite, &fileExists) == 0 && S_ISREG(fileExists.st_mode))
    {
        fprintf(stderr, ".id file for this client already exists. My existence is pointless. Goodbye...\n");
        return 1;
    }

    // Create file and open it:
    FILE *file = fopen(fileToWrite, "ab+");

    // Get process id and write it to file:
    pid_t processID = getpid();
    char processIDString[20];
    sprintf(processIDString, "%lu", (unsigned long)processID);
    fputs(processIDString, file);

    // Free and return success:
    free(fileToWrite);
    fclose(file);
    return 0;
}