#include "./search/test_levenshtein.c"
#include "common.c"
#include "db/migrations.h"
#include <stdbool.h>
#include <stdio.h>

typedef void (*sh_testfn_t)(bool *, char **, char **);

void sh_tests() {
    sh_testfn_t tests[] = {&test_levenshtein, &test_similarity_score,
                           &test_word_based_similarity_score};

    bool anyfails = false;

    int total_tests = sizeof(tests) / sizeof(sh_testfn_t);

    for (int i = 0; i < total_tests; i++) {
        sh_testfn_t fn_tr = tests[i];

        bool passed = false;
        char *name = "";
        char *log = "";
        fn_tr(&passed, &name, &log);
        char *passed_txt = "\033[32mpassed\033[0m";

        if (!passed) {
            if (anyfails == false) {
                printf(" - Search \033[31mfailed\033[0m \n");
            }

            anyfails = true;
            passed_txt = "\033[31mfailed\033[0m";

            if (get_log_index() != 0) {
                passed_txt = "\033[41mfatal\033[0m";
            }

            printf("  - (%s) [%s] %s  \n", passed_txt, name, log);
        }

        error_print_all_cb(_print_err);
    }

    if (!anyfails)
        printf(" - Search \033[32mpassed\033[0m all %d tests \n", total_tests);

    error_print_all();
}
