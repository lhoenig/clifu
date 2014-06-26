#include <stdio.h>

#ifndef ERRDIE_H
#define ERRDIE_H

void error_die(char *m) {
  fprintf(stderr, "ERROR: %s\n", m);
  exit(1);
}

#endif