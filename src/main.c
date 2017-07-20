#include "httpd.h"

//void* accept_request(void *arg)
//{
//	int connfd = (int)arg;
//	pthread_detach(pthread_self());
//	return accept_handler(connfd);
//}
int turn = 1;
int send_data = 1;
const char *log_path = "log/httpd.log";
int logfd = 0;

int set_non_blocking(int fd)
{
	int oldoption, newoption;
	oldoption = fcntl(fd, F_GETFL);
	newoption = oldoption | O_NONBLOCK;
	fcntl(fd, F_SETFL, newoption);
	return oldoption;
}
void addfd(int epollfd, int fd)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	set_non_blocking(fd);
}
void delfd(int epollfd, int fd)
{
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
}
void worker_do(int listenfd, struct ConnPipe *conn_pipe, int index)
{
	close(conn_pipe[index]._pipe[1]);
	//printf("%d\n", conn_pipe[index]._pipe[0]);
	int anoy_pipe = conn_pipe[index]._pipe[0];
	int epollfd = epoll_create(256);
	struct epoll_event revent[ARRAYSIZE];
	addfd(epollfd, anoy_pipe);
	//addfd(epollfd, listenfd);

	while(1)
	{
		int ret = epoll_wait(epollfd, revent, ARRAYSIZE, -1);
		if(ret < 0)
		{
			//
			continue ;
		}
		//printf("worker epill_wait over!!!\n");
		for(int i = 0; i < ret; i++)
		{
			int sockfd = revent[i].data.fd;
			if(sockfd == anoy_pipe)
			{
				//printf("worker accpet!!!\n");
				printlog("worker start to accept", FATAL);
				while(1)
				{
					char buf[128];
					recv(sockfd, buf, sizeof(buf), 0);
					struct sockaddr_in clientAddr;
					socklen_t len = sizeof(clientAddr);
					int connfd = accept(listenfd, (struct sockaddr*)&clientAddr, &len);
					if(connfd < 0)
					{
						printlog("worker accept error", FATAL);
						break;
					}
					addfd(epollfd, connfd);
					dprintf(logfd, "come a newer : ip<%s> port<%d> sockfd<%d> date:%s", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), connfd, get_cur_time());
				}
				printlog("worker accept over", FATAL);
			}
			//
			else
			{
				//char *msg = "received\n";
				//send(sockfd, msg, strlen(msg), 0);
				//printf("handler sockfd!!!\n");
				accept_handler(sockfd);
				printlog("deal over", FATAL);
				delfd(epollfd, sockfd);
				close(sockfd);
				dprintf(logfd, "sockfd<%d> quit date:%s", sockfd, get_cur_time());
			}
		}
	}
}
void master_do(int listenfd, struct ConnPipe *conn_pipe, int worker_nums)
{
	int epollfd = epoll_create(256);
	struct epoll_event revent[ARRAYSIZE];
	addfd(epollfd, listenfd);
	

	while(1)
	{
		int ret = epoll_wait(epollfd, revent, ARRAYSIZE, -1);
		if(ret < 0)
		{
			//printf("WARNING: epoll_wait\n");
			continue ;
		}
		for(int i = 0 ; i < ret; i++)
		{
			int sockfd = revent[i].data.fd;
			if(sockfd == listenfd)
			{
				//printf("master listen!!!\n");
				int tmp = turn;
				do
				{
					if(conn_pipe[turn]._pid != -1)
						break;
					turn++;
					if(turn >= worker_nums+1)
					{
						turn %= worker_nums;
					}
				}while(turn != tmp);
				if(conn_pipe[turn]._pid == -1)
				{
					dprintf(logfd, "no workers!\n");
					continue;
				}
				send(conn_pipe[turn++]._pipe[1], (char*)&send_data, sizeof(send_data), 0);
				//printf("master send over!!!\n");
				if(turn >= worker_nums+1)
				{
					turn %= worker_nums;
				}
				char bf[128];
				sprintf(bf, "turn's value is %d", turn);
				printlog(bf, FATAL);
			}
			else
			{
				//
			}
		}
	}
}

int main(int argc, char *argv[])
{
	if(argc != 4)
	{
		dprintf(logfd, "usage: %s [local_ip] [local_port] [cpu_cores]\n", basename(argv[0]));
		return 1;
	}
	if(logfd == 0)
	{
		logfd = open(log_path, O_WRONLY | O_CREAT | O_TRUNC);
	}
	int listenfd = startup(argv[1], atoi(argv[2]));
	struct sockaddr_in clientaddr;
	socklen_t len;
	
	int worker_nums = atoi(argv[3]);
	if(worker_nums <= 0)
	{
		return 7;
	}
	struct ConnPipe *conn_pipe = (struct ConnPipe*)malloc(sizeof(struct ConnPipe)*(worker_nums+1));
	for(int i = 1; i <= worker_nums; i++)
	{
		socketpair(AF_UNIX, SOCK_STREAM, 0, conn_pipe[i]._pipe);
		conn_pipe[i]._pid = -1;
	}
	for(int i = 1; i <= worker_nums; i++)
	{
		pid_t pid = fork();
		if(pid < 0)
		{
			return 8;
		}
		else if(pid == 0)
		{
			worker_do(listenfd, conn_pipe, i);
			exit(0);
		}
		close(conn_pipe[i]._pipe[0]);
		conn_pipe[i]._pid = pid;
	}
	daemon(1, 0);
	master_do(listenfd, conn_pipe, worker_nums);

	free(conn_pipe);
	close(listenfd);
	close(logfd);
	return 0;
	//daemon(1, 0);	//create daemon
	//int count = 0;
	//while(1)
	//{
	//	len = sizeof(clientaddr);
	//	int connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &len);
	//	if(connfd < 0)
	//	{
	//		printlog("accept", FATAL);
	//		continue;
	//	}
	//	pthread_t tid;
	//	int ret = pthread_create(&tid, NULL, accept_request, (void*)connfd);
	//	if(ret != 0)
	//	{
	//		printlog("pthread_create", WARNING);
	//		char *buf = "server is too busy!";
	//		send(connfd, buf, strlen(buf), 0);
	//		close(connfd);
	//		printf("%d\n", count);
	//		continue ;
	//	}
	//	count++;
	//	//pthread_detach(tid);
	//}
}
