#include "common.c"
#include "database/test_album.c"
#include "database/test_album_artist.c"
#include "database/test_album_music.c"
#include "database/test_artist.c"
#include "database/test_migrations.c"
#include "database/test_music.c"
#include "database/test_music_altname.c"
#include "errors/errors.h"
#include <sqlite3.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef void (*db_testfn_t)(sqlite3 *, bool *, char **, char **);

void db_tests() {
    sqlite3 *db;
    sqlite3_open(":memory:", &db);
    printf(" - Database \r");

    db_testfn_t tests[] = {&test_run_migrations,
                           &test_run_migration_2,
                           &test_music_get_preadd,
                           &test_album_get_preadd,
                           &test_artist_get_preadd,
                           &test_music_add,
                           &test_album_add,
                           &test_artist_add,
                           &test_music_get_metadata,
                           &test_music_get,
                           &test_music_get_by_title,
                           &test_music_get_all,
                           &test_music_update,
                           &test_music_add_altname,
                           &test_music_get_altname,
                           &test_music_get_all_altname,
                           &test_music_get_by_any_title,
                           &test_music_delete_altname,
                           &test_album_get,
                           &test_album_get_all,
                           &test_album_update,
                           &test_album_music_add,
                           &test_album_artist_add,
                           &test_artist_get,
                           &test_artist_get_all,
                           &test_artist_update,
                           &test_music_delete,
                           &test_album_delete,
                           &test_artist_delete};

    bool anyfails = false;

    int total_tests = sizeof(tests) / sizeof(db_testfn_t);

    for (int i = 0; i < total_tests; i++) {
        db_testfn_t fn_tr = tests[i];

        bool passed = false;
        char *name = "";
        char *log = "";
        fn_tr(db, &passed, &name, &log);
        char *passed_txt = "\033[32mpassed\033[0m";

        if (!passed) {
            if (anyfails == false) {
                printf(" - Database \033[31mfailed\033[0m \n");
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
        printf(" - Database \033[32mpassed\033[0m all %d tests \n",
               total_tests);

    sqlite3_close(db);
}
