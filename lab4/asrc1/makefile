CC=gcc
CFLAGS=-I.
LDFLAGS=-pthread

all : parallel_min_max process_memory parallel_sum

process_memory: process_memory.c
	$(CC) -o process_memory process_memory.c $(CFLAGS)

parallel_min_max : utils.o find_min_max.o utils.h find_min_max.h
	$(CC) -o parallel_min_max utils.o find_min_max.o parallel_min_max.c $(CFLAGS)

utils.o : utils.h
	$(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o : utils.h find_min_max.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)
	
	sum.o : sum.h
	$(CC) -o sum.o -c sum.c $(CFLAGS)

parallel_sum : utils.o sum.o utils.h sum.h parallel_sum.c
	$(CC) -o psum utils.o sum.o parallel_sum.c $(CFLAGS) $(LDFLAGS)

clean :
	rm utils.o sum.o find_min_max.o parallel_min_max process_memory
