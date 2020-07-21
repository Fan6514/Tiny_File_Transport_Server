#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERV_PORT 8000
#define LISTENMAX 5

ssize_t Recv(int sockfd, void *buff, size_t nbytes, int flags);
ssize_t Send(int sockfd, const void *buff, size_t nbytes, int flags);
void Close(int sockfd);
int Accept(int sockfd, struct sockaddr* cliaddr, socklen_t *clilen);
void Listen(int sockfd, int backlog);
void Bind(int socket, struct sockaddr* servaddr, socklen_t addrlen);
int Socket(int family, int type, int protocol);
int socket_create();