#include "httpd.h"
#include <time.h>
#include <sys/epoll.h>

char *ok_msg = "HTTP/1.0 200 OK\r\n";
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


extern const char *log_path;
extern int logfd;

int set_blocking(int fd)
{
	int oldoption, newoption;
	oldoption = fcntl(fd, F_GETFL);
	newoption = oldoption & (~O_NONBLOCK);
	fcntl(fd, F_SETFL, newoption);
	return oldoption;
}


char* get_cur_time()
{
	time_t now_time;
	time(&now_time);
	return ctime(&now_time);
}
void printlog(char *info, int level)
{
	char *error_msg[8] = {"WARNING", "ERROR", "FATAL"};
	dprintf(logfd, "%s level:[%s] date:%s", info, error_msg[level], get_cur_time());
}
char *find_point(char *str)
{
	char *end = str;
	while(*end != '\0')
	{
		end++;
	}
	while(*end != '/' && *end != '.')
	{
		end--;
	}
	if(*end == '.')
		return end+1;
	else
		return NULL;
}
//deal error
static void echo_error()
{
	
}
int startup(char *ip, int port)
{
	assert(ip != NULL);
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		printlog("socket", FATAL);
		exit(2);
	}
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = inet_addr(ip);
	socklen_t len = sizeof(servaddr);
	int ret = bind(sockfd, (struct sockaddr*)&servaddr, len);
	if(ret < 0)
	{
		printlog("bind", FATAL);
		exit(3);
	}
	ret = listen(sockfd, 5);
	if(ret < 0)
	{
		printlog("listen", FATAL);
		exit(4);
	}
	return sockfd;
}
int get_line(int sockfd, char *buf, int len)
{

	char c = 1;
	int i = 0;
	set_blocking(sockfd);
	while(i < len-1 && c != '\n')
	{
		recv(sockfd, &c, 1, 0);
		if(c == '\r')
		{
			recv(sockfd, &c, 1, MSG_PEEK);
			if(c == '\n')
				recv(sockfd, &c, 1, 0);
			else
				c = '\n';
		}
		buf[i++] = c;
	}
	buf[i] = 0;
	set_non_blocking(sockfd);
	return i;
}
static void clear_head(int sockfd)
{
	int ret = 0;
	char buf[MAXSIZE];
	do{
		ret = get_line(sockfd, buf, MAXSIZE);
	}while(ret != 1);
}
static int echo_www(int sockfd, char *path, int size)
{
	pthread_mutex_lock(&mutex);
	int fd = open(path, O_RDONLY);
	if(fd < 0)
	{
		printlog("echo_www:open", FATAL);
		echo_error();
		return 7;
	}
	send(sockfd, ok_msg, strlen(ok_msg), 0);
	send(sockfd, "\r\n", strlen("\r\n"), 0);
	sendfile(sockfd, fd, NULL, size);
	close(fd);
	pthread_mutex_unlock(&mutex);
	return 0;
}
static int excute_cgi(int sockfd, char *path, char *method, char *arg_string)
{
	//printf("excute_cgi!!!\n");
	//int ret = 0;
	int content_length = -1;
	//clear head "GET" and "POST" get content_length
	if(strcasecmp(method, "GET") == 0)
	{
		clear_head(sockfd);
	}
	else
	{
		char buf[MAXSIZE];
		int s;
		do
		{
			s = get_line(sockfd, buf, sizeof(buf));
			if(strncasecmp(buf, "Content-Length: ", 16) == 0)
			{
				content_length = atoi(buf+16);
			}
		}while(s != 1);
	}
	
	//create pipe
	int input[2];
	int output[2];
	pipe(input);
	pipe(output);
	
	//fork and dup and putenv
	char method_env[1024];
	char arg_string_env[1024];
	char content_len_env[1024];
	pid_t pid = fork();
	if(pid < 0)
	{
		printlog("cgi: fork", FATAL);
		return 1;
	}
	else if(pid == 0)
	{
		close(input[1]);
		close(output[0]);
		dup2(input[0], 0);
		dup2(output[1], 1);
		
		sprintf(method_env, "METHOD=%s", method);
		putenv(method_env);
		if(strcasecmp(method, "GET") == 0)
		{
			sprintf(arg_string_env, "ARG_STRING=%s", arg_string);
			putenv(arg_string_env);
		}
		else
		{
			if(content_length == -1)
			{
				printlog("POST but no header Content-Length", ERROR);
				exit(1);
			}
			sprintf(content_len_env, "CONTENT_LENGTH=%d", content_length);
			putenv(content_len_env);
		}
		//support C,python,php
		char *suffix = find_point(path);
		if(suffix == NULL)
		{
			execl(path, path, NULL);
		}
		else if(strcasecmp(suffix, "py") == 0)
		{
			execlp("python", "python", path, NULL);
		}
		else if(strcasecmp(suffix, "php") == 0)
		{
			execlp("php", "php", path, NULL);
		}
		else		/*other script*/
		{
		}
		exit(2);
	}
	else
	{
		close(input[0]);
		close(output[1]);

		if(strcasecmp(method, "POST") == 0)
		{
			char c;
			int i;
			for(i = 0; i < content_length; i++)
			{
				recv(sockfd, &c, 1, 0);
				write(input[1], &c, 1);
			}
		}
		send(sockfd, ok_msg, strlen(ok_msg), 0);
		send(sockfd, "\r\n", strlen("\r\n"), 0);
		char ch;
		while(read(output[0], &ch, 1) != 0)
		{
			send(sockfd, &ch, 1, 0);
		}
		waitpid(pid, NULL, 0);
	}
	return 0;
}
void* accept_handler(int connfd)
{
	//printf("accept_handler!!!\n");
	char method[64];
	char uri[MAXSIZE];
	char buf[MAXSIZE];
	char *arg_string = NULL;
	char path[MAXSIZE];
	int ret = 0;
	int cgi = 0;
	int start = 0;
	int i;

	//get head line
	printlog("start to deal", FATAL);
	if(logfd == 0)
	{
		logfd = open(log_path, O_WRONLY | O_CREAT | O_TRUNC);
	}
	ret = get_line(connfd, buf, MAXSIZE);
	for(i = 0; i < sizeof(method) && start < strlen(buf)-1 && buf[start] != ' '; i++, start++)
	{
		method[i] = buf[start];
	}
	method[i] = '\0';
	if(strcasecmp(method, "GET") != 0 && strcasecmp(method, "POST") != 0)
	{
		ret = 5;
		printlog("method error[GET or POST]", WARNING);
		goto end;
	}
	printlog("here compare over", FATAL);
	if(strcasecmp(method, "POST") == 0)
	{
		cgi = 1;
	}
	while(buf[start] == ' ') start++;
	for(i = 0; i < sizeof(uri) && start < strlen(buf)-1 && buf[start] != ' '; i++, start++)
	{
		uri[i] = buf[start];
	}
	uri[i] = '\0';
	if(strcasecmp(method, "GET") == 0)
	{
		for(char *t = uri; *t != '\0'; t++)
		{
			if(*t == '?')
			{
				cgi = 1;
				*t = '\0';
				arg_string = ++t;
				break;
			}
		}
	}
	//
	printlog("uri anylyse over", FATAL);
	sprintf(path, "wwwroot%s", uri);
	struct stat st;
	if(stat(path, &st) == 0)
	{
		if(S_ISDIR(st.st_mode))
		{
			if(path[strlen(path)-1] == '/')
				strcat(path, "index.html");
			else
				strcat(path, "/index.html");
		}
		else
		{
			if((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH))
			{
				cgi = 1;
			}
		}
	}
	else
	{
		ret = 6;
		printlog("access file is not exist.", WARNING);
		goto end;
	}
	//
	printlog("judge cgi", FATAL);
	if(cgi == 0)
	{
		printlog("clear header", FATAL);
		set_blocking(connfd);
		clear_head(connfd);
		set_non_blocking(connfd);
		printlog("at echo_www", FATAL);
		ret = echo_www(connfd, path, st.st_size);
	}
	else
	{
		printlog("at excute_cgi", FATAL);
		ret = excute_cgi(connfd, path, method, arg_string);
	}
end:
	//close(connfd);
	return (void*)ret;
}
