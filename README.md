clifu
=====

Interface to the commandlinefu.com API, written in C and using Ncurses

BETA - there is still a memory bug lying around in the parse_page_content function. Filling up the results array is not reliable yet.

## Building  
  
`make && sudo make install`  
  
## Usage  

`clifu sed`  
Will query commandlinefu.com for all commands matching "sed". Pressing enter in the menu will quit the program and show the corresponging command.
