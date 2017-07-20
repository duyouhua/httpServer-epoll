httpd:/home/ubnt/myprogram/TinyServer/src/httpd.o /home/ubnt/myprogram/TinyServer/src/main.o 
	gcc -o $@ $^ -lpthread;
%.o:%.c
	gcc -c $<

.PHONY:clean
clean:
	rm -f httpd *.o
