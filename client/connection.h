#ifndef CONNECTION_H_
#define CONNECTION_H_
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERV_PORT 8000
#define MAXLINE 4096

void Connect(int socket, struct sockaddr* servaddr, socklen_t addrlen);
int Socket(int family, int type, int protocol);
int serv_conncetion(const char *ipv4);

#endif