CC=gcc --std=gnu99 -g

all: main

main: main.c
	$(CC) main.c -o smallsh 

clean:
	rm -f smallsh *.o
