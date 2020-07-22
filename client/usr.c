/************************************************************
 * File name : usr.c
 * author : fan
 * date : 2020-07-08
 * purpose : 文件传输系统的客户端用户操作程序
 * notice : 主要提供了用户发送指令与接收服务器响应的函数
 * ***********************************************************/
#include "usr.h"

void usr_operator(int sockfd)
{
	char sendline[CMDLINE], recvline[CMDLINE];

	while(1) {
		char *str = "Connect> ";
		char *p = readline(str);
		add_history(p);
		memset(sendline, 0, CMDLINE);
		strcpy(sendline, p);

		if (sendline[0] == 0 || sendline[0] == ' ')
			continue;
		else {		// 每次接收完数据都要清空缓冲区，以免与以后接收的数据混合
			if (strncmp(sendline, "close", 5) == 0) {
				send(sockfd, sendline, CMDLINE, 0);
				recv(sockfd, recvline, CMDLINE, 0);
				if (strncmp(recvline, "close", 5) == 0) {
					printf("The Server is Closed...\n");
					exit(0);
				}
				else {
					printf("The Server Closed Failed.\n");
				}
			}
			else if (strncmp(sendline, "quit", 4) == 0 ||
				strncmp(sendline, "exit", 4) == 0)
			{
				printf("Client is closing...\n");
				strcpy(sendline, "quit");
				send(sockfd, sendline, CMDLINE, 0);
				sleep(3);
				exit(0);
			}
			else if (strncmp(sendline, "ls", 2) == 0) {
				send(sockfd, sendline, CMDLINE, 0);
				recv(sockfd, recvline, CMDLINE, 0);
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
				putfile(sockfd, sendline + 4);
				memset(recvline, 0, CMDLINE);
				continue;
			}
			else if (strncmp(sendline, "get", 3) == 0) {
				if (strlen(sendline) == 3 || strlen(sendline) == 4)
					continue;
				getfile(sockfd, sendline + 4);
			}
			else if (strncmp(sendline, "help", 4) == 0) {
				help_info();
				continue;
			}
			else {
				printf("client: command not found: %s\n", sendline);
				printf("you can input \"help\" to check.\n");
				continue;
			}
		}
	}
}

/************************************************************
 * Function : 	putfile()
 * Descript : 	函数用于上传文件到服务器
 * Calls : 		
 * Called by :	usr_operator
 * notice : 	
 * ***********************************************************/
static char file_cache[MAXLINE] = { 0 }; 	// 文件发送与接收缓存
void putfile(int sockfd, char* filename)
{
	int fd;
	char buffer[CMDLINE] = { 0 };
	char file_path[CMDLINE] = { 0 };	// 文件本地路径
	long long bytes, filesize;

	/* 添加文件地址 */
	if (filename[0] == ' ')
		return;
	if (filename[0] != '.') {
		strcpy(file_path, "./");
		strcat(file_path, filename);
	}

	if ((fd = open(file_path, O_RDONLY)) < 0)
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

	/* 向服务端发送指令和要上传文件的文件名 */
	sprintf(buffer, "put %s", filename);
	send(sockfd, buffer, CMDLINE, 0);

	/* 接收服务器 begin 命令 */
	recv(sockfd, buffer, CMDLINE, 0);
	if (strncmp(buffer, "begin", 5) != 0) {
		printf("putfile exit!\n");
		return;
	}

	/* 发送要上传文件的大小 */
	sprintf(buffer, "%lld", filesize);
	send(sockfd, buffer, CMDLINE, 0);

	/* 接收服务器 OK 命令 */
	recv(sockfd, buffer, CMDLINE, 0);
	if (strncmp(buffer, "no", 2) == 0) {
		printf("Transport error.\n");
		return;
	}
	else if (strncmp(buffer, "ok", 2) == 0) {
		long long sum = 0;

		printf("开始传输文件\n");
		while ((bytes = read(fd, file_cache, MAXLINE)) > 0)
		{
			send(sockfd, file_cache, bytes, 0);
			sum += bytes;
			print_bar(filename, (float)sum, (float)filesize);
			if (sum == filesize) {
				printf("\n");
				break;
			}
		}
	}

	sleep(1);

	strcpy(buffer, "##over##");
	send(sockfd, buffer, CMDLINE, 0);

	printf("%s : Upload Over\n", filename);
	close(fd);
}

/************************************************************
 * Function : 	getfile()
 * Descript : 	函数用于接收服务器发送的文件
 * Calls : 		
 * Called by :	usr_operator()
 * notice : 	
 * ***********************************************************/
void getfile(int sockfd, char* filename)
{
	int fd;
	long long bytes, file_size, newfile_size;
	char buffer[CMDLINE] = { 0 };
	char file_name[CMDLINE] = { 0 };

	/* 向服务端发送指令和要下载文件的文件名 */
	sprintf(buffer, "get %s", filename);
	send(sockfd, buffer, CMDLINE, 0);

	recv(sockfd, buffer, CMDLINE, 0);
	if (strncmp(buffer, "openerror", 9) == 0) {
		printf("server file error\n");
		return;
	}
	else if (strncmp(buffer, "nonexist", 8) == 0) {
		printf("File: %s is not found in your server.\n", filename);
		return;
	}
	else if (strncmp(buffer, "begin", 5) == 0)
		printf("等待服务器发送文件...\n");

	strcpy(file_name, "./");
	strcat(file_name, filename);

	/* 接收文件大小 */
	recv(sockfd, buffer, CMDLINE, 0);
	printf("File Size: %s\n", buffer);
	file_size = atoll(buffer);

	/* 创建新文件接收文件 */
	if ((fd = open(file_name, O_CREAT | O_WRONLY | O_TRUNC, 0666)) < 0) {
		printf("[%s] create error", filename);
		strcpy(buffer, "no");
		send(sockfd, buffer, CMDLINE, 0);
		return;
	}

	struct pollfd fds[1];
	fds[0].fd = sockfd;
	fds[0].events = POLLHUP;
	fds[0].revents = 0;

	strcpy(buffer, "ok");
	send(sockfd, buffer, CMDLINE, 0);
	printf("正在下载%s,请稍候\n", filename);
	long long sum = 0;
	
	while ((bytes = recv(sockfd, file_cache, MAXLINE, 0)) > 0)
	{
		int n = poll(fds, 1, 0);
		if (n < 0) {
			perror("poll fail\n");
			exit(0);
		}
		if (fds[0].revents & POLLHUP) {
			printf("服务器异常中断\n");
			close(sockfd);
			return;
		}
		if (strncmp(file_cache, "##over##", 8) == 0)
			break;
		
		write(fd, file_cache, bytes);
		sum += bytes;
		print_bar(filename, (float)sum, (float)file_size);
	}
	close(fd);
	struct stat st;
	stat(file_name, &st);
	float filesize = (float)st.st_size / 1024;
	printf("\n%s下载完毕，大小为%.2fkb\n", filename, filesize);
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

void help_info()
{
    FILE * fp = fopen("./help_info", "r");
    if (fp == NULL) {
    	perror("fopen error");
    	exit(1);
    }
    
    char buffer[CMDLINE] = { 0 };
    while(fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        printf("%s", buffer);
    }
    printf("\n");
    fclose(fp);
}
