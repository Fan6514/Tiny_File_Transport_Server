CFLAGS = -Wall  -lreadline

ALL: cli

cli: cli.o connection.o usr.o
	gcc -o cli cli.o connection.o usr.o ${CFLAGS}

cli.o: cli.c
	gcc -c cli.c

connection.o: connection.c
	gcc -c connection.c

usr.o: usr.c
	gcc -c usr.c

clean:
	rm -rf cli *.o

.PHONY: clean ALL