#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFSIZE 1024
#define DATANUM 2

int main()
{
	char method[BUFSIZE];
	char content_data[BUFSIZE];
	int content_length = 0;
	char *arr[DATANUM];

	if(getenv("METHOD"))
	{
		strcpy(method, getenv("METHOD"));
		if(strcasecmp(method, "GET") == 0)
		{
			if(getenv("ARG_STRING"))
			{
				strcpy(content_data, getenv("ARG_STRING"));
			}
			else
			{
				printf("GET:ARG_STRING is not exist!");
				return 2;
			}
		}
		else
		{
			if(getenv("CONTENT_LENGTH"))
			{
				content_length = atoi(getenv("CONTENT_LENGTH"));
			}
			else
			{
				printf("POST:CONTENT_LENGTH is not exist!");
				return 3;
			}
		}
	}
	else
	{
		printf("METHOD is not exist!");
		return 1;
	}

	if(strcasecmp(method, "POST") == 0)
	{
		char c;
		int i;
		for(i = 0; i < content_length; i++)
		{
			read(0, &c, 1);
			content_data[i] = c;
		}
		content_data[i] = 0;
	}
	int j = 0;
	for(char *start = content_data; *start != '\0'; start++)
	{
		if(*start == '=')
		{
			if(j < DATANUM)
				arr[j++] = start+1;
		}
		else if(*start == '&')
		{
			*start = '\0';
		}
	}
	int data1 = atoi(arr[0]);
	int data2 = atoi(arr[1]);
	printf("<html><head></head>");
	printf("<body>");
	printf("<h2>%d + %d = %d<h2><br/>", data1, data2, data1+data2);
	printf("<h2>%d - %d = %d<h2><br/>", data1, data2, data1-data2);
	printf("<h2>%d * %d = %d<h2><br/>", data1, data2, data1*data2);
	printf("<h2>%d / %d = %d<h2><br/>", data1, data2, data1/data2);
	printf("</body>");
	printf("</html>");
	return 0;
}
