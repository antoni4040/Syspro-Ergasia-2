#include "validation.h"


// Check the validity of the command line parameters and create the necessary directories:
int validateParameters(unsigned long int clientID, char* commonDirectory, char* inputDirectory,
    char* mirrorDirectory, unsigned int bufferSize, char* logFile)
{
    // Check if parameters are given:
    if(commonDirectory == NULL || inputDirectory == NULL || mirrorDirectory == NULL ||
        bufferSize == 0 || logFile == NULL)
    {
        fprintf(stderr, "Command line parameters missing. Well done. I had low expecations, but you still managed to dissapoint me...\n");
        return 1;
    }

    struct stat dirExists;

    // Check if input dir doesn't exists:
    if (!(stat(inputDirectory, &dirExists) == 0 && S_ISDIR(dirExists.st_mode)))
    {
        fprintf(stderr, "Input directory doesn't exist. Where did it go?\n");
        return 1;
    }

    // Check if mirror dir exists:
    if (stat(mirrorDirectory, &dirExists) == 0 && S_ISDIR(dirExists.st_mode))
    {
        fprintf(stderr, "Mirror directory exists. This town is too small for two clients...\n");
        return 1;
    }
    // If it doesn't, create it:
    else
    {
        printf("Creating mirror directory for ya...\n");
        mkdir(mirrorDirectory, 0777);
    }

    // Check if common_dir exists:
    if (stat(commonDirectory, &dirExists) == 0 && S_ISDIR(dirExists.st_mode))
    {
        fprintf(stderr, "Common directory exists. But I'm ok with that...\n");
    }
    // If it doesn't, create it:
    else
    {
        printf("Creating common directory for glorious communist regime...\n");
        mkdir(commonDirectory, 0777);
    }

    // Create log file:
    FILE *logFileOpen = fopen(logFile, "ab+");
    // Write client id:
    char clientIDString[20];
    sprintf(clientIDString, "%lu", (unsigned long int )clientID);
    fputs(clientIDString, logFileOpen);
    fputs("\n", logFileOpen);
    fclose(logFileOpen);

    return 0;
}

// Get the client id from the filename client_id.id:
unsigned long int getClientIDFromFilename(char* filename)
{
    unsigned long int id;
    sscanf(filename, "%lu.id", &id);
    return id;
}

// Check filename is an .id file:
int isIDFile(char* filename)
{
    char *ext;
    ext = strrchr(filename, '.');
    if(ext == NULL)
        return 1;
    return strcmp(".id", ext);
}