CC=gcc --std=gnu99 -g

all: main

main: main.c
	$(CC) main.c -o movie_by_year 

clean:
	rm -f movies *.o
