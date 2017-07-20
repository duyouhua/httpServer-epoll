#ifndef _MYSQL_CONNECT_
#define _MYSQL_CONNECT_

#include <stdio.h>
#include <mysql.h>
#include <my_global.h>
#include <string.h>

#define MAX_COLUMN_LEN 32

class mysql_api
{
private:
	MYSQL _mysql_conn;

public:
	mysql_api();
	~mysql_api();
	int connect();
	int dis_connect();
	int insert_act_student(const char *name, const int age, const char *school);
	int delete_act_student(int cid);
	int update_act_student(const char *name, const int age, const char *school, int cid);
	void select_act_student();
};


#endif
