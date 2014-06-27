clifu
=====

Command line interface for commandlinefu.com, written in C and using curses.  
Allows you to get some prompt inspiration where you need it, whithout leaving the terminal.
  
## Dependencies  
* curl
* ncurses
* gcc

## Building  
  
`make`  
`make install`  
  
## Using    

`./fu sed`  
  
Will query commandlinefu.com for all commands matching "sed" and show the results in a curses menu.  
Press left or right to toggle between description and command preview, enter to select and print the full command.  
