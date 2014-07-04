// clifu.c 
//
// ncurses interface to commandlinefu.com
// github.com/rub1k/clifu

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>
#include <menu.h>
#include "errdie.h"
#include "cmdout.h"
#include "base64.h"

#define MAX_ENTRIES  100

#define ANSI_CYAN    "\x1b[36m"
#define ANSI_LGREEN  "\x1b[92m"
#define ANSI_RESET   "\x1b[0m"


// data structure for a parsed API page
typedef struct page {
  char *commands[MAX_ENTRIES];
  char *descriptions[MAX_ENTRIES];
  int entries;
} page;


// global page object
page parsed_page;


char *construct_url(char *);
char *page_content(char *);
void parse_page(const char *);
int setup_menu(void);
char *substr(char *, int, int, char *);
void print_usage(char *);
void dump_page(int, int, int);


int main(int argc, char *argv[]) {
	
	// check if search pattern is supplied
  if (argc >= 2) {
    
    char *url = construct_url(argv[1]);

    // curl the request url
    char *content = page_content(url);

    // put matches into parsed_page
    parse_page(content);

    if (parsed_page.entries > 0) {  
      
      int sel_cmd = setup_menu();
      
      if (sel_cmd != -1)
        printf("\n%s%s\n%s%s%s\n\n", ANSI_LGREEN, 
                                     parsed_page.descriptions[sel_cmd], 
                                     ANSI_CYAN,
                                     parsed_page.commands[sel_cmd],
                                     ANSI_RESET);

    } else printf("No matches found\n");
    
    free(url);
    free(content);
    for (int i = 0; i < parsed_page.entries; ++i) {
      free(parsed_page.commands[i]);
      free(parsed_page.descriptions[i]);
    }
    return 0;
  
  } else { 
    print_usage(argv[0]);
    return 1;
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
  memset(c, 0, len2);
	strncpy(c, url_with_b64, len2);
  free(b64_encoded);
	return c;
}



// get page using curl

char *page_content(char *url) {

  // construct curl command
  char *curl = command_output("which curl", STRIP);
  if (strlen(curl) == 0) error_die("No version of curl found");

  const char *flags = " -s ";
  int l = strlen(curl) + strlen(flags) + strlen(url);
  char *cmd = malloc(l);
  memset(cmd, 0, l);
  strcpy(cmd, curl);
  strcat(cmd, flags);
  strcat(cmd, url);
  
  // run command and return output
  char *out = command_output(cmd, DONT_STRIP);
  free(cmd);
  free(curl);
  return out;
}



// fill parsed_page with regex-parsed commands and descriptions

void parse_page(const char *to_match) {
    
    regex_t r;                              // regex object
    const char *expr = "(#.+\n)(.+)";       // single entry representation

    const char *p = to_match;   // points to the end of the previous match
    int c = 0;                  // number of matches
    int end = 0;                // loop flag

    // compile regex and check success
    int ret_comp = regcomp(&r, expr, REG_EXTENDED|REG_NEWLINE);
    if (ret_comp) error_die("Could not compile regex");

    while (!end) {
      
      regmatch_t m[3];            // pmatch array     
      
      if (c < MAX_ENTRIES && (regexec(&r, p, 3, m, 0) != REG_NOMATCH)) {
      
        if (m[0].rm_so == -1) continue;
        
        for (int i = 1; i <= 2; i++) {
          
          // extract matched substring
          int start = m[i].rm_so + (p - to_match);
          int finish = m[i].rm_eo + (p - to_match);
          int len = (finish - start);
          const char *fstring = to_match + start;
          
          char *match_str = malloc(len + 1);
          memset(match_str, 0, len + 1);
          strncpy(match_str, fstring, len);
          match_str[len] = '\0';

          if (i == 1) {       // description       
            int len = strlen(match_str) - 3;
            char *remove_front = malloc(len + 1);
            memset(remove_front, 0, len + 1);
            strncpy(remove_front, match_str + 2, len);
            remove_front[len + 1] = '\0';
            parsed_page.descriptions[c] = remove_front;
          }
          else if (i == 2) {  // command 
            // TODO check if entry is already contained
            parsed_page.commands[c] = match_str;     
          }
        }
        p += m[0].rm_eo;  // advance string pointer
        c++;
      }
      else end = 1;
    }
    regfree(&r);
    parsed_page.entries = c;
}



// create the ncurses menu

int setup_menu(void) {
  
  ITEM **items;
  MENU *menu;
  int n_items = parsed_page.entries;
  items = (ITEM **)calloc(n_items + 1, sizeof(ITEM *));
  
  int i;
  for (i = 0; i < n_items; ++i) {
          const char *title = parsed_page.descriptions[i];
          items[i] = new_item(title, "");
          
          if (items[i] == NULL) {
            printf("i = %i\n", i);
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

  // menu creation
  menu = new_menu(items);
  set_menu_format(menu, n_items, 1);
  set_menu_mark(menu, " ");
  post_menu(menu);
  refresh();

  int selected_cmd = 0;
  int c;  // keycode stored here
  int end = 0;
  int rst = 0;
  
  // menu key actions
  while(!end) {
    c = wgetch(stdscr);
    switch(c) { 
        case KEY_DOWN:
            if (rst) {
              int index = item_index(current_item(menu));
              char *n_title = parsed_page.descriptions[index];
              items[index] = new_item(n_title, "");
              unpost_menu(menu);
              set_menu_items(menu, items);
              post_menu(menu);
              set_current_item(menu, items[index]);
              rst = 0;
            } 
            menu_driver(menu, REQ_DOWN_ITEM);
            break;
        case KEY_UP:
            if (rst) {
              int index = item_index(current_item(menu));
              char *n_title = parsed_page.descriptions[index];
              items[index] = new_item(n_title, "");
              unpost_menu(menu);
              set_menu_items(menu, items);
              post_menu(menu);
              set_current_item(menu, items[index]);
              rst = 0;
            } 
            menu_driver(menu, REQ_UP_ITEM);
            break;
        case KEY_RIGHT: {
            int index = item_index(current_item(menu));
            char *n_title = parsed_page.commands[index];
            items[index] = new_item(n_title, "");
            unpost_menu(menu);
            set_menu_items(menu, items);
            post_menu(menu);
            set_current_item(menu, items[index]);
            rst = 1;
            refresh();
            break; }
        case KEY_LEFT: {
            int index = item_index(current_item(menu));
            char *n_title = parsed_page.descriptions[index];
            items[index] = new_item(n_title, "");
            unpost_menu(menu);
            set_menu_items(menu, items);
            post_menu(menu);
            set_current_item(menu, items[index]);
            rst = 0;
            refresh();
            break; }
        case 0x71:   // q
            selected_cmd = -1;
            end = 1;
            break;
        case 0xA:    // enter
            selected_cmd = item_index(current_item(menu));
            end = 1;
            break;
    }
  } 

  // menu cleanup
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



void dump_page(int a, int b, int mode) {
  int i;
  if (mode == 0 || mode == 2) {
    for (i = a; i <= b; i++) {
      printf("%i: \"%s\"\n", i, parsed_page.descriptions[i]);
    }
  }
  if (mode == 1 || mode == 2) {
    for (i = a; i <= b; i++) {
      printf("%i: \"%s\"\n", i, parsed_page.commands[i]);
    }
  } 
}


void print_usage(char *arg0) {
	printf("%s\n%s%s%s\n", 
         "grep the commandlinefu.com archive.",  
         "usage: ", arg0, " <pattern>");
}
