#ifndef _MEDIA_SCAN_H
#define _MEDIA_SCAN_H

#include <pthread.h>
#include <sqlite3.h>
#include <stdint.h>
#include <utils/generic_vec.h>

#define FILE_STATE_FOUND 0
#define FILE_STATE_ANALYSING 1
#define FILE_STATE_ACCEPTED 2
#define FILE_STATE_REJECTED 3
#define FILE_STATE_ALREADY_IN 4

typedef struct {
    char *name;
    char *artist;
    char *album;
} MusicContext;

typedef struct {
    int state;
    char *filename;
    char *filepath;
    MusicContext metadata;
} FileState;

typedef struct {
    Vec *data;
    pthread_mutex_t *mutex;
    uint32_t working_index;
    uint32_t *processed;
} ThreadContext;

typedef struct {
    pthread_mutex_t mutex;
    Vec *data;
    Vec *threads;
    Vec *threads_ctx;
    pthread_t joins_thread;
    int finalized;
    uint32_t processed;
} ScanContext;

int msleep(long msec);

ScanContext *start_scan(sqlite3 *db, Vec *sources, int threads);

/**
 * @memberof ScanContext
 * @return `0` if the scan has not finalized, and `1` if it has
 */
int wait_scan(ScanContext *scan_ctx, Vec **data);

/**
 * @memberof ScanContext
 */
void finalize_scan(ScanContext *scan_ctx);

/**
 * @memberof ScanContext
 */
int lock_scan(ScanContext *scan_ctx);

/**
 * @memberof ScanContext
 */
void unlock_scan(ScanContext *scan_ctx);

#endif
