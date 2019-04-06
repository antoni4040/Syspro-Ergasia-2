// Created by Antonis Karvelas.
#include "child_processes.h"

int synchronizeClients(unsigned long int client1, unsigned long int client2,
    char* commonDir, char* inputDir)
{
    pid_t childA, childB;
    struct stat fifoExists;

    childA = fork();

    if (childA == 0)
    {
        // Child A code, responsible for sending through fifo:
        int fd;
        char buffer[100];
        char id_to_id[128];

        // Create fifo filename:
        char* fifoFile = malloc(sizeof(char) * (strlen(commonDir) + 16));
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
        if((fd=open(fifoFile, O_WRONLY)) < 0)
        {
            perror("Can't open fifo. Maybe trying booting FIFA?");
            exit(1);
        }

        // Get all files from current client input file:
        DIR *directory;
        struct dirent *directory_entry;
        printf("\n Input dir:%s\n", inputDir);
        char* firstDirName = malloc(sizeof(char) * strlen(inputDir));
        strcpy(firstDirName, inputDir);
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
                    char* newDirName = malloc(sizeof(char) * (
                        strlen(directory_entry->d_name) + strlen(dirName) + 1));
                    strcpy(newDirName, dirName);
                    strcat(newDirName, "/");
                    strcat(newDirName, directory_entry->d_name);
                    printf("dir: %s\n", newDirName);
                    Node* newNode = initializeNode(newDirName);
                    appendToLinkedList(toVisit, newNode);
                }
                else
                {
                    printf("File: %s\n", directory_entry->d_name);
                }
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
            char* buffer[100];
            char id_to_id[128];

            // Create fifo filename:
            char* fifoFile = malloc(sizeof(char) * (strlen(commonDir) + 16));
            strcpy(fifoFile, commonDir);
            strcat(fifoFile, "/");
            sprintf(id_to_id, "id%lu_to_id_%lu.fifo", client2, client1);
            strcat(fifoFile, id_to_id);
            // printf("aa %s\n", fifoFile);

            // // If fifo doesn't exist, create it:
            if(mkfifo(fifoFile, 0666) == -1)
            {
                if(errno != EEXIST)
                {
                    perror("sender : mkfifo failed ");
                    exit(1);
                }
            }

            // Open fifo file to read data:
            if((fd=open(fifoFile, O_RDWR | O_NONBLOCK)) < 0)
            {
                perror("Can't open fifo. Let me in, LET ME IN!");
                exit(3);
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
    char* inputDir, LinkedList* clientList)
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
        synchronizeClients(ID, newID, commonDir, inputDir);
        closedir(directory);
    }
}