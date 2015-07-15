# Author Kamil Agrawala\
Class: CS344-001\
Email: agrawalk@onid.orst.edu\
Homework Assingment #5\
Due: 03/15/15\

CC = gcc
DEBUG = -g
option = -v
CFLAGS = $(DEBUG) -Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement $(DEFINES)
PROGS = myhostdb socket_server socket_client
testfile = latin1.txt
testfile2 = jargon.txt
testfile3 = words.txt
testfile4 = websters.txt

all: $(PROGS) -lm

myhostdb: myhostdb.o 
	$(CC) $(CFLAGS) -o $@ $^ -lm -lrt

myhostdb.o: myhostdb.c 
	  $(CC) $(CFLAGS) -c $< 

socket_server: socket_server.o  
	$(CC) $(CFLAGS) -o $@ $^ -lm -lrt

socket_server.o: socket_server.c
	$(CC) $(CFLAGS) -c $<

socket_client: socket_client.o  
	$(CC) $(CFLAGS) -o $@ $^ -lm -lrt

socket_client.o: socket_client.c
	$(CC) $(CFLAGS) -c $<


clean: 
	rm -rf *.o
	\rm -rf *#
	\rm -rf *.#
	\rm -rf *~
	\rm -rf JUNK1 JUNK2
	\rm -rf LOGS LOGC
	\rm -rf myhostdb.o
	\rm -rf socket_server.o
	\rm -rf socket_client.o


#	Git Revision Control System (BitBucket) 
push:
	make clean 
	git add --all
	git commit -am " Automatic Makefile Backup"
	git push -u origin master  

checkout:
	git fetch
	git checkout origin/master myhostdb.c

checkout2:
	git fetch
	git checkout origin/master socket_server.c

checkout3:
	git fetch
	git checkout origin/master socket_client.c


#Checks for Memory leaks
valgrid: 
	valgrind --leak-check=yes -v ./socket_server 10080 