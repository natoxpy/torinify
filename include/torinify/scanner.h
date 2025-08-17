#ifndef _TORINIFY_SCANNER_H
#define _TORINIFY_SCANNER_H

#include "media/scan.h"

typedef struct {
    sqlite3 *db;
    ScanContext *scan_ctx;
    Vec *roots;
} ScannerContext;

ScannerContext *sc_scan_start_single_root(sqlite3 *db, int root_id);
ScannerContext *sc_scan_start_all_roots(sqlite3 *db);
ScannerContext *sc_scan_start(sqlite3 *db, Vec *roots);

/**
 * @param db
 * @param sources - Vec full of item references MediaSourceRow
 */
ScannerContext *sc_scan_start_from_vec(sqlite3 *db, Vec *sources);

int sc_lock_scan(ScannerContext *ctx);
void sc_unlock_scan(ScannerContext *db);

void sc_scan_context_free(ScannerContext *db);

void sc_scan_context_free_and_commit(ScannerContext *db);
void sc_scan_context_free_and_cancel(ScannerContext *db);

#endif
