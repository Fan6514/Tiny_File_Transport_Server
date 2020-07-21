/************************************************************
 * File name : usr.c
 * author : fan
 * date : 2020-07-08
 * purpose : 文件传输系统的客户端用户操作程序
 * notice : 主要提供了用户发送指令与接收服务器响应的函数
 * ***********************************************************/
#include "usr.h"

void usr_operator(int socket)
{
	char sendline[MAXLINE], recvline[MAXLINE];

	while(1) {
		char *str = "Connect> ";
		char *p = readline(str);
		add_history(p);
		memset(sendline, 0, MAXLINE);
		strcpy(sendline, p);

		if (sendline[0] == 0 || sendline[0] == ' ')
			continue;
		else {		// 每次接收完数据都要清空缓冲区，以免与以后接收的数据混合
			if (strncmp(sendline, "close", 5) == 0) {
				send(socket, sendline, strlen(sendline), 0);
				memset(recvline, 0, MAXLINE);
				recv(socket, recvline, MAXLINE, 0);
				if (strncmp(recvline, "close", 5) == 0) {
					printf("The Server is Closed...\n");
					exit(0);
				}
			}
			else if (strncmp(sendline, "quit", 4) == 0 ||
				strncmp(sendline, "exit", 4) == 0)
			{
				printf("Client is closing...\n");
				exit(0);
			}
			else if (strncmp(sendline, "ls", 2) == 0) {
				send(socket, sendline, strlen(sendline), 0);
				memset(recvline, 0, MAXLINE);
				recv(socket, recvline, MAXLINE, 0);
				if (strncmp(recvline, "openerror", 9) == 0) {
					printf("Open Dir Fail.\n");
					break;
				}
				fputs(recvline, stdout);
				continue;
			}
			else if (strncmp(sendline, "put", 3) == 0) {
				if (strlen(sendline) == 3 || strlen(sendline) == 4)
					continue;
				putfile(socket, sendline + 4);
				memset(recvline, 0, MAXLINE);
				continue;
			}
			else if (strncmp(sendline, "help", 4) == 0) {
				
			}
			else {
				printf("client: command not found: %s\n", sendline);
				printf("you can input \"help\" to check.\n");
			}
		}
	}
}

/************************************************************
 * Function : 	putfile()
 * Descript : 	函数用于发送文件
 * Calls : 		
 * Called by :	usr_operator
 * notice : 	
 * ***********************************************************/
void putfile(int sockfd, char* filename)
{
	int fd;
	char buffer[MAXLINE] = { 0 };
	long long bytes, filesize;

	if (filename[0] == ' ')
		return;

	if ((fd = open(filename, O_RDONLY)) < 0)
	{
		if (errno == ENOENT)
		{
			printf("The file don't exist！\n");
			return;
		}
		else
		{
			printf("fail to open");
		}
	}

	/* 获取上传文件大小 */
	filesize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);	

	/* 向服务端发送指令和要获取的文件名 */
	sprintf(buffer, "put %s", filename);
	send(sockfd, buffer, strlen(buffer), 0);

	/* 接收服务器 begin 命令 */
	recv(sockfd, buffer, MAXLINE, 0);
	if (strncmp(buffer, "begin", 5) != 0) {
		printf("putfile exit!\n");
		return;
	}

	/* 发送要上传文件的大小 */
	sprintf(buffer, "%lld", filesize);
	send(sockfd, buffer, strlen(buffer), 0);

	/* 接收服务器 OK 命令 */
	recv(sockfd, buffer, MAXLINE, 0);
	if (strncmp(buffer, "ok", 2) == 0) {
		long long sum = 0;

		printf("开始传输文件\n");
		while ((bytes = read(fd, buffer, MAXLINE)) > 0)
		{
			send(sockfd, buffer, bytes, 0);
			sum += bytes;
			print_bar(filename, (float)sum, (float)filesize);
			if (sum == filesize) {
				printf("\n");
				break;
			}
		}
	}
	else if (strncmp(buffer, "no", 2) == 0) {
		printf("Transport error.\n");
		return;
	}

	sleep(1);

	strcpy(buffer, "##over##");
	send(sockfd, buffer, MAXLINE, 0);

	printf("%s : Upload Over\n", filename);
}

/************************************************************
 * Function : 	print_bar()
 * Descript : 	函数用于打印下载或上传文件的进度条
 * Calls : 		
 * Called by :	putfile、getfile
 * notice : 	
 * ***********************************************************/
void print_bar(const char *file_name, float sum, float file_size)
{
	float percent = (sum / file_size) * 100;
	char *sign = "#";
	if ((int)percent != 0){
		sign = (char *)malloc((int)percent + 1);
		strncpy(sign,"####################################################",(int) percent);
	}
	printf("%s %7.2f%% [%-*.*s] %.2f/%.2f kb\r",
		file_name,percent, 50, (int)percent / 2, sign,
		sum / 1024.0, file_size / 1024.0);
	if ((int)percent != 0)
		free(sign);
	fflush(stdout);
}
