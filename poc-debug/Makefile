poc: svga.c poc.c
	gcc -Wall -ggdb -std=gnu99 -o poc svga.c poc.c -lpciaccess

run:
	sudo chmod 777 ./poc
	sudo ./poc

clean:
	rm poc
