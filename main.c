// Created by Antonis Karvelas.
// Let there be light...
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/inotify.h>

#include "validation.h"
#include "file_edits.h"
#include "linked_list.h"
#include "child_processes.h"

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define EVENT_BUFFER_LEN  (1024*(EVENT_SIZE + 16))

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
        else if(strcmp(argv[i], "-l") == 0)
        {
            i++;
            logFile = malloc(strlen(argv[i]) * sizeof(char));
            strcpy(logFile, argv[i]);
        }
    }

    // Validate parameters and create directories if necessary:
    int validation = validateParameters(commonDirectory, inputDirectory, mirrorDirectory, bufferSize, logFile);
    // If validation fails, give up:
    if(validation == 1)
    {
        return 0;
    }

    // Create .id file.
    int IDFileWritten = writeIDFile(ID, commonDirectory);
    // If .id file with client id already exists, bye-bye...
    if(IDFileWritten == 1)
    {
        return 0;
    }

    // Create clients linked list:
    LinkedList* clientsList = initializeLinkedList();

    // Synchronize with already existing clients:
    synchronizeExistingClients(ID, commonDirectory, clientsList);

    // Initialize inotify:
    int length, i;
    int inotifyInstance;
    int dirWatch;
    char buffer[EVENT_BUFFER_LEN];
    inotifyInstance = inotify_init();

    // Check everything is ok:
    if (inotifyInstance < 0) {
        fprintf(stderr, "Something didn't work right with inotify. Oh well...\n");
        return 1;
    }

    // Add common dir to the list of dirs to check:
    dirWatch = inotify_add_watch( inotifyInstance, commonDirectory, IN_CREATE | IN_DELETE );

    // Now your job is to keep guard for any changes in the common directory:
    while(1)
    {
        // Read until an event occurs:
        length = read(inotifyInstance, buffer, EVENT_BUFFER_LEN);

        // Check for errors in length:
        if (length < 0) {
            fprintf(stderr, "Something didn't work right with inotify. Oh well...\n");
            return 1;
        }

        // Read the changes happening:
        i = 0;
        while(i < length)
        {
            struct inotify_event *event = (struct inotify_event*)&buffer[i];
            if (event->len) {
                if ((event->mask & IN_CREATE) && !(event->mask & IN_ISDIR)) {
                    char *newFilename = malloc(sizeof(char) * strlen(event->name));
                    strcpy(newFilename, event->name);
                    if(isIDFile(newFilename) != 0)
                    {
                        i += EVENT_SIZE + event->len;
                        free(newFilename);
                        continue;
                    }
                    // Get client id from filename:
                    unsigned long int newID = getClientIDFromFilename(newFilename);

                    // Check if id exists in linked list:
                    int idFound = checkClientInLinkedList(newID, clientsList);
                    if(idFound == 0)
                    {
                        fprintf(stderr, "Not the first time I see this id, something didn't go well...\n");
                        continue;
                    }

                    // Add new client to linked list:
                    client* newClient = initializeClient(newID, 0);
                    Node* newClientNode = initializeNode(newClient);
                    appendToLinkedList(clientsList, newClientNode);

                    // Begin synchronization procedure:
                    printf("Begin synchronization.\n");
                    synchronizeClients(ID, newID, commonDirectory);
                }
                else if((event->mask & IN_DELETE) && !(event->mask & IN_ISDIR))
                {
                    char *deletedFilename = malloc(sizeof(char) * strlen(event->name));
                    strcpy(deletedFilename, event->name);
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }
    // Remove the common dir directory from the watch list:
    inotify_rm_watch(inotifyInstance, dirWatch);

    // Close inotify instance:
    close(inotifyInstance);
}