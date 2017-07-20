#ifndef __HTTPD_H__
#define __HTTPD_H__

#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>

#define MAXSIZE 4096
#define ARRAYSIZE 1024

#define WARNING 0
#define ERROR 1
#define FATAL 2


struct ConnPipe
{
	int _pipe[2];
	int _pid;
	//struct ConnPipe()
	//{
	//	pipe(_pipe);
	//	_pid = -1;
	//}
};

void printlog(char *info, int level);
int startup(char *ip, int port);
void* accept_handler(int connfd);

////////////////////////////////////////////
void worker_do();
void master_do();
#endif
