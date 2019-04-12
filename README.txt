Created by Antonis Karvelas.

To compile project, simply run make.
Run the mirror client as described in the exercise pdf.
Alternatively, use my python script with:
python3 clients_simulation.py mirror_client 10 0
to create 10 clients. I've included the python script since I made it but I've also shared it
on piazza, so I hope this doesn't contibute on the plagiarism check.
Some parts like the inotify setup, the signals and the select are somewhat taken bits and pieces
from various sources, but it's mostly boilerplate code, nothing that requires thinking.

Main:
The client starts by validating the command line parameters. Then, it setups the singal for it's
termination and then tries to synchronize with prexisting clients before setting up the inotify
to check for additions and deletions in the common directory. If it detects a new .id file,
that means that there's a new client, so it tries to synchronize with that. If an .id file gets
deleted, then the approriate client terminates and all the approriate directories get deleted.

Synchronization:
Forking is done in a while loop and the signal simply flips a switch as to whether to repeat the
loop or not and kills the child processes, in case they're still alive.
If everything goes well, the parent prints a message that the transfer was succesfull.
Else, the loop repeats and if any of the two children need to be executed again, it does, up to 3
more tries each. I don't really like this implementation, but signals are somewhat limiting.

Senders:
The child process responsible for sending data creates/opens a fifo and then it traverses the
directory tree using a queue (I hate recursion, can't handle memory properly) and for every
folder and files sends the approriate data through the fifo. Folders have a content size of -1.

Receivers:
The child process responsible for receiving data creates/opens a fifo and then starts listening
for data up to 30 seconds and then gives up (then it tries 3 more times). It does that using the
select function. If it gets the data, it starts creating folders and files accordingly.
