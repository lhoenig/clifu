all:
	gcc -Wall -std=gnu99 clifu.c -o clifu -lncurses -lmenu

debug:
	gcc -Wall -std=gnu99 -g -O0 clifu.c -o clifu -lncurses -lmenu

install:
	cp clifu /usr/local/bin

clean:
	rm clifu
