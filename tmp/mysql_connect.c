#include <stdio.h>
#include <mysql.h>

int main()
{
	printf("Mysql client version: %s\n", mysql_get_client_info());
	return 0;
}
