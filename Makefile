all:
	gcc -Wall -lncurses -lmenu clifu.c -o clifu

debug:
	gcc -Wall -g -lncurses -lmenu clifu.c -o clifu

install:
	mv clifu /usr/local/bin

clean:
	rm clifu
