/************************************************************
 * File name : serv.c
 * author : fan
 * date : 2020-07-07 
 * purpose : 文件传输系统主程序
 * notice : 使用 TCP 连接进行文件的传输
 * ***********************************************************/
#include "socket.h"
#include "thread.h"

int main()
{
	int  				listenfd, connfd;
	socklen_t 			clilen;
	struct sockaddr_in 	cliaddr;

	listenfd = socket_create();

	while (1) {
		clilen = sizeof(cliaddr);
		if ((connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &clilen)) < 0)
			continue;

		char conn_time[1024];
		getSysTime(conn_time);
		printf("Date: %s", conn_time);
		printf("New client connection: %d\n", connfd);
			
		if (thread_create(connfd) == ERROR)
			Close(connfd);
	}
}