#include "utils/generic_vec.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline int min2(int a, int b) { return (a < b) ? a : b; }
static inline int min3(int a, int b, int c) { return min2(a, min2(b, c)); }

void to_lowercase(char *dest, const char *src) {
    while (*src) {
        *dest++ = tolower((unsigned char)*src++);
    }
    *dest = '\0';
}

int levenshtein_distance(const char *s, size_t m, const char *t, size_t n) {
    if (strcmp(s, t) == 0)
        return 0;
    if (m == 0)
        return n;
    if (n == 0)
        return m;

#ifdef _WIN32
    int **d = malloc((m + 1) * sizeof(int *));
    for (size_t i = 0; i <= m; i++) {
        d[i] = malloc((n + 1) * sizeof(int));
    }
#elif __unix__
    int d[m + 1][n + 1];
#endif

    for (size_t i = 0; i <= m; i++)
        d[i][0] = i;
    for (size_t j = 0; j <= n; j++)
        d[0][j] = j;

    for (size_t i = 1; i <= m; i++) {
        for (size_t j = 1; j <= n; j++) {
            int cost = (s[i - 1] == t[j - 1]) ? 0 : 1;
            d[i][j] = min3(d[i - 1][j] + 1,         // Deletion
                           d[i][j - 1] + 1,         // Insertion
                           d[i - 1][j - 1] + cost); // Substitution
        }
    }

    int value = d[m][n];
#ifdef _WIN32
    // Free the allocated memory
    for (size_t i = 0; i <= m; i++)
        free(d[i]);
    free(d);
#endif

    return value;
}

double similarity_score(const char *s, const char *t) {
    size_t m = strlen(s), n = strlen(t);

#ifdef _WIN32
    char *s_lower = malloc((m + 1) * sizeof(int));
    char *t_lower = malloc((n + 1) * sizeof(int));
#elif __unix__
    char s_lower[m + 1], t_lower[n + 1];
#endif

    to_lowercase(s_lower, s);
    to_lowercase(t_lower, t);

    int dist = levenshtein_distance(s_lower, m, t_lower, n);

#ifdef _WIN32
    free(s_lower);
    free(t_lower);
#endif

    return 1.0 - ((double)dist / (double)(m > n ? m : n));
}

char *strsep(char **stringp, const char *delim) {
    if (*stringp == NULL) {
        return NULL;
    }
    char *token_start = *stringp;
    *stringp = strpbrk(token_start, delim);
    if (*stringp) {
        **stringp = '\0';
        (*stringp)++;
    }
    return token_start;
}

Vec *split_string(char *copy_string, char *delimiter) {
    char splitter[] = {'-', '(', ')', '.', ';'};

    char *out;

    Vec *words = vec_init_with_capacity(sizeof(char **), 8);
    Vec *fw = vec_init_with_capacity(sizeof(char **), 8);

    while ((out = strsep(&copy_string, delimiter)) != NULL) {
        vec_push(words, &out);
    }

    for (int i = 0; i < words->length; i++) {
        char *s = vec_get_ref(words, i);
        char *start = s;
        char *ptr = s;

        if (*ptr == '\0')
            continue;

        while (*ptr != '\0') {
            for (int y = 0; y < sizeof(splitter) / sizeof(char); y++) {
                char splt = splitter[y];

                if (*ptr == splt) {
                    char tmp_ptr = *ptr;

                    *ptr = '\0';

                    if (ptr - start > 0)
                        vec_push(fw, &start);

                    start = ptr + 1;
                }
            }

            ptr++;
        }

        if (ptr - start > 0)
            vec_push(fw, &start);
    }

    vec_free(words);
    return fw;
}

void split_string_free(Vec *v, char *tfree) {
    free(tfree);
    vec_free(v);
}

double word_based_similarity(char *s, char *t) {
    Vec *words_s, *words_t;
    double total_score = 0;
    int calculations = 0;

    char *s_tofree, *s_copy, *t_tofree, *t_copy;

    s_tofree = s_copy = strdup(s);
    t_tofree = t_copy = strdup(t);

    char *delimiter = " ";
    words_s = split_string(s_copy, delimiter);
    words_t = split_string(t_copy, delimiter);

    Vec *opts[2] = {words_s, words_t};

    for (int l = 0; l < 2; l++) {

        Vec *m = opts[l];
        Vec *b = opts[1 - l];

        for (int i = 0; i < m->length; i++) {
            char *m_str = vec_get_ref(m, i);
            if (*m_str == '\xFF')
                m_str++;

            double best = 0;

            for (int j = 0; j < b->length; j++) {
                char *b_str = vec_get_ref(b, j);
                if (*b_str == '\xFF')
                    b_str++;

                double score = similarity_score(m_str, b_str);

                if (score > best)
                    best = score;
            }

            total_score += best;
            calculations++;
        }
    }

    split_string_free(words_s, s_tofree);
    split_string_free(words_t, t_tofree);

    return total_score / (calculations > 0 ? calculations : 1);
}

/*
double o_word_based_similarity(const char *s, const char *t) {
    char *s_copy = strdup(s);
    char *t_copy = strdup(t);
    char **words_s = malloc(20 * sizeof(char *));
    char **words_t = malloc(20 * sizeof(char *));

    int len_s = 0, len_t = 0;

    char *token = strtok(s_copy, " ");
    while (token && len_s < 20) {
        words_s[len_s++] = strdup(token);
        token = strtok(NULL, " ");
    }

    token = strtok(t_copy, " ");
    while (token && len_t < 20) {
        words_t[len_t++] = strdup(token);
        token = strtok(NULL, " ");
    }

    double total_score = 0.0;
    int comparisons = 0;

    for (int i = 0; i < len_s; i++) {
        double best_score = 0.0;
        for (int j = 0; j < len_t; j++) {
            double score = similarity_score(words_s[i], words_t[j]);
            if (score > best_score) {
                best_score = score;
            }
        }
        total_score += best_score;
        comparisons++;
    }

    for (int j = 0; j < len_t; j++) {
        double best_score = 0.0;
        for (int i = 0; i < len_s; i++) {
            double score = similarity_score(words_s[i], words_t[j]);
            if (score > best_score) {
                best_score = score;
            }
        }
        total_score += best_score;
        comparisons++;
    }

    // Free memory
    for (int i = 0; i < len_s; i++)
        free(words_s[i]);
    for (int i = 0; i < len_t; i++)
        free(words_t[i]);
    free(words_s);
    free(words_t);
    free(s_copy);
    free(t_copy);

    return total_score / (comparisons > 0 ? comparisons : 1);
}
*/
