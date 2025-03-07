#include "torinify/scanner.h"
#include "db/exec.h"
#include "db/tables.h"
#include "media/scan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ScannerContext *sc_scan_start_single_root(sqlite3 *db, int root_id) {
    Vec *roots = vec_init(sizeof(char *));

    Vec *sources;
    DB_query_source_all(db, &sources);

    for (int i = 0; i < sources->length; i++) {
        MediaSourceRow *row = vec_get_ref(sources, i);
        if (root_id == row->id) {
            char *s = strdup(row->path);
            vec_push(roots, &s);

            break;
        }
    }

    dbt_source_vec_rows_free(sources);

    return sc_scan_start(db, roots);
}

ScannerContext *sc_scan_start_all_roots(sqlite3 *db) {
    Vec *roots = vec_init(sizeof(char *));

    Vec *sources;
    DB_query_source_all(db, &sources);

    for (int i = 0; i < sources->length; i++) {
        MediaSourceRow *row = vec_get_ref(sources, i);
        char *s = strdup(row->path);
        vec_push(roots, &s);
    }

    return sc_scan_start(db, roots);
}

ScannerContext *sc_scan_start(sqlite3 *db, Vec *roots) {
    ScanContext *scan_ctx = start_scan(db, roots, 1);

    ScannerContext *ctx = malloc(sizeof(ScannerContext));

    if (ctx == NULL)
        return NULL;

    ctx->db = db;
    ctx->scan_ctx = scan_ctx;
    ctx->roots = roots;

    return ctx;
}

int sc_lock_scan(ScannerContext *ctx) { return lock_scan(ctx->scan_ctx); }
void sc_unlock_scan(ScannerContext *ctx) { unlock_scan(ctx->scan_ctx); }

void sc_scan_context_free(ScannerContext *ctx) {
    if (ctx == NULL)
        return;

    finalize_scan(ctx->scan_ctx);
    for (int i = 0; i < ctx->roots->length; i++) {
        char *root = vec_get_ref(ctx->roots, i);
        free(root);
    }
    vec_free(ctx->roots);
    free(ctx);
}

void sc_scan_context_free_and_commit(ScannerContext *ctx) {

    for (int i = 0; i < ctx->scan_ctx->data->length; i++) {
        FileState *file_state = vec_get_ref(ctx->scan_ctx->data, i);

        if (file_state->state == FILE_STATE_ACCEPTED) {
            printf("Accepted %s\n", file_state->filename);
        }
    }

    sc_scan_context_free(ctx);
}

void sc_scan_context_free_and_cancel(ScannerContext *ctx) {
    sc_scan_context_free(ctx);
}
