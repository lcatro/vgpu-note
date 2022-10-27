all: 
	gcc -Wall -ggdb -std=gnu99 -o test svga3d.c svga.c test.c -lpciaccess

run:
	sudo chmod 777 ./test
	sudo ./test

clean:
	rm test
