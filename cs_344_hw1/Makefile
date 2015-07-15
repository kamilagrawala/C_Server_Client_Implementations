# Author: Kamil Agrawala\
* Email: agrawalk@onid.orst.edu\
* Class: Cs344-001\
* Assingment #1\
* Programming Projects Makefile\

cc = gcc
Debug = -g
testFile = latin5.txt
CFLAGS = $(DEBUG) -Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement $(DEFINES)
PROGS = rm_ws 
SOURCES=

all: $(PROGS)

rm_ws: rm_ws.o
	$(cc) $(CFLAGS) -o $@ $^

rm_ws.o: rm_ws.c
	  $(cc) $(CFLAGS) -c $< 

clean: 
	\rm -rf *.o
	\rm -rf rm_ws_test
	\rm -rf JUNK1 JUNK2
	\rm -rf rm_ws
	\rm -rf *~
	\rm -rf *#
#git add --all
#git commit -am " Automatic Makefile commit"
#git push -u origin master 

test: 
	cp /usr/local/classes/eecs/winter2015/cs344-001/src/Homework1/rm_ws ./rm_ws_test
	./rm_ws -b < $(testFile) > JUNK1
	./rm_ws_test -b < $(testFile) > JUNK2
	diff JUNK1 JUNK2

	./rm_ws -i < $(testFile) > JUNK1
	./rm_ws_test -i < $(testFile) > JUNK2
	diff JUNK1 JUNK2

	./rm_ws -e < $(testFile) > JUNK1
	./rm_ws_test -e < $(testFile) > JUNK2
	diff JUNK1 JUNK2

	./rm_ws -b -e -i < $(testFile) > JUNK1
	./rm_ws_test -b -e -i < $(testFile) > JUNK2
	diff JUNK1 JUNK2 

	./rm_ws -b -e < $(testFile) > JUNK1
	./rm_ws_test -b -e < $(testFile) > JUNK2
	diff JUNK1 JUNK2

	./rm_ws -b -i < $(testFile) > JUNK1
	./rm_ws_test -b -i < $(testFile) > JUNK2
	diff JUNK1 JUNK2

	./rm_ws -e -i < $(testFile) > JUNK1
	./rm_ws_test -e -i < $(testFile) > JUNK2
	diff JUNK1 JUNK2
