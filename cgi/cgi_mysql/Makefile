main:main.cpp mysql_connect.cpp
	g++ -g -o $@ $^ -I../../mysql_c_connect/include -L../../mysql_c_connect/lib -lmysqlclient

.PHONY:clean
clean:
	rm -f main *.o
