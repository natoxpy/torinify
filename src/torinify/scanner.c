#include "torinify/scanner.h"
#include "db/exec.h"
#include "db/helpers.h"
#include "db/tables.h"
#include "media/scan.h"
#include "storage/album.h"
#include "storage/artist.h"
#include "storage/music.h"
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

    dbh_start_transaction(ctx->db);

    for (int i = 0; i < ctx->scan_ctx->data->length; i++) {
        FileState *file_state = vec_get_ref(ctx->scan_ctx->data, i);

        if (file_state->state != FILE_STATE_ACCEPTED) {
            continue;
        }

        if (file_state->metadata.name == NULL) {

            wchar_t wtitle[1012];
            mbstowcs(wtitle, file_state->filepath, 1012);
        }

        // wprintf("%ls\n", wtitle);

        Music music = {.id = -1,
                       .title = file_state->metadata.name,
                       .fullpath = file_state->filepath};

        if (s_music_add(ctx->db, &music) != TDB_SUCCESS) {
            continue;
        }

        Vec *albums;
        Album *album;
        s_album_get_by_title(ctx->db, file_state->metadata.album, &albums);
        Album album_new = {
            .id = 0, .title = file_state->metadata.album, .year = "0000-00-00"};

        if (albums->length == 0 && file_state->metadata.album) {

            s_album_add(ctx->db, &album_new);
            s_music_add_album(ctx->db, music.id, album_new.id);

            album = &album_new;
        } else if (albums->length != 0) {
            Album *album_ref = vec_get_ref(albums, 0);
            s_music_add_album(ctx->db, music.id, album_ref->id);
            album = album_ref;
        }

        Vec *artists;
        s_artist_get_by_name(ctx->db, file_state->metadata.artist, &artists);

        if (artists->length == 0 && file_state->metadata.artist) {
            Artist artist = {.id = 0, .name = file_state->metadata.artist};
            s_artist_add(ctx->db, &artist);
            s_album_add_artist(ctx->db, album->id, artist.id,
                               ARTIST_TYPE_ARTIST);
        } else if (artists->length != 0) {
            Artist *artist_ref = vec_get_ref(artists, 0);
            s_album_add_artist(ctx->db, album->id, artist_ref->id,
                               ARTIST_TYPE_ARTIST);
        }

        s_album_vec_free(albums);
        s_artist_vec_free(artists);
    }

    dbh_commit_transaction(ctx->db);

    sc_scan_context_free(ctx);
}

void sc_scan_context_free_and_cancel(ScannerContext *ctx) {
    sc_scan_context_free(ctx);
}
