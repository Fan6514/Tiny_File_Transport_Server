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
	pthread_detach(tid);
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
	char recvline[CMDLINE], sendline[CMDLINE];
	int i;

	while (1) {		// 对客户端发送的命令进行解析
		memset(recvline, 0, CMDLINE);
		int n = recv(connfd, recvline, CMDLINE, 0);

		if (strncmp(recvline, "close", 5) == 0 ) {
			send(connfd, "close", 5, 0);
			printf("The server is closing...\n");
			exit(0);
		}
		else if (strncmp(recvline, "quit", 4) == 0) {
			break;
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
		else if (strncmp(cmd, "get", 3) == 0) {
			getfile(connfd, comm_argv[1]);
		}
	}
	printf("Client is closed.\n");
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
	char file_name[CMDLINE] = {0};

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
	send(sockfd, file_name, CMDLINE, 0);

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
static char file_cache[MAXLINE] = { 0 };		// 文件接收与发送缓存
void putfile(int connfd, char* filename)
{
	int fd;
	long long bytes, file_size;
	char buffer[CMDLINE] = { 0 };
	char file_path[CMDLINE] = { 0 }; 		// 文件本地路径

	strcpy(file_path, "./file/");
	strcat(file_path, filename);

	/* 向客户端发送 begin 指令 */
	strcpy(buffer, "begin");
	printf("File %s from client ready to receive.\n", filename);
	send(connfd, buffer, strlen(buffer), 0);
	memset(buffer, 0, CMDLINE);

	/*  接收客户端发送文件的大小 */
	recv(connfd, buffer, CMDLINE, 0);
	printf("File Size: %s\n", buffer);
	file_size = atoll(buffer);

	/* 创建新文件以接收 */
	if ((fd = open(file_path, O_CREAT | O_WRONLY | O_TRUNC, 0666)) < 0) {
		printf("[%s] create error", filename);
		strcpy(buffer, "no");
		send(connfd, buffer, strlen(buffer), 0);
		return;
	}

	struct pollfd fds[1];
	fds[0].fd = connfd;
	fds[0].events = POLLHUP;
	fds[0].revents = 0;

	strcpy(buffer, "ok");
	send(connfd, buffer, strlen(buffer), 0);
	printf("正在下载%s,请稍候\n", filename);
	
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
	printf("%s下载完毕，大小为%.2fkb\n", filename, filesize);
}

/************************************************************
 * Function : 	getfile()
 * Descript : 	发送客户端下载的文件
 * Calls : 
 * Called by :	thread_work()
 * notice : 	
 * ***********************************************************/
void getfile(int connfd, char* filename)
{
	int fd;
	long long bytes, file_size, newfile_size;
	char buffer[CMDLINE] = { 0 };
	char file_name[CMDLINE] = { 0 };
	
	if (filename[0] == ' ')
		return;
	if (filename[0] != '.') {
		strcpy(file_name, "./file/");
		strcat(file_name, filename);
	}
	if ((fd = open(file_name, O_RDONLY)) < 0)
	{
		if (errno == ENOENT)
		{
			strcpy(buffer, "nonexist");
			send(connfd, buffer, CMDLINE, 0);
		}
		else
		{
			strcpy(buffer, "openerror");
			send(connfd, buffer, CMDLINE, 0);
		}
		return;
	}
	strcpy(buffer, "begin");
	send(connfd, buffer, CMDLINE, 0);

	 /* 发送下载文件大小 */
	file_size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	sprintf(buffer, "%lld", file_size);
	printf("传输文件大小 %s\n", buffer);
	send(connfd, buffer, CMDLINE, 0);

	/* 接收服务器 OK 命令 */
	recv(connfd, buffer, CMDLINE, 0);
	if (strncmp(buffer, "no", 2) == 0) {
		printf("Transport error.\n");
		return;
	}
	else if (strncmp(buffer, "ok", 2) == 0) {
		long long sum = 0;

		printf("开始传输文件...\n");
		while ((bytes = read(fd, file_cache, MAXLINE)) > 0)
		{
			send(connfd, file_cache, bytes, 0);
			sum += bytes;
			if (sum == file_size) {
				break;
			}
		}
	}

	sleep(1);

	strcpy(buffer, "##over##");
	send(connfd, buffer, CMDLINE, 0);

	printf("%s : Download Over\n", filename);
}

