/************************************************************
 * File name : thread.c
 * author : fan
 * date : 2020-07-07 
 * purpose : 服务器文件接收和传输的核心，主要提供文件的接收和传输功能
 * notice : 
 * ***********************************************************/
#include "thread.h"
#include "socket.h"

/************************************************************
 * Function : 	thread_create()
 * Descript : 	创建一个用户线程，提供多用户并发访问服务器
 * Calls : 		thread_work()
 * Called by :	ser.c - main()
 * notice : 	创建一个新线程，成功则返回 SUCESS，线程从 thread_work 
 				函数开始执行
 				失败返回 ERROR，SUCESS 和 ERROR 定义在 thread.h 中
 * ***********************************************************/
int thread_create(int connfd)
{
	pthread_t tid;
	int ret;

	ret = pthread_create(&tid, NULL, thread_work, (void*)connfd);
	if (ret != 0)
		return ERROR;
	return SUCESS;
}

/************************************************************
 * Function : 	thread_work()
 * Descript : 	实现对客户端命令解析
 * Calls : 		send_filelist()、putfile()
 * Called by :	thread_create()
 * notice : 	
 * ***********************************************************/
void *thread_work(void* arg)
{
	int connfd = (int)arg;
	char recvline[MAXLINE], sendline[MAXLINE];
	int i;

	while (1) {		// 对客户端发送的命令进行解析
		memset(recvline, 0, MAXLINE);
		int n = recv(connfd, recvline, MAXLINE, 0);

		if (strncmp(recvline, "close", 5) == 0 ) {
			send(connfd, "close", 5, 0);
			printf("The server is closing...\n");
			exit(0);
		}
		if (n <= 0)	// recv 返回 0 表示通信双方关闭连接
			break;

		int i = 0, comm_argc = 0;
		char *ptr = NULL;
		char *comm_argv[ARGC] = { 0 };
		char *argv_tmp = strtok_r(recvline, " ", &ptr);
		while (argv_tmp != NULL) {		// 获取命令的每个参数
			comm_argv[i++] = argv_tmp;
			argv_tmp = strtok_r(NULL, " ", &ptr);
		}
		comm_argc = i;

		char *cmd = comm_argv[0];	// 命令
		if (strncmp(cmd, "ls", 2) == 0) {
			send_filelist(connfd);
			continue;
		}
		else if (strncmp(cmd, "put", 3) == 0) {
			putfile(connfd, comm_argv[1]);
			continue;
		}
	}
	return NULL;
}

/************************************************************
 * Function : 	getSysTime()
 * Descript : 	获取当前的系统时间
 * Calls : 
 * Called by :	thread_create()、serv.c - main()
 * notice : 	
 * ***********************************************************/
void getSysTime(char * buf)
{
	time_t tm;
	time(&tm);
	ctime_r(&tm, buf);
	return;
}

/************************************************************
 * Function : 	send_filelist()
 * Descript : 	向套接字中写入指定目录下的文件名，并发送
 * Calls : 
 * Called by :	thread_work()
 * notice : 	
 * ***********************************************************/
void send_filelist(int sockfd)
{
	DIR 			*dp;
	struct dirent 	*dirp;
	char file_name[4096] = {0};

	if ((dp = opendir("./file")) == NULL){
		printf("opendir error");
		send(sockfd, "openerror", 9, 0);
		return;
	}
	while ((dirp = readdir(dp)) != NULL) {
		if (strcmp(dirp->d_name, ".") == 0 || 
			strcmp(dirp->d_name, "..") == 0)
			continue;
		strcat(file_name, dirp->d_name);
		strcat(file_name, " ");
		// fputs(file_name, stdout);
	}
	strcat(file_name, "\n");
	send(sockfd, file_name, strlen(file_name), 0);

	closedir(dp);
	return;
}

/************************************************************
 * Function : 	putfile()
 * Descript : 	接收客户端上传的文件
 * Calls : 
 * Called by :	thread_work()
 * notice : 	
 * ***********************************************************/
char buffer[MAXLINE] = { 0 };
char filename[MAXLINE] = { 0 };

void putfile(int connfd, char* file_name)
{
	int fd;
	int flag = 0;
	long long bytes, file_size, newfile_size;

	strcpy(filename, "./file/");
	strcat(filename, file_name);

	strcpy(buffer, "begin");
	printf("File %s from client ready to receive.\n", file_name);
	send(connfd, buffer, MAXLINE, 0);

	/* 接收文件的大小 */
	recv(connfd, buffer, MAXLINE, 0);
	printf("File Size: %s\n", buffer);
	file_size = atoll(buffer);

	/* 创建新文件以接收 */
	if ((fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0666)) < 0) {
		printf("[%s] create error", file_name);
		strcpy(buffer, "no");
		send(connfd, buffer, MAXLINE, 0);
		return;
	}

	struct pollfd fds[1];
	fds[0].fd = connfd;
	fds[0].events = POLLHUP;
	fds[0].revents = 0;

	strcpy(buffer, "ok");
	send(connfd, buffer, MAXLINE, 0);
	printf("正在下载%s,请稍候\n", file_name);
	char file_cache[MAXLINE] = { 0 };
	
	while ((bytes = recv(connfd, file_cache, MAXLINE, 0)) > 0)
	{
		int n = poll(fds, 1, 0);
		if (n < 0) {
			perror("poll fail\n");
			exit(0);
		}
		if (fds[0].revents & POLLHUP) {
			printf("客户端异常中断\n");
			close(connfd);
			return;
		}
		if (strncmp(file_cache, "##over##", 8) == 0)
			break;

		write(fd, file_cache, bytes);
	}
	close(fd);
	struct stat st;
	stat(filename, &st);
	float filesize = (float)st.st_size / 1024;
	printf("%s下载完毕，大小为%.2fkb\n", file_name, filesize);
}

