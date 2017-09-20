CC=gcc
EXECUTABLE=spa.out

CFLAGS=-Wall -O3
LDFLAGS=-lm

CPPLIST=ranvec.c

all:
	$(CC) $(CFLAGS) Program.c $(CPPLIST) -o $(EXECUTABLE) $(LDFLAGS) 

clean: 
	@rm -f *.o *.out

