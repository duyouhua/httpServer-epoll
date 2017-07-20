.PHONY:all
all:
	cd src;make;cd ..;
	cd cgi;make;cd ..;

.PHONY:clean
clean:
	cd src;make clean;cd ..;
	cd cgi;make clean;cd ..;

.PHONY:output
output:
	mkdir -p output;
	cp -f /home/ubnt/myprogram/TinyServer/src/httpd output/;
	cp -rf wwwroot output/;
	cp -rf conf output/;
	cp -rf log output/;
	cp -rf auto output/;
	cp -f httpd_ctl output/;
	mkdir -p output/wwwroot/cgi;	
	cp -f /home/ubnt/myprogram/TinyServer/cgi/cgi_math output/wwwroot/cgi/
	cp -f /home/ubnt/myprogram/TinyServer/cgi/cgi_math.php output/wwwroot/cgi/
	cp -f /home/ubnt/myprogram/TinyServer/cgi/cgi_math.py output/wwwroot/cgi/

.PHONY:ban_output
ban_output:
	rm -rf output
