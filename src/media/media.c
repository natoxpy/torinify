#include <errors/errors.h>
#include <linux/limits.h>
#include <media/media.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *SQL_INSERT_TO_AUDIOSOURCE =
    "INSERT INTO MediaSource (path) VALUES (?);";

static const char *SQL_GET_ALL_AUDIOSOURCE =
    "select id, path from MediaSource;";

static const char *SQL_GET_ONE_AUDIOSOURCE =
    "select id, path from MediaSource where id = ?;";

static const char *SQL_REMOVE_AUDIOSOURCE =
    "DELETE FROM MediaSource where id = ?;";

void M_free_sources(MediaSources *media_srcs) {
    if (!media_srcs)
        return;

    if (media_srcs->sources) {
        for (int i = 0; i < media_srcs->size; i++) {
            free(media_srcs->sources[i].source);
        }

        free(media_srcs->sources);
    }

    free(media_srcs);
}

void M_free_source(MediaSource *media_src) {
    if (!media_src)
        return;

    if (media_src->source)
        free(media_src->source);

    free(media_src);
}

T_CODE M_register_source(sqlite3 *db, char *dirname) {
    sqlite3_stmt *stmt;
    int ret;

    char fullpath[4096];
    realpath(dirname, fullpath);

    ret = sqlite3_prepare_v2(db, SQL_INSERT_TO_AUDIOSOURCE, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        error_log("Could not prepare sql statement, cause \"%s\"",
                  sqlite3_errmsg(db));

        ret = T_DB_SQL_CANNOT_PREPARE;
        goto cleanup;
    }

    sqlite3_bind_text(stmt, 1, fullpath, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        error_log("SQLITE3 statement failed, cause \"%s\"", sqlite3_errmsg(db));

        ret = T_FAIL;
        goto cleanup;
    }

    ret = T_SUCCESS;

cleanup:
    if (stmt)
        sqlite3_finalize(stmt);

    return ret;
}

T_CODE M_get_sources(sqlite3 *db, MediaSources **media_srcs) {
    sqlite3_stmt *stmt;
    int ret;

    MediaSources *md_srcs = malloc(sizeof(MediaSources));

    if (!md_srcs) {
        error_log("Could not allocate enough memory for MediaSources");
        ret = T_FAIL;
        goto cleanup;
    }

    md_srcs->size = 0;
    md_srcs->sources = NULL;

    ret = sqlite3_prepare_v2(db, SQL_GET_ALL_AUDIOSOURCE, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        error_log("Could not prepare sql statement, cause \"%s\"",
                  sqlite3_errmsg(db));

        ret = T_DB_SQL_CANNOT_PREPARE;
        goto cleanup;
    }

    while ((ret = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *path = sqlite3_column_text(stmt, 1);

        md_srcs->size++;
        MediaSource *sources =
            realloc(md_srcs->sources, sizeof(MediaSource) * md_srcs->size);

        if (!sources) {
            error_log("could not allocate enough memory for MediaSource");
            ret = T_FAIL;
            goto cleanup;
        }

        sources[md_srcs->size - 1].id = id;
        sources[md_srcs->size - 1].source = strdup((const char *)path);

        md_srcs->sources = sources;
    }

    if (ret != SQLITE_DONE) {
        error_log(
            "SQLITE execution did not end as expected, possible cause \"%s\"",
            sqlite3_errmsg(db));

        ret = T_FAIL;
        goto cleanup;
    }

    *media_srcs = md_srcs;

    ret = T_SUCCESS;
cleanup:
    if (stmt)
        sqlite3_finalize(stmt);

    if (ret != T_SUCCESS)
        return ret;

    return T_SUCCESS;
}

T_CODE M_get_source(sqlite3 *db, MediaSource **media_src, int id) {
    sqlite3_stmt *stmt;
    int ret;

    MediaSource *md_src = malloc(sizeof(MediaSources));

    if (!md_src) {
        error_log("Could not allocate enough memory for MediaSources");
        ret = T_FAIL;
        goto cleanup;
    }

    md_src->id = 0;
    md_src->source = NULL;

    ret = sqlite3_prepare_v2(db, SQL_GET_ONE_AUDIOSOURCE, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        error_log("Could not prepare sql statement, cause \"%s\"",
                  sqlite3_errmsg(db));

        ret = T_DB_SQL_CANNOT_PREPARE;
        goto cleanup;
    }

    sqlite3_bind_int(stmt, 1, id);

    if ((ret = sqlite3_step(stmt)) == SQLITE_ROW) {
        md_src->id = sqlite3_column_int(stmt, 0);
        md_src->source = strdup((const char *)sqlite3_column_text(stmt, 1));
    } else {
        ret = T_DB_NOT_FOUND;
        goto cleanup;
    }

    *media_src = md_src;

    ret = T_SUCCESS;
cleanup:
    if (stmt)
        sqlite3_finalize(stmt);

    if (ret != T_SUCCESS) {
        M_free_source(md_src);
        return ret;
    }

    return T_SUCCESS;
}

T_CODE M_remove_source(sqlite3 *db, int id) {
    sqlite3_stmt *stmt;
    int ret;

    ret = sqlite3_prepare_v2(db, SQL_REMOVE_AUDIOSOURCE, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        error_log("Could not prepare sql statement, cause \"%s\"",
                  sqlite3_errmsg(db));

        ret = T_DB_SQL_CANNOT_PREPARE;
        goto cleanup;
    }

    sqlite3_bind_int(stmt, 1, id);

    if ((ret = sqlite3_step(stmt)) != SQLITE_DONE) {
        error_log("Could not execute sql to delete source id \"%d\", possible "
                  "cause \"%s\"",
                  id, sqlite3_errmsg(db));
        ret = T_FAIL;
        goto cleanup;
    }

    ret = T_SUCCESS;
cleanup:
    if (stmt)
        sqlite3_finalize(stmt);

    if (ret != T_SUCCESS)
        return ret;

    return T_SUCCESS;
}

int M_supported_music_file(char *fullpath) {
    FILE *file = fopen(fullpath, "rb");

    if (!file) {
        error_log("Could not open file path \"%s\"", fullpath);
        return 0;
    }

    unsigned char buffer[4];
    size_t read_size = fread(buffer, 1, 4, file);
    fclose(file);

    if (read_size < 4)
        return 0; // File too small to be a music file

    // MP3 (ID3 tag)
    if (buffer[0] == 0x49 && buffer[1] == 0x44 && buffer[2] == 0x33) {
        return 1; // MP3
    }
    // FLAC (fLaC)
    else if (buffer[0] == 0x66 && buffer[1] == 0x4C && buffer[2] == 0x61 &&
             buffer[3] == 0x43) {
        return 1; // FLAC
    }
    // WAV (RIFF)
    else if (buffer[0] == 0x52 && buffer[1] == 0x49 && buffer[2] == 0x46 &&
             buffer[3] == 0x46) {
        return 1; // WAV
    }
    // M4A (ftyp)
    else if (buffer[0] == 0x66 && buffer[1] == 0x74 && buffer[2] == 0x79 &&
             buffer[3] == 0x70) {
        return 1; // M4A
    }
    // OPUS (Opus)
    else if (buffer[0] == 0x4F && buffer[1] == 0x70 && buffer[2] == 0x75 &&
             buffer[3] == 0x73) {
        return 1; // OPUS
    }
    // OGG (OggS)
    else if (buffer[0] == 0x4F && buffer[1] == 0x67 && buffer[2] == 0x67 &&
             buffer[3] == 0x53) {
        return 1; // OGG
    }
    // MIDI (MThd)
    else if (buffer[0] == 0x4D && buffer[1] == 0x54 && buffer[2] == 0x68 &&
             buffer[3] == 0x64) {
        return 1; // MIDI
    }

    return 0; // Not a recognized music file format
}
