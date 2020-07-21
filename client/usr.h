#ifndef USR_H_
#define USR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAXLINE 4096

void usr_operator(int socket);
void putfile(int socket, char* filename);

void print_bar(const char *file_name, float sum, float file_size);

#endif