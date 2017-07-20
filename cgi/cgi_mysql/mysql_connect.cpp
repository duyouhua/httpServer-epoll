#include "mysql_connect.h"

mysql_api::mysql_api()
{
	mysql_init(&_mysql_conn);
}
mysql_api::~mysql_api()
{
}
int mysql_api::connect()
{
	const char *host="127.0.0.1";
	const char *user="root";
	const char *passwd="1";
	const char *db="mytest";
	unsigned int port=3306;
	const char *unix_socket=NULL;
	unsigned long clientflag=0;
	if(mysql_real_connect(&_mysql_conn, host, user, passwd, db, port, unix_socket, clientflag) == NULL)
	{
		return 0;
	}
	return 1;
}
int mysql_api::dis_connect()
{
	mysql_close(&_mysql_conn);
}
int mysql_api::insert_act_student(const char *name, const int age, const char *school)
{
	char cmd[1024];
	sprintf(cmd, "insert into student (name, age, school, register_time) values ('%s', %d, '%s', now());", name, age, school);
	return mysql_query(&_mysql_conn, cmd);
}
int  mysql_api::delete_act_student(int cid)
{
	char cmd[1024];
	sprintf(cmd, "delete from student where id=%d;", cid);
	return mysql_query(&_mysql_conn, cmd);
}
int mysql_api::update_act_student(const char *name, const int age, const char *school, int cid)
{
	char cmd[1024];
	sprintf(cmd, "update student set name='%s', age=%d, school='%s' where id=%d;", name, age, school, cid);
	return mysql_query(&_mysql_conn, cmd);
}
void mysql_api::select_act_student()
{
	MYSQL_RES *res;
	MYSQL_ROW sql_row;
	MYSQL_FIELD *field;
	int cols;
	int i;
	char column[MAX_COLUMN_LEN][MAX_COLUMN_LEN];
	char cmd[1024] = "select * from student;";
	mysql_query(&_mysql_conn, cmd);
	res = mysql_store_result(&_mysql_conn);
	cols = mysql_num_fields(res);
	for(i =0;field = mysql_fetch_field(res); i++)
	{
		bzero(column[i], sizeof(column[i]));
		strcpy(column[i], field->name);
	}
	cols = i;
	for(i = 0; i < cols; i++)
	{
		printf("%s\t", column[i]);
	}
	printf("\n");
	while(sql_row = mysql_fetch_row(res))
	{
		for(i = 0; i < cols; i++)
		{
			printf("%s\t", sql_row[i]);
		}
		printf("\n");
	}
}
