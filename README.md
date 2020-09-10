# Tiny_File_Transport_Server

## 项目介绍

文件传输系统服务器采用了基于 TCP 的 Socket 编程，使用多线程来处理每个客户端对服务器的连接，通过自定义的通信协议解析客户端发送的命令，实现客户端从服务器上查看、上传和下载文件的基本功能。

该项目主要用来学习 Linux 网络编程和系统编程。

## 项目技术点

### TCP 服务器

服务器：socket -> bind -> listen -> accept -> recv -> send -> close

客户端：socket -> connect -> send -> recv -> close

```c
int socket(int family, int type, int protocol);
int connect(int sockfd, const struct sockaddr *servaddr, socklen_t addrlen);
int bind(int sockfd, const struct sockaddr *myaddr, socklen_t addrlen);
int listen(int sockfd, int backlog);
/* accept 返回新的 socket 文件描述符 */
int accept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen);
int close(int sockfd);
```

### 文件操作

查看文件列表、上传和下载文件都使用了文件操作，主要包括打开文件目录、读取目录、打开文件、读写文件

目录操作相关：

```c
// 打开目录
DIR *opendir(const char *pathname);
// 读取目录信息
struct dirent *readdir(DIR *dp);
// dirent 结构至少包括以下两种信息
ino_t d_ino;			// i-node number
char d_name[256]	// filename
```

文件操作相关：

```c
// 打开文件
int open(const char *path, int pflag, mode_t mode);
// 创建一个新文件并打开
int fd = open(filepath, O_CREAT | O_WRONLY | O_TRUNC, 0666);
// 读文件，成功返回读到的字节数，若到文件尾返回 0
ssize_t read(int fd, void *buf, size_t nbytes);
// 写文件，成功返回已写的字节数
ssize_t write(int fd, const void *buf, size_t nbytes);
```

### 多线程

线程是操作系统能运算调度的最小单位，一条线成是进程中一个单一顺序的控制流，一个进程中可以并发多个线程，每条线程并行执行不同的任务。

**线程与进程的区别：**

* 进程是资源分配的最小单位，线程是程序执行的最小单位
* 进程有自己的独立地址空闲，而线程共享进程的数据
* 同一进程下的线程共享全局变量和静态变量等数据，而进程间通信需要依靠 IPC 方式进行（管道、信号、消息队列、共享内存和套接字等）
* 多进程程序更健壮，多线程程序只要一个线程死掉，整个进程也死掉；而一个进程死掉不会影响另一个进程

```c
// 创建线程
int phtread_create(pthread_pid *tid, const pthread_attr_t *attr,
                  void *(*start_routine)(void *), void *arg);
// 线程分离
int pthread_detach(pthread_t tid);
```

线程分离状态：线程主动与主控线程断开关系，线程结束后，其退户状态不由其他线程获取，而是自己直接释放。

### I/O 复用

在文件传输的过程使用 poll 函数监听 POLLHUP 事件，如果客户端在文件传输过程中断开连接，此时捕获 POLLHUP 后关闭该连接的套接字。

I/O 复用可监视多个文件描述符（套接字），一旦某个文件描述符就绪，就通知程序进行相应读写操作。

1. select 模型

内核件事所有 select 负责的 socket，任何一个 socket 数据准备好后，select 就会返回，此时用户进程调用 read 将数据从内核拷贝到用户进程

缺点：单进程能监视的文件描述符有上限，Linux 下一般为 1024 个；每次调用 select 要把 fd 集合从用户态拷贝到内核态，开销较大；轮询的方式效率较低

2. poll 模型

与 select 类似，轮询一定数量的文件描述符，但没有监听的最大数量限制

3. epoll 模型

Linux 内核 2.6 版本引入的高效 epoll 模型，使用一个 epollfd 管理多个文件描述符，采用监听回调机制，避免多次遍历就绪的文件描述符列表。

两种模式：LT 和 ET。

LT 模式：只要 fd 还有数据可读，每次 epoll_wait 都会返回它的事件，提醒用户程序操作

ET 模式：只会提醒一次，直到下次再有数据流入前都不会再提示，无论 fd 中是否还有数据可读。因此 ET 模式下 read 一个 fd 时一定要读完 buffer 或遇到 EAGAIN 错误

## 项目扩展

1. 文件传输完整性

可以采用 MD5 摘要算法，例如上传文件，首先客户端计算文件的 MD5 值传送给服务端，然后上传文件。文件传输完成后计算传输后文件的 MD5 值，与之前收到的对比来验证文件传输的完整性。

2. 断点传输

对于下载时的断点传输，直接将客户端本地已经下载文件的大小传输给服务器，服务器端直接偏移相应大小来打开文件，开始传输数据；而客户端收到数据后从文件的末尾开始写。上传时与下载的续传功能相似。
