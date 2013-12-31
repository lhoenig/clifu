clifu
=====

Interface to the commandlinefu.com API, written in C and using Ncurses

BETA - there is still a memory bug in the parse_page_content function. Filling up the results array is not reliable yet. Will investigate very soon.
  
## Requirements  
* curl
* ncurses (on Linux: libncurses5-dev package)
* gcc

## Building  
  
`make && sudo make install`  
  
## Usage  

`clifu sed`  
  
Will query commandlinefu.com for all commands matching "sed" and show the results in an Ncurses menu.  
Press left or right to toggle between description and command, enter to quit and print the full command.  
