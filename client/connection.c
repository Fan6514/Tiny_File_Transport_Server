/************************************************************
 * File name : serv_connection.c
 * author : fan
 * date : 2020-07-08
 * purpose : 客户端连接服务端程序
 * notice : 
 * ***********************************************************/
#include "connection.h"

void Connect(int socket, struct sockaddr* servaddr, socklen_t addrlen)
{
	int res = connect(socket, servaddr, addrlen);
	if (res < 0) {
		perror("connect error");
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

/************************************************************************************************   
 * Function:       serv_conncetion()
 * Description:    本函数为客户端连接服务器程序，负责创建套接字、连接服务器
 * Calls:          
 * Called By:      cli.c - main()
 * Other    :      程序首先建立与客端的连接，然后获取到客户端发送的连接套接字值
************************************************************************************************/

int serv_conncetion(const char *ipv4)
{
	int 				sockfd;
	struct sockaddr_in 	servaddr;

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	if (inet_pton(AF_INET, ipv4, &servaddr.sin_addr) < 0) {
		perror("server address error");
		exit(1);
	}

	Connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));

	return sockfd;
}