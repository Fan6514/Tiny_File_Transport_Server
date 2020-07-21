CFLAGS = -Wall -lpthread

ALL: serv

serv: serv.o socket.o thread.o
	gcc -o serv ${CFLAGS} serv.o socket.o thread.o

serv.o: serv.c
	gcc -c serv.c

socket.o: socket.c
	gcc -c socket.c

thread.o: thread.c
	gcc -c thread.c

clean:
	rm -rf serv *.o

.PHONY: clean ALL