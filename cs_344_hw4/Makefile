# Author Kamil Agrawala\
Class: CS344-001\
Email: agrawalk@onid.orst.edu\
Homework Assingment #4\
Due: 03/02/15\


CC = gcc
DEBUG = -g
option = -v
CFLAGS = $(DEBUG) -Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement $(DEFINES)
PROGS = posixmsg_server posixmsg_client
testfile = latin1.txt
testfile2 = jargon.txt
testfile3 = words.txt
testfile4 = websters.txt

all: $(PROGS) -lm

posixmsg_server: posixmsg_server.o #posixmsg.h  
	$(CC) $(CFLAGS) -o $@ $^ -lm -lrt

posixmsg_server.o: posixmsg_server.c
	  $(CC) $(CFLAGS) -c $< 

posixmsg_client: posixmsg_client.o #posixmsg.h 
	$(CC) $(CFLAGS) -o $@ $^ -lm -lrt

posixmsg_client.o: posixmsg_client.c 
	$(CC) $(CFLAGS) -c $<

clean: 
	rm -rf *.o
	\rm -rf *#
	\rm -rf *.#
	\rm -rf *~
	\rm -rf JUNK1 JUNK2
	\rm -rf LOGS LOGC
	\rm -rf posixmsg_client posixmsg_server


#	Git Revision Control System (BitBucket) 
push:
	make clean 
	git add --all
	git commit -am " Automatic Makefile Backup"
	git push -u origin master  

checkout:
	git fetch
	git checkout origin/master posixmsg_server.c

checkout2:
	git fetch
	git checkout origin/master posixmsg_client.c

test: 
	./myword_freq $(testfile) | head -25 > JUNK1
	./word_freq $(testfile) | head -25 > JUNK2
	diff JUNK1 JUNK2


test1: 
	./myword_freq $(testfile2) | head -25 > JUNK1
	./word_freq $(testfile2) | head -25 > JUNK2
	diff JUNK1 JUNK2

test2:
	cat $(testfile2) $(testfile2) | ./myword_freq | grep 'ly$$' | tail -1 > JUNK1	
	cat $(testfile2) $(testfile2) | ./word_freq | grep 'ly$$' | tail -1 > JUNK2
	diff JUNK1 JUNK2

test3:
	cat $(testfile3) $(testfile3) | ./myword_freq | grep 'ly$$' | tail -1 > JUNK1	
	cat $(testfile3) $(testfile3) | ./word_freq | grep 'ly$$' | tail -1 > JUNK2
	diff JUNK1 JUNK2

test4:
	cat $(testfile4) $(testfile4) | ./myword_freq | grep 'ly$$' | tail -1 > JUNK1	
	cat $(testfile4) $(testfile4) | ./word_freq | grep 'ly$$' | tail -1 > JUNK2
	diff JUNK1 JUNK2


#Checks for Memory leaks
valgrid: 
	valgrind --leak-check=yes -v <Actual executable command here>