CC=gcc
CFLAGS=-I.

 mutex_on: 
	$(CC) -o mutex_on mutex_on.c -lpthread $(CFLAGS)

 mutex_off: 
	$(CC) -o mutex_off mutex_off.c $(CFLAGS)

 program2: 
	$(CC) -o p2 p2.c $(CFLAGS)

 program3: 
	$(CC) -o p3 p3.c -lpthread $(CFLAGS)

clean :
	rm mutex_on mutex_off