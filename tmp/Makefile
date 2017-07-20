mysql_connect:mysql_connect.c
	gcc -o $@ $^ -I../mysql_c_connect/include -L../mysql_c_connect/lib -lmysqlclient

.PHONY:clean
clean:
	rm -f mysql_connect
