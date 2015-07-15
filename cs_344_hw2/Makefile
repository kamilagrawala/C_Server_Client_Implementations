# Author Kamil Agrawala\
Class: CS344-001\
Homework Assingment #2\
Due: 02/02/15\
One Day late submission used

CC = gcc
Debug = -g
option = -v
file = 1-s.txt
multipleFiles = 123456789012345678901234567890 1-s.txt 2-s.txt 3-s.txt 4-s.txt 5-s.txt
testArchive = A1.oscar
testFile = latin5.txt
CFLAGS = $(DEBUG) -Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement $(DEFINES)
PROGS = myoscar 

all: $(PROGS) -lm

myoscar: myoscar.o oscar.h  
	$(CC) $(CFLAGS) -o $@ $^ -lm

myoscar.o: myoscar.c
	  $(CC) $(CFLAGS) -c $< 

header: 
	ln -s /usr/local/classes/eecs/winter2015/cs344-001/src/Homework2/oscar.h .

clean: 
	rm -rf oscar
	rm -rf *.o
	rm -rf myoscar
	\rm -rf myoscar_archive
	\rm -rf oscar_archive
	\rm -rf *#
	\rm -rf *.#
	\rm -rf *~
	\rm -rf JUNK1 JUNK2 junkFile
	\rm -rf 0-s.txt

#	Git Revision Control System (BitBucket) 
push:
	make clean 
	git add --all
	git commit -am " Automatic Makefile commit"
	git push -u origin master --force

checkout:
	git fetch
	git checkout origin/master myoscar.c

test:
	./myoscar -v myoscar_archive $(file)

testm: 
	./myoscar -v myoscar_archive $(multipleFiles)

optiont:
	./myoscar -t myoscar_archive 

optione:
	./myoscar -e $(testArchive)

comparea:
	cp /usr/local/classes/eecs/winter2015/cs344-001/src/Homework2/oscar ./
	./oscar -a oscar_archive $(multipleFiles) 
	./myoscar -a myoscar_archive $(multipleFiles)
	diff oscar_archive myoscar_archive

comparet:
	cp /usr/local/classes/eecs/winter2015/cs344-001/src/Homework2/oscar ./
	./oscar -t $(testArchive)  > JUNK1
	./myoscar -t $(testArchive) > JUNK2
	diff JUNK1 JUNK2 

compareT:
	cp /usr/local/classes/eecs/winter2015/cs344-001/src/Homework2/oscar ./
	./oscar -T $(testArchive) > JUNK1
	./myoscar -T $(testArchive) > JUNK2
	diff JUNK1 JUNK2

#Checks for Memory leaks
valgrid: 
	valgrind --leak-check=yes -v ./myoscar -t myoscar_archive