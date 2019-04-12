// Created by Antonis Karvelas.
#include "child_processes.h"

// Use these variables in case we need to retry the transfers:
pid_t childA;
pid_t childB;
int triesChildA = 0;
int triesChildB = 0;
int tryChildA = 1;
int tryChildB = 1;

int synchronizeClients(unsigned long int client1, unsigned long int client2,
    char* commonDir, char* inputDir, char* mirrorDir, char* logFile, unsigned int bufferSize)
{
    // The first time, obviously try to fork. After that, only if something failed:
    while(tryChildA == 1 || tryChildB == 1)
    {
    childA = fork();

    if(childA == 0 && tryChildB == 1)
    {
        // Child A code, responsible for sending through fifo:
        int fd;
        char buffer[bufferSize];
        char id_to_id[256];

        // Data to be transferred through fifo:
        int16_t filenameSize;
        char* filename;
        int32_t contentSize;

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
        // printf("Opening %s for writing.\n", fifoFile);
        if((fd=open(fifoFile, O_WRONLY)) < 0)
        {
            perror("Can't open fifo. Maybe trying booting FIFA?");
            exit(1);
        }

        // Get all files from current client input file:
        DIR *directory;
        struct dirent *directory_entry;
        // printf("\n  Client: %lu Input dir:%s\n", client1, inputDir);
        char* firstDirName = malloc(sizeof(char) * strlen(inputDir));
        strcpy(firstDirName, inputDir);

        // As I want to pass only the relative dir to the fifo, I need a way to get rid of
        // .../inputDir/ and pass only the part after:
        const int dirCharsToRemove = strlen(inputDir) + 1;

        // BFS traversal using a linked list. I prefer it that way, as I have control over
        // the memory being used, unlike the recursive solution.
        Node* firstNode = initializeNode(firstDirName);
        LinkedList* toVisit = initializeLinkedList();
        appendToLinkedList(toVisit, firstNode);


        // While our queue is not empty:
        while(toVisit->head != NULL)
        {
            Node* node = popStart(toVisit);
            char* dirName = (char*)node->item;
            if ((directory = opendir(dirName)) == NULL)
                break;
            // For every subdirectory in a directory:
            while((directory_entry = readdir(directory)) != NULL)
            {
                if(strcmp(directory_entry->d_name, ".") == 0 || strcmp(directory_entry->d_name, "..") == 0)
                    continue;
                if(directory_entry->d_type == DT_DIR)
                {
                    // Create path using parent name and dir name:
                    int16_t dirnameSize;
                    char* newDirName = malloc(sizeof(char) * (
                        strlen(directory_entry->d_name) + strlen(dirName) + 2));
                    strcpy(newDirName, dirName);
                    strcat(newDirName, "/");
                    strcat(newDirName, directory_entry->d_name);
                    // printf("\n  dir: %s\n", newDirName);
                    Node* newNode = initializeNode(newDirName);
                    appendToLinkedList(toVisit, newNode);

                    // Dirname that will be passed to the fifo:
                    char* dirname = malloc(sizeof(char) * (strlen(newDirName) - dirCharsToRemove));
                    strcpy(dirname, newDirName+dirCharsToRemove);

                    // Write 2 bytes for dirname size to fifo:
                    dirnameSize = strlen(dirname);
                    // printf("Client %lu: Writing dirname size %d\n", client1, (int)dirnameSize);
                    if ((write(fd, &dirnameSize, 2)) == -1)
                    { 
                        perror ("Error writing dirname size to fifo.");
                        exit(2);
                    }       

                    // Write dirname to fifo:
                    // printf("Client %lu: Writing dirname %s\n", client1, dirname);
                    if ((write(fd, dirname, (size_t)dirnameSize)) == -1)
                    { 
                        perror ("Error writing dirname to fifo.");
                        exit(2);
                    }

                    // As there is no file content, transmit -1 to inform the receiver:
                    int32_t noFile = -1;
                    // printf("Client %lu: Writing no file.\n", client1);
                    if ((write(fd, &noFile, 4)) == -1)
                    { 
                        perror ("Error writing no file to fifo.");
                        exit(2);
                    }
                    free(dirname);    
                }
                else
                {
                    // Create filename path using directory and filename:
                    filename = malloc(sizeof(char) * (
                        strlen(directory_entry->d_name) + strlen(dirName) + 2));
                    strcpy(filename, dirName);
                    strcat(filename, "/");
                    strcat(filename, directory_entry->d_name);

                    char* filenameToWrite = malloc(sizeof(char) * (strlen(filename) - dirCharsToRemove) + 2);
                    strcpy(filenameToWrite, filename+dirCharsToRemove);

                    // Write 2 bytes for filename size to fifo:
                    filenameSize = strlen(filenameToWrite);
                    // printf("Client %lu: Writing filename size %d\n", client1, (int)filenameSize);
                    if ((write(fd, &filenameSize, 2)) == -1)
                    { 
                        perror ("Error writing filename size to fifo.");
                        exit(2);
                    }       

                    // Write filename to fifo:
                    // printf("Client %lu: Writing filename %s\n", client1, filenameToWrite);
                    if ((write(fd, filenameToWrite, (size_t)filenameSize)) == -1)
                    { 
                        perror ("Error writing filename to fifo.");
                        exit(2);
                    }                                  

                    // Get filesize:
                    struct stat fileStat;
                    stat(filename, &fileStat);
                    contentSize = fileStat.st_size;

                    // // Write content size to fifo:
                    // printf("Client %lu: Writing filesize %d\n", client1, contentSize);
                    if ((write(fd, &contentSize, 4)) == -1)
                    { 
                        perror ("Error writing filesize to fifo.");
                        exit(2);
                    }

                    // printf("%lu %i %s %i\n", client1, filenameSize, filename, contentSize);

                    // Open file:
                    int openFile = open(filename, O_RDONLY);

                    // Get file content:
                    int bytes;
                    while((bytes = read(openFile, buffer, bufferSize)))
                    {
                        if ((write(fd, buffer, bytes)) == -1)
                            { 
                                perror ("Error writing to fifo.");
                                exit(2);
                            }
                    }

                    // Open log file for appending:
                    FILE* logFileOpen = fopen(logFile, "a");

                    // File transfer complete, update log:
                    char* logUpdate = malloc(sizeof(char) * (strlen(filename) + 36));
                    char contentSizeString[20];
                    strcpy(logUpdate, "S ");
                    strcat(logUpdate, filename);
                    sprintf(contentSizeString, "%lu", (unsigned long int)contentSize);
                    strcat(logUpdate, " ");
                    strcat(logUpdate, contentSizeString);                    
                    fputs(logUpdate, logFileOpen);
                    fputs("\n", logFileOpen);

                    free(logUpdate);
                    fclose(logFileOpen);

                    free(filename);
                    free(filenameToWrite);
                    close(openFile);
                }
            }
            free(node->item);
            free(node);
            closedir(directory);
        }
        // Add 00 to fifo to show end of transmition:
        // printf("\nClient %lu: writing end of transmition.\n", client1);
        int16_t end = 0;
        if ((write(fd, &end, 2)) == -1)
        { 
            perror ("Error writing end to fifo.");
            exit(2);
        }
        free(toVisit);
        free(fifoFile);
        close(fd);
        tryChildA = 0;
        exit(0);
    }
    else
    {
        childB = fork();

        if(childB == 0 && tryChildB == 1)
        {
            // Child B code, , responsible for receiving through fifo:
            int fd;
            char buffer[bufferSize];
            char id_to_id[256];
            struct stat dirExists;

            // Create fifo filename:
            char* fifoFile = malloc(sizeof(char) * (strlen(commonDir) + 24));
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
            if(!(stat(mirrorIDDir, &dirExists) == 0 && S_ISDIR(dirExists.st_mode)))
            {
                mkdir(mirrorIDDir, 0777);
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
            // printf("Client %lu: Opening %s for reading.\n", client1, fifoFile);
            if((fd=open(fifoFile, O_RDWR)) < 0)
            {
                perror("Can't open fifo. Open up please!");
                exit(2);
            }

            int filenameSize;
            char* filename;
            int32_t contentSize;

            // Set up waiting period of 30 seconds:
            fd_set readfds;
            struct timeval timeOut;

            timeOut.tv_sec = 10;
            timeOut.tv_usec = 0;
            int selectReturn = 0;

            FD_ZERO(&readfds);
            FD_SET(fd, &readfds);
            selectReturn = select(fd+1, &readfds, (fd_set *)0, (fd_set *)0, &timeOut);

            // If we actually get input from fifo within 30 seconds:
            if(selectReturn > 0)
            {
            for(;;)
            {
                // Get filename size from fifo:
                if(read(fd, &filenameSize, 2) < 0)
                {
                    perror("Can't read filename size.");
                    exit(2);
                }
                // printf("Client %lu: Reading filename size %d\n", client1, (int)filenameSize);

                if(filenameSize == 0)
                {
                    // printf("\n\nTHIS IS THE END. YOU KNOW!\n\n");
                    break;
                }

                // Get filename from fifo:
                filename = malloc(filenameSize + 1);
                if(read(fd, filename, filenameSize) < 0)
                {
                    perror("Can't read filename.");
                    exit(2);
                }
                filename[filenameSize] = '\0';
                // printf("Client %lu: Reading filename %s\n", client1, filename);

                // Get content size from fifo:
                if(read(fd, &contentSize, 4) < 0)
                {
                    perror("Can't read content size.");
                    exit(2);
                }
                // printf("Client %lu: Reading content size %d\n", client1, contentSize);
                
                // Save a copy of content size of the log files:
                int32_t contentSizeCopy = contentSize;

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
                    }
                    free(dirname);
                }
                // If content size is greater than zero, get data to write to file:
                else if(contentSize >= 0)
                {
                    char* fileToWrite = malloc(sizeof(char) * (strlen(mirrorIDDir) + strlen(filename) + 2));
                    strcpy(fileToWrite, mirrorIDDir);
                    strcat(fileToWrite, "/");
                    strcat(fileToWrite, filename);
                    // printf("Client %lu: Writing content to file %s\n", client1, fileToWrite);
                    // Open file with write permissions, create it if it doesn't exist, empty it if it already exists.
                    // Allow the user to read and write that file.
                    int file = open(fileToWrite, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

                    int bytes;
                    while(contentSize > 0)
                    {
                        if(contentSize > bufferSize)
                            bytes = read(fd, buffer, bufferSize);
                        else
                            bytes = read(fd, buffer, contentSize);
                        if ((write(file, buffer, bytes)) == -1)
                        { 
                            perror ("Error writing to file.");
                            exit(2);
                        }
                        contentSize -= bufferSize;
                    }
                    close(file);
                    free(fileToWrite);
                }


                if(contentSizeCopy != -1)
                {
                    // Open log file for appending:
                    FILE* logFileOpen = fopen(logFile, "a");

                    // File transfer complete, update log:
                    char* logUpdate = malloc(sizeof(char) * (strlen(filename) + 36));
                    char contentSizeString[20];
                    strcpy(logUpdate, "R ");
                    strcat(logUpdate, filename);
                    sprintf(contentSizeString, "%lu", (unsigned long int)contentSizeCopy);
                    strcat(logUpdate, " ");
                    strcat(logUpdate, contentSizeString);
                    fputs(logUpdate, logFileOpen);
                    fputs("\n", logFileOpen);

                    free(logUpdate);
                    fclose(logFileOpen);
                }

                free(filename);
            }
            }
            else
            {
                printf("Waited for too long. Aborting this one...\n");
                exit(0);
            }

            free(fifoFile);
            close(fd);
            tryChildB = 0;
            exit(0);
        }
        else
        {
            // Parent code:
            waitpid(childA, NULL, 0);
            waitpid(childB, NULL, 0);
            printf("File transfer from client %lu to client %lu complete.\n", client1, client2);
        }
    }
    }
    return 0;
}

int synchronizeExistingClients(unsigned long int ID, char* commonDir,
    char* inputDir, char* mirrorDir, char* logFile, LinkedList* clientList, unsigned int bufferSize)
{
    struct dirent *directory_entry;
    DIR *directory = opendir(commonDir);

    if(directory == NULL)
    {
        closedir(directory);
        fprintf(stderr, "Can't open the directory God damn it...\n");
        return 0;
    }

    // Read through the common directory to find the prexisting IDs:
    while ((directory_entry = readdir(directory)) != NULL)
    {
        char *newFilename = directory_entry->d_name;
        if(isIDFile(newFilename) != 0)
        {
            continue;
        }
        // Get client id from filename:
        unsigned long int newID = getClientIDFromFilename(newFilename);

        // Not interested in our id:
        if(newID == ID)
        {
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
        client* newClient = initializeClient(newID);
        Node* newClientNode = initializeNode(newClient);
        appendToLinkedList(clientList, newClientNode);

        // Begin synchronization procedure:
        synchronizeClients(ID, newID, commonDir, inputDir, mirrorDir, logFile, bufferSize);
    }
    closedir(directory);
    return 0;
}