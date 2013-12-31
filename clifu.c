#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>
#include <menu.h>
#include "errdie.h"
#include "cmd_output.h"
#include "base64.h"

#define MAX_ENTRIES  100

#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


// data structure for a parsed API page
typedef struct page {
  char *commands[MAX_ENTRIES];
  char *descriptions[MAX_ENTRIES];
  int entries;
} page;


char *construct_url(char *);
char *page_content(char *);
page parse_page_content(const char *);
int setup_menu(page *);
char *substr(char *, int, int, char *);
void print_usage(char *);


int main(int argc, char *argv[]) {
	
	// check if search pattern is supplied
  if (argc >= 2) {
    
    // request url
    char *url = construct_url(argv[1]);
    // curl the request url
    char *content = page_content(url);
    // put matches into parsed_page
    page parsed_page = parse_page_content(content);
    
    if (parsed_page.entries > 0) {  
      int sel_cmd = setup_menu(&parsed_page);
      if (sel_cmd != -1)
        printf("\n%s%s%s\n\n", ANSI_COLOR_CYAN, parsed_page.commands[sel_cmd], ANSI_COLOR_RESET);
        //printf("\n%s\n\n", parsed_page.commands[sel_cmd]);
    } else {
        printf("No match found\n");
    }

    free(url);
    free(content);
    int i;
    for (i = 0; i < parsed_page.entries; ++i) {
      free(parsed_page.commands[i]);
      free(parsed_page.descriptions[i]);
    }

    return 0;
  
  } else { 
    print_usage(argv[0]);
    return(0);
  }
}



// construct request url

char *construct_url(char *grep) {
	
	char *base = "http://www.commandlinefu.com/commands/matching/";
	
	int len = strlen(base) + strlen(grep) + 1;
	char url[len];
	strncpy(url, base, len);
	strcat(url, grep);
	strcat(url, "/");

  int b64_len;
  char *b64_encoded = base64((const void *)grep , strlen(grep), &b64_len);

  int len2 = strlen(url) + b64_len + strlen("/plaintext");
	char url_with_b64[len2];
	strncpy(url_with_b64, url, len2);
	strcat(url_with_b64, b64_encoded);
	strcat(url_with_b64, "/plaintext");

	char *c = malloc(len2);
	strncpy(c, url_with_b64, len2);
  free(b64_encoded);
	return c;
}



// get page using curl

char *page_content(char *url) {

  // construct curl command
  char *curl = command_output("which curl", STRIP_OUTPUT);
  if (strlen(curl) == 0) error_die("No version of curl found");

  const char *flags = " -s ";
  char *cmd = malloc(strlen(curl) + strlen(flags) + strlen(url));
  strcpy(cmd, curl);
  strcat(cmd, flags);
  strcat(cmd, url);
  
  // run command and return output
  char *out = command_output(cmd, DONT_STRIP_OUTPUT);
  free(cmd);
  free(curl);
  return out;
}



// return structure with regex-parsed commands and descriptions

struct page parse_page_content(const char *to_match) {
    
    struct page ret_page;                   // returned page structure
    regex_t r;                              // regex object
    const char *expr = "(#.+\n)(.+)";       // expression to match against
    
    // compile regex and check success
    int ret_comp = regcomp(&r, expr, REG_EXTENDED|REG_NEWLINE);
    if(ret_comp) error_die("Could not compile regex");

    const char *p = to_match;     // 'p' is a pointer into the string which points to the end of the previous match
    regmatch_t m[MAX_ENTRIES];    // contains the matches found
    int c = 0;                    // number of matches

    while (1) {

        int i = 0;
        int nomatch = regexec(&r, p, MAX_ENTRIES, m, 0);
        if (nomatch) break;
        
        for (i = 0; i < MAX_ENTRIES; i++) {
            
            if (m[i].rm_so == -1) break;
            
            int start = m[i].rm_so + (p - to_match);
            int finish = m[i].rm_eo + (p - to_match);
            
            if (i != 0) {

                int len = (finish - start);
                const char *fstring = to_match + start;
                char stripped[len + 1];
                strncpy(stripped, fstring, len);
                stripped[len] = '\0';
                
                char *as_pointer = malloc(strlen(stripped)); // make consistent
                strcpy(as_pointer, stripped);

                if (i == 1) {   // description
                  char *remove_front = malloc(strlen(as_pointer) -2);
                  substr(as_pointer, 2, strlen(as_pointer) - 3, remove_front);
                  //printf("%s\n", remove_front);
                  ret_page.descriptions[c] = remove_front;
                  //ret_page.descriptions[c] = malloc(strlen(remove_front));
                  //strcpy(ret_page.descriptions[c], remove_front);
                }
                if (i == 2) {   // command 
                  ret_page.commands[c] = as_pointer;
                  //ret_page.commands[c] = malloc(strlen(as_pointer));
                  //strcpy(ret_page.commands[c], as_pointer);
                }
            }
        }
        p += m[0].rm_eo;
        c++;
    }
    regfree(&r);
    ret_page.entries = c - 1;
    
    return ret_page;
}



// setup the ncurses menu

int setup_menu(page *p) {
  
  ITEM **items;
  MENU *menu;
  int n_items = p->entries;
  items = (ITEM **)calloc(n_items + 1, sizeof(ITEM *));
  
  int i;
  for (i = 0; i < n_items; ++i) {
          const char *title = p->descriptions[i];
          items[i] = new_item(title, "");
          
          if (items[i] == NULL) {
            error_die("Error creating menu item");
            break;
          }
  }
  items[n_items] = 0;

  // start ncurses
  initscr();
  cbreak();
  keypad(stdscr, TRUE);
  noecho();

  // create the menu
  menu = new_menu(items);
  set_menu_format(menu, n_items, 1);
  set_menu_mark(menu, " ");
  post_menu(menu);
  refresh();

  int selected_cmd = 0;
  int c ;  // keycode stored here
  int end = 0;
  
  // menu key actions
  while(end == 0) {
    c = wgetch(stdscr);
    switch(c) { 
        case KEY_DOWN:
            menu_driver(menu, REQ_DOWN_ITEM);
            break;
        case KEY_UP:
            menu_driver(menu, REQ_UP_ITEM);
            break;
        case KEY_RIGHT: {
            int index = item_index(current_item(menu));
            char *n_title = p->commands[index];
            items[index] = new_item(n_title, "");
            unpost_menu(menu);
            set_menu_items(menu, items);
            post_menu(menu);
            set_current_item(menu, items[index]);
            refresh();
            break; }
        case KEY_LEFT: {
            int index = item_index(current_item(menu));
            char *n_title = p->descriptions[index];
            items[index] = new_item(n_title, "");
            unpost_menu(menu);
            set_menu_items(menu, items);
            post_menu(menu);
            set_current_item(menu, items[index]);
            refresh();
            break; }
        case 0x71:   // q
            selected_cmd = -1;
            end = 1;
            break;
        case 0xA:   // enter
            selected_cmd = item_index(current_item(menu));
            end = 1;
            break;
    }
  } 

  // cleanup the menu
  unpost_menu(menu);
  free_menu(menu);
  int n;
  for (n = 0; i < n_items; ++n) {
      free_item(items[i]);  
  }
  free(items);
  endwin();

  return selected_cmd;
}



////////////////////////////////
//                            //   
//       HELPER FUNCTIONS     //
//                            //
//                            //
////////////////////////////////


char *substr(char *input, int offset, int len, char *dest) {
  
  int input_len = strlen(input);
  if (offset + len > input_len) {
     return 0;
  }
  strncpy(dest, input + offset, len);
  return dest;
}


void print_usage(char *arg0) {
	printf("%s\n%s%s%s\n", 
         "grep the commandlinefu.com archive.",  
         "usage: ", arg0, " <pattern>");
}
