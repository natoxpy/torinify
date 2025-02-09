#ifndef _UTILS_LEVENSHTEIN_H
#define _UTILS_LEVENSHTEIN_H
#include <stddef.h>
int levenshtein_distance(char *s, size_t m, char *t, size_t n);
double similarity_score(char *s, char *t);
double word_based_similarity(const char *s, const char *t);
#endif
