#ifndef _UTILS_STR_H
#define _UTILS_STR_H
#include <bits/types/mbstate_t.h>
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

char *ltrim(char *s);
char *rtrim(char *s);
char *trim(char *s);
int utf8_display_width(const char *s);
int utf8_str_get(const char *s, char *s_out, size_t index);
int print_until_limit(char *s, size_t limit);
void print_with_blanks(char *s, char blank, size_t limit);
#endif
