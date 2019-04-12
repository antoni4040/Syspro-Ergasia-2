OBJS    =	main.o  child_processes.o  file_edits.o  linked_list.o  validation.o
SOURCE  =	main.c  child_processes.c  file_edits.c  linked_list.c  validation.c
HEADER  =	child_processes.h  file_edits.h  linked_list.h  validation.h
OUT     =	mirror_client
CC      =	gcc
FLAGS   =       -Wall   -g      -c      -std=c99	-D_XOPEN_SOURCE=700

$(OUT): $(OBJS)
	$(CC)   -g      $(OBJS) -o      $@

main.o: main.c
	$(CC)   $(FLAGS)        main.c

child_processes.o: child_processes.c
	$(CC)   $(FLAGS)        child_processes.c

file_edits.o: file_edits.c
	$(CC)   $(FLAGS)        file_edits.c

linked_list.o: linked_list.c
	$(CC)   $(FLAGS)        linked_list.c

validation.o: validation.c
	$(CC)   $(FLAGS)        validation.c

clean:
	rm -f $(OBJS) $(OUT)

count:
	wc $(SOURCE) $(HEADER)

