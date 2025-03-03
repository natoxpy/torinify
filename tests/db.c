#include "database/test_migrations.c"
#include "database/test_music_table.c"
#include "db/migrations.h"
#include <sqlite3.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef void (*db_testfn_t)(sqlite3 *, bool *, char **, char **);

void db_tests() {
    sqlite3 *db;
    sqlite3_open(":memory:", &db);
    printf(" - Database \n");

    db_testfn_t tests[] = {
        &test_run_migrations,  &test_run_migrations_after_migration,
        &test_add_music,       &test_query_music,
        &test_remove_music,    &test_insert_many_music_with_transaction,
        &test_query_all_music, &test_remove_all_music};

    for (int i = 0; i < sizeof(tests) / sizeof(db_testfn_t); i++) {
        db_testfn_t fn_tr = tests[i];

        bool passed = false;
        char *name = "";
        char *log = "";
        fn_tr(db, &passed, &name, &log);
        char *passed_txt = "\033[32mpassed\033[0m";
        if (!passed)
            passed_txt = "\033[31mfailed\033[0m";

        printf("  - (%s) [%s] %s  \n", passed_txt, name, log);
    }

    error_print_all();

    sqlite3_close(db);
}
