#include <stdio.h>

#ifndef COMMON
#define COMMON
void _print_err(char *err) { fprintf(stderr, "    - %s\n", err); }
#endif
