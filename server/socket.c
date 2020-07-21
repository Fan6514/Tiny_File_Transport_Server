/********************************************************************************
 * File name : socket.c
 * author : fan
 * date : 2020-07-07 
 * purpose : 	socket 创建以及 accept、listen、bind 和 socket 的包裹函数，并且封装了
				一些常用的接收发送 recv() 和 send() 函数的包裹函数
 * notice : socket_create 函数用来创建 TCP 连接
 * *******************************************************************************/
#include "socket.h"

ssize_t Recv(int sockfd, void *buff, size_t nbytes, int flags)
{
	int res = recv(sockfd, buff, nbytes, flags);
	if (res < 0) {
		perror("recv error");
		exit(1);
	}
	return res;
}

ssize_t Send(int sockfd, const void *buff, size_t nbytes, int flags)
{
	int res = send(sockfd, buff, nbytes, flags);
	if (res < 0) {
		perror("send error");
		exit(1);
	}
	return res;
}

void Close(int sockfd)
{
	int res = close(sockfd);
	if (res < 0) {
		perror("close error");
		exit(1);
	}
}

int Accept(int sockfd, struct sockaddr* cliaddr, socklen_t *clilen)
{
	int connfd = accept(sockfd, cliaddr, clilen);
	if (connfd < 0) {
		perror("accept error");
		exit(1);
	}
	return connfd;
}

void Listen(int sockfd, int backlog)
{
	int res = listen(sockfd, backlog);
	if (res < 0) {
		perror("listen error");
		exit(1);
	}
}

void Bind(int socket, struct sockaddr* servaddr, socklen_t addrlen)
{
	int res = bind(socket, servaddr, addrlen);
	if (res < 0) {
		perror("bind error");
		exit(1);
	}
}

int Socket(int family, int type, int protocol)
{
	int sockfd = socket(family, type, protocol);
	if (sockfd < 0) {
		perror("socket error");
		exit(1);
	}
	return sockfd;
}

int socket_create()
{
	int sockfd;
	struct sockaddr_in servaddr;

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	Bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

	Listen(sockfd, LISTENMAX);

	return sockfd;
}
