#include "./search/test_levenshtein.c"
#include "db/migrations.h"
#include <stdbool.h>
#include <stdio.h>

typedef void (*sh_testfn_t)(bool *, char **, char **);

void sh_tests() {
    printf(" - Search \n");

    sh_testfn_t tests[] = {&test_levenshtein, &test_similarity_score,
                           &test_word_based_similarity_score};

    for (int i = 0; i < sizeof(tests) / sizeof(sh_testfn_t); i++) {
        sh_testfn_t fn_tr = tests[i];

        bool passed = false;
        char *name = "";
        char *log = "";
        fn_tr(&passed, &name, &log);
        char *passed_txt = "\033[32mpassed\033[0m";
        if (!passed)
            passed_txt = "\033[31mfailed\033[0m";

        printf("  - (%s) [%s] %s  \n", passed_txt, name, log);
    }

    error_print_all();
}
