#include <stdio.h>
#include "errdie.h"

#ifndef CMDOUTPUT_H
#define CMDOUTPUT_H

#define STRIP_OUTPUT       1
#define DONT_STRIP_OUTPUT  0


// get output of cmd, strip newline if (cut == STRIP_OUTPUT)

char *command_output(char *cmd, int cut) {
  
  FILE *pipe = popen(cmd, "r");
  if (!pipe) error_die("Failed to open pipe");

  char *tmp = malloc(1);
  char *c = malloc(1);
  int b_read = 0;
  int t_read = 0;
  size_t cap = 0;

  // read output line by line
  while ((b_read = getline(&tmp, &cap, pipe)) != -1) {
      t_read += b_read;
      c = realloc(c, t_read + 1);
      if (!c) error_die("realloc failed");
      strncat(c, tmp, t_read - strlen(c));
  }
  pclose(pipe);

  // cut off last character (newline)
  if (cut == STRIP_OUTPUT) 
    c[strlen(c) - 1] = '\0';
  
  free(tmp);
  return c;
}

#endif