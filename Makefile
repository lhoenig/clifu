all:
	gcc -Wall -std=gnu99 clifu.c -o fu -lncurses -lmenu

debug:
	gcc -Wall -std=gnu99 -g -O0 clifu.c -o fu -lncurses -lmenu

install:
	cp fu /usr/local/bin

clean:
	rm fu
