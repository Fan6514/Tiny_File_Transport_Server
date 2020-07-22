#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

#define SUCESS 0
#define ERROR -1
#define MAXLINE 4096	// 缓存大小
#define CMDLINE 128 	// 命令行和文件名大小
#define ARGC 10			// 命令参数个数
#define NEW_FILE 1
#define OLD_FILE 2

int thread_create(int connfd);
void *thread_work(void* arg);
void getSysTime(char * buf);
void send_filelist(int sockfd);
void putfile(int connfd, char* filename);
void getfile(int connfd, char* filename);

