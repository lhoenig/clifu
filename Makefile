all:
	gcc -Wall clifu.c -o clifu -lncurses -lmenu

debug:
	gcc -Wall -g clifu.c -o clifu -lncurses -lmenu

install:
	mv clifu /usr/local/bin

clean:
	rm clifu
