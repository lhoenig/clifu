#include <stdio.h>
#include "errdie.h"

#ifndef CMDOUTPUT_H
#define CMDOUTPUT_H

#define STRIP       1
#define DONT_STRIP  0


// get output of cmd, strip newline if (cut == STRIP_OUTPUT)

char *command_output(char *cmd, int cut) {
  
  FILE *pipe = popen(cmd, "r");
  if (!pipe) error_die("Failed to open pipe");

  char *line = NULL;
  char *c = NULL;
  ssize_t len = 0;
  ssize_t total_bytes = 0;
  size_t cap = 0;

  // read output line by line
  while ((len = getline(&line, &cap, pipe)) > 0) {
      total_bytes += len;
      c = realloc(c, total_bytes + 1);
      if (!c) error_die("realloc error");
      memset(c + (total_bytes - len), 0, len + 1);
      strncat(c, line, len);
  }
  pclose(pipe);

  // cut off last character (newline)
  if (cut == STRIP) {
    c[strlen(c) - 1] = '\0';
  }
  free(line);
  return c;
}

#endif