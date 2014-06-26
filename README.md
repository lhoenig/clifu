clifu
=====

Command line interface for commandlinefu.com, written in C and using curses
  
## Dependecies  
* curl
* ncurses
* gcc

## Building  
  
`make` `make debug`  
  
## Usage  

`./fu sed`  
  
Will query commandlinefu.com for all commands matching "sed" and show the results in a curses menu.  
Press left or right to toggle between description and command, enter to leave and print the full command.  
