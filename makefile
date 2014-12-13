CC = g++

SOURCES = structs.cpp transformations.c vectors.cpp
main: main.o
	$(CC) -std=c++11 $@.o $(SOURCES) -o main

clean: 
	-rm -r main *.o core *.dSYM