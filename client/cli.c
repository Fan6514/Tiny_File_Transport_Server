/************************************************************
 * File name : cli.c
 * author : fan
 * date : 2020-07-08
 * purpose : 文件传输系统的客户端程序
 * notice : 启动时需要输入服务器的 ip 地址
 * ***********************************************************/
#include "usr.h"
#include "connection.h"

int main(int argc, char *argv[])
{
	int sockfd;

	if (argc != 2) {
		perror("usage: ./cli <Server IP Address>");
		exit(1);
	}

	sockfd = serv_conncetion(argv[1]); 	// 连接服务器
	usr_operator(sockfd);				// 用户操作界面

	return 0;
}