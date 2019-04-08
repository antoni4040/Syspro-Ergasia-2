// Created by Antonis Karvelas.
#include "child_processes.h"

int synchronizeClients(unsigned long int client1, unsigned long int client2,
    char* commonDir, char* inputDir, char* mirrorDir)
{
    pid_t childA, childB;
    struct stat fifoExists;
    int bufferSize = 100;

    childA = fork();

    if (childA == 0)
    {
        // Child A code, responsible for sending through fifo:
        int fd;
        char buffer[bufferSize];
        char id_to_id[256];

        // Data to be transferred through fifo:
        unsigned short int filenameSize;
        char* filename;
        int contentSize;
        char* content;

        // Create fifo filename:
        char* fifoFile = malloc(sizeof(char) * (strlen(commonDir) + 24));
        strcpy(fifoFile, commonDir);
        strcat(fifoFile, "/");
        sprintf(id_to_id, "id%lu_to_id_%lu.fifo", client1, client2);
        strcat(fifoFile, id_to_id);
        // printf("bb %s\n", fifoFile);

        // If fifo doesn't exist, create it:
        if(mkfifo(fifoFile, 0666) == -1)
        {
            if(errno != EEXIST)
            {
                perror("sender : mkfifo failed ");
                exit(1);
            }
        }

        // Open fifo file to write data:
        printf("Opening %s for writing.\n", fifoFile);
        if((fd=open(fifoFile, O_WRONLY)) < 0)
        {
            perror("Can't open fifo. Maybe trying booting FIFA?");
            exit(1);
        }

        // Get all files from current client input file:
        DIR *directory;
        struct dirent *directory_entry;
        printf("\n  Client: %lu Input dir:%s\n", client1, inputDir);
        char* firstDirName = malloc(sizeof(char) * strlen(inputDir));
        strcpy(firstDirName, inputDir);

        // As I want to pass only the relative dir to the fifo, I need a way to get rid of
        // .../inputDir/ and pass only the part after:
        int dirCharsToRemove = stlen(firstDirName) + 1;

        // BFS traversal using a linked list. I prefer it that way, as I have control over
        // the memory being used, unlike the recursive solution.
        Node* firstNode = initializeNode(firstDirName);
        LinkedList* toVisit = initializeLinkedList();
        appendToLinkedList(toVisit, firstNode);

        while(toVisit->head != NULL)
        {
            Node* node = popStart(toVisit);
            char* dirName = (char*)node->item;
            if ((directory = opendir(dirName)) == NULL)
                break;
            while((directory_entry = readdir(directory)) != NULL)
            {
                if(strcmp(directory_entry->d_name, ".") == 0 || strcmp(directory_entry->d_name, "..") == 0)
                    continue;
                if(directory_entry->d_type == DT_DIR)
                {
                    unsigned short int dirnameSize;
                    char* newDirName = malloc(sizeof(char) * (
                        strlen(directory_entry->d_name) + strlen(dirName) + 1));
                    strcpy(newDirName, dirName);
                    strcat(newDirName, "/");
                    strcat(newDirName, directory_entry->d_name);
                    // printf("dir: %s\n", newDirName);
                    Node* newNode = initializeNode(newDirName);
                    appendToLinkedList(toVisit, newNode);

                    // Write 2 bytes for dirname size to fifo:
                    dirnameSize = strlen(newDirName);
                    printf("Writing %hu\n", dirnameSize);
                    if ((write(fd, &dirnameSize, sizeof(short int))) == -1)
                    { 
                        perror ("Error writing dirname size to fifo.");
                        exit(2);
                    }       

                    // Write dirname to fifo:
                    if ((write(fd, newDirName, dirnameSize)) == -1)
                    { 
                        perror ("Error writing dirname to fifo.");
                        exit(2);
                    }

                    // As there is no file content, transmit -1 to inform the receiver:
                    int noFile = -1;
                    if ((write(fd, &noFile, sizeof(int))) == -1)
                    { 
                        perror ("Error writing no file to fifo.");
                        exit(2);
                    }     
                }
                else
                {
                    int fifoW;

                    // Create filename path using directory and filename:
                    filename = malloc(sizeof(char) * (
                        strlen(directory_entry->d_name) + strlen(dirName) + 2));
                    strcpy(filename, dirName);
                    strcat(filename, "/");
                    strcat(filename, directory_entry->d_name);

                    // Write 2 bytes for filename size to fifo:
                    filenameSize = strlen(filename);
                    printf("Writing %hu\n", filenameSize);
                    if ((write(fd, &filenameSize, sizeof(short int))) == -1)
                    { 
                        perror ("Error writing filename size to fifo.");
                        exit(2);
                    }       

                    // Write filename to fifo:
                    if ((write(fd, filename, filenameSize)) == -1)
                    { 
                        perror ("Error writing filename to fifo.");
                        exit(2);
                    }                                  

                    // Get filesize:
                    struct stat fileStat;
                    stat(filename, &fileStat);
                    contentSize = fileStat.st_size;

                    // // Write filesize to fifo:
                    if ((write(fd, &contentSize, sizeof(int))) == -1)
                    { 
                        perror ("Error writing filesize to fifo.");
                        exit(2);
                    }

                    // printf("%lu %i %s %i\n", client1, filenameSize, filename, contentSize);

                    // Open file:
                    int openFile = open(filename, O_RDONLY);

                    // Get file content:
                    int bytes = read(openFile, buffer, bufferSize);
                    while(1)
                    {
                        if(bytes > 0)
                        {
                            if ((write(fd, buffer, bytes)) == -1)
                            { 
                                perror ("Error writing to fifo.");
                                exit(2);
                            }
                            // We reached the end:
                            if(bytes < bufferSize)
                            {
                                break;
                            }
                            bytes = read(openFile, buffer, bufferSize);
                        }
                        else
                        {
                            perror("Error reading the file.");
                            exit(2);
                        }
                    }

                    // printf("File: %s\n", filename);
                    // free(filename);
                }
            }
            // Add 00 to fifo to show end of transmition:
            unsigned short int end = 0;
            if ((write(fd, &end, sizeof(short int))) == -1)
            { 
                perror ("Error writing end to fifo.");
                exit(2);
            }
            free(node->item);
            free(node);
            closedir(directory);
        }
        free(toVisit);
        free(fifoFile);
    }
    else
    {
        childB = fork();

        if (childB == 0)
        {
            // Child B code, , responsible for receiving through fifo:
            int fd;
            char buffer[bufferSize];
            char id_to_id[256];
            struct stat dirExists;

            // Create fifo filename:
            char* fifoFile = malloc(sizeof(char) * (strlen(commonDir) + 16));
            strcpy(fifoFile, commonDir);
            strcat(fifoFile, "/");
            sprintf(id_to_id, "id%lu_to_id_%lu.fifo", client2, client1);
            strcat(fifoFile, id_to_id);
            // printf("aa %s\n", fifoFile);

            // Check if client id dir exists in the mirror directory and create it:
            char* mirrorIDDir = malloc(sizeof(char) * (strlen(mirrorDir) + 10));
            char idString[10];
            strcpy(mirrorIDDir, mirrorDir);
            strcat(mirrorIDDir, "/");
            sprintf(idString, "%lu", client2);
            strcat(mirrorIDDir, idString);
            if (!(stat(mirrorIDDir, &dirExists) == 0 && S_ISDIR(dirExists.st_mode)))
            {
                mkdir(mirrorIDDir, 0777);
                return 1;
            }

            // If fifo doesn't exist, create it:
            if(mkfifo(fifoFile, 0666) == -1)
            {
                if(errno != EEXIST)
                {
                    perror("receiver : mkfifo failed ");
                    exit(1);
                }
            }

            // Open fifo file to read data:
            printf("Opening %s for reading.\n", fifoFile);
            while((fd=open(fifoFile, O_RDONLY)) < 0)
            {
                perror("Can't open fifo. Open up please!");
                exit(2);
            }
            
            unsigned short int filenameSize;
            char* filename;
            int contentSize;

            while(1)
            {
                // Get filename size from fifo:
                if(read(fd, &filenameSize, sizeof(unsigned short int)) < 0)
                {
                    perror("Can't read filename size.");
                    exit(2);
                }
                printf("Reading filename size %hu\n", filenameSize);

                // Get filename from fifo:
                filename = malloc(filenameSize * sizeof(char));
                if(read(fd, filename, filenameSize) < 0)
                {
                    perror("Can't read filename.");
                    exit(2);
                }
                printf("Reading filename %s\n", filename);

                // Create directories and file:


                // Get content size from fifo:
                if(read(fd, &contentSize, sizeof(int)) < 0)
                {
                    perror("Can't read content size.");
                    exit(2);
                }
                printf("Reading content size %d\n", contentSize);

                // If content size is -1, then there is no content, make a directory:
                if(contentSize == -1)
                {
                    char* dirname = malloc(sizeof(char) * (strlen(mirrorIDDir) + strlen(filename) + 2));
                    strcpy(dirname, mirrorIDDir);
                    strcat(dirname, "/");
                    strcat(dirname, filename);
                    // If the directory doesn't exist, create it:
                    if (!(stat(dirname, &dirExists) == 0 && S_ISDIR(dirExists.st_mode)))
                    {
                        mkdir(dirname, 0777);
                        return 1;
                    }
                }
                else if(contentSize > 0)
                {
                    char* fileToWrite = malloc(sizeof(char) * (strlen(mirrorIDDir) + strlen(filename)));
                    int file = open(filename, O_WRONLY | O_CREAT);

                    int bytes = read(fd, buffer, bufferSize);
                    while(1)
                    {
                        if(bytes > 0)
                        {
                            if ((write(file, buffer, bytes)) == -1)
                            { 
                                perror ("Error writing to file.");
                                exit(2);
                            }
                            // We reached the end:
                            if(bytes < bufferSize)
                            {
                                break;
                            }
                            bytes = read(fd, buffer, bufferSize);
                        }
                        else
                        {
                            perror("Error reading the fifo.");
                            exit(2);
                        }
                    }
                }

                // If content size is greater than zero, get data to write to file:

                
                printf("Reading content %.*s\n", bufferSize, buffer);
            }

            free(fifoFile);
        }
        else
        {
            // Parent code:
        }
    }
}

int synchronizeExistingClients(unsigned long int ID, char* commonDir,
    char* inputDir, char* mirrorDir, LinkedList* clientList)
{
    struct dirent *directory_entry;
    DIR *directory = opendir(commonDir);

    if(directory == NULL)
    {
        closedir(directory);
        fprintf(stderr, "Can't open the directory God damn it...\n");
        return 0;
    }

    while ((directory_entry = readdir(directory)) != NULL)
    {
        char *newFilename = directory_entry->d_name;
        if(isIDFile(newFilename) != 0)
        {
            continue;
        }
        // Get client id from filename:
        unsigned long int newID = getClientIDFromFilename(newFilename);

        if(newID == ID)
        {
            closedir(directory);
            continue;
        }

        // Check if id exists in linked list:
        int idFound = checkClientInLinkedList(newID, clientList);
        if(idFound == 0)
        {
            closedir(directory);
            fprintf(stderr, "Not the first time I see this id, something didn't go well...\n");
            continue;
        }

        // Add new client to linked list:
        client* newClient = initializeClient(newID, 0);
        Node* newClientNode = initializeNode(newClient);
        appendToLinkedList(clientList, newClientNode);

        // Begin synchronization procedure:
        printf("Begin synchronization.\n");
        synchronizeClients(ID, newID, commonDir, inputDir, mirrorDir);
        closedir(directory);
    }
}