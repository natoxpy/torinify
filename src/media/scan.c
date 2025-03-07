#include "crossplatform/directories.h"
#include "db/exec/music_table.h"
#include "db/migrations.h"
#include <media/scan.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <taglib/tag_c.h>
#include <time.h>
#include <utils/generic_vec.h>

int supported_music_file(char *fullpath) {
#ifdef _WIN32
    wchar_t wfullpath[1024];
    mbstowcs(wfullpath, fullpath, 1024); // Convert the string
    MultiByteToWideChar(CP_UTF8, 0, fullpath, -1, wfullpath, 1024);

    FILE *file = _wfopen(wfullpath, L"rb");

    if (!file) {
        error_log("Could not open file path \"%s\"", fullpath);
        return 0;
    }
#elif __unix__
    FILE *file = fopen(fullpath, "rb");

    if (!file) {
        error_log("Could not open file path \"%s\"", fullpath);
        return 0;
    }
#endif

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

FileState *file_state_init() {
    FileState *fs = malloc(sizeof(FileState));

    fs->state = FILE_STATE_FOUND;
    fs->filepath = NULL;
    fs->filename = NULL;
    fs->metadata.name = NULL;
    fs->metadata.artist = NULL;
    fs->metadata.album = NULL;

    return fs;
}

void file_state_vec_free(Vec *files) {
    if (!files)
        return;

    for (int i = 0; i < files->length; i++) {
        FileState *fs = vec_get_ref(files, i);

        free(fs->filepath);
        free(fs);
    }

    vec_free(files);
}

void recursive_readdir(Vec *sources, Vec **out) {
    Vec *files = vec_init(sizeof(FileState *));

    for (int i = 0; i < sources->length; i++) {
        char *path = vec_get_ref(sources, i);
        Directory *dir = directory_open(path);
        if (dir == NULL)
            continue;

        DirectoryEntry *entry;

        Vec *dirs = vec_init(sizeof(Directory *));
        Vec *paths = vec_init(sizeof(char *));

        vec_push(dirs, &dir);
        vec_push(paths, &path);

        while (1) {
            while ((entry = directory_read(dir)) != NULL) {
                if (entry->name[0] == '.')
                    continue;

                char np[2048];
                char *r = NULL;

                for (int i = 0; i < paths->length; i++) {
                    if (r == NULL) {
                        sprintf(np, "%s", (char *)vec_get_ref(paths, i));
                    } else {
                        sprintf(np, "%s/%s", r, (char *)vec_get_ref(paths, i));
                    }

                    r = np;
                }

                sprintf(np, "%s/%s", np, entry->name);

                if (entry->type == ENTRY_TYPE_DIRECTORY) {
                    dir = directory_open(np);

                    if (dir != NULL) {
                        vec_push(dirs, &dir);
                        char *name = entry->name;
                        vec_push(paths, &name);
                    } else {
                        dir = vec_get_ref(dirs, dirs->length - 1);
                    }
                } else if (entry->type == ENTRY_TYPE_FILE) {
                    if (supported_music_file(np)) {
                        char *name = strdup(np);

                        FileState *fs = file_state_init();
                        fs->filepath = name;
                        fs->filename = strdup(entry->name);

                        vec_push(files, &fs);
                    }
                }
            }

            if (paths->length <= 1)
                break;
            else {
                char *s = vec_pop_ref(paths);
                Directory *top = vec_pop_ref(dirs);
                directory_close(top);
                dir = vec_get_ref(dirs, dirs->length - 1);
            }
        }

        for (int i = 0; i < dirs->length; i++) {
            directory_close(vec_get_ref(dirs, i));
        }

        vec_free(dirs);
        vec_free(paths);
    }

    *out = files;
}

void scan_file(char *fullpath, MusicContext *music_ctx) {

    TagLib_File *file = taglib_file_new(fullpath);

    if (!file) {
        return;
    }

    TagLib_Tag *tag = taglib_file_tag(file);

    if (!tag) {
        taglib_file_free(file);
        return;
    }

    music_ctx->name = strdup(taglib_tag_title(tag));
    music_ctx->artist = strdup(taglib_tag_artist(tag));
    music_ctx->album = strdup(taglib_tag_album(tag));

    taglib_file_free(file);
    taglib_tag_free_strings();
}

int scan_thread(void *arg) {
    ThreadContext *thread_ctx = arg;

    for (int i = 0; i < thread_ctx->data->length; i++) {
        FileState *file_state = vec_get_ref(thread_ctx->data, i);

        MusicContext mc = {0, 0, 0};

        mtx_lock(thread_ctx->mutex);

        scan_file(file_state->filepath, &mc);

        file_state->metadata = mc;
        file_state->state = FILE_STATE_ACCEPTED;

        thread_ctx->working_index = i;
        (*thread_ctx->processed)++;

        MusicRow *row;
        if (DB_query_music_single_by_fullpath(
                thread_ctx->db, file_state->filepath, &row) == TDB_SUCCESS) {
            file_state->state = FILE_STATE_ALREADY_IN;
            dbt_music_row_free(row);
        }

        mtx_unlock(thread_ctx->mutex);
    }

    return 0;
}

int _wait_scan(void *args) {

    ScanContext *scan_ctx = (ScanContext *)args;

    for (int i = 0; i < scan_ctx->threads->length; i++) {
        thrd_t *thread = vec_get(scan_ctx->threads, i);
        thrd_join(*thread, NULL);
    }

    mtx_lock(&scan_ctx->mutex);
    scan_ctx->finalized = 1;
    mtx_unlock(&scan_ctx->mutex);

    return 0;
}

///
/// @todo
/// This code work with pthread multithreading library which is posix only,
/// please refactor to work on windows as well.
ScanContext *start_scan(sqlite3 *db, Vec *sources, int threads) {
    Vec *files;
    recursive_readdir(sources, &files);

    Vec *split_files;
    vec_n_split_vec(files, &split_files, threads);

    ScanContext *scan_ctx = malloc(sizeof(ScanContext));

    scan_ctx->data = files;
    scan_ctx->finalized = 0;
    scan_ctx->threads = vec_init(sizeof(thrd_t *));
    scan_ctx->threads_ctx = vec_init(sizeof(ThreadContext *));
    scan_ctx->processed = 0;

    mtx_init(&scan_ctx->mutex, mtx_plain);

    for (int i = 0; i < threads; i++) {
        ThreadContext *ctx = malloc(sizeof(ThreadContext));

        ctx->data = vec_get_ref(split_files, i);
        ctx->mutex = &scan_ctx->mutex;
        ctx->working_index = 0;
        ctx->processed = &scan_ctx->processed;
        ctx->db = db;

        thrd_t thread;
        thrd_create(&thread, scan_thread, ctx);

        vec_push(scan_ctx->threads_ctx, &ctx);
        vec_push(scan_ctx->threads, &thread);
    }

    vec_free(split_files);

    thrd_create(&scan_ctx->joins_thread, _wait_scan, scan_ctx);

    return scan_ctx;
}

int lock_scan(ScanContext *scan_ctx) {
    mtx_lock(&scan_ctx->mutex);
    return scan_ctx->finalized;
};

void unlock_scan(ScanContext *scan_ctx) { mtx_unlock(&scan_ctx->mutex); };

void finalize_scan(ScanContext *scan_ctx) {

    for (int i = 0; i < scan_ctx->data->length; i++) {
        FileState *fs = vec_get_ref(scan_ctx->data, i);

        if (fs->filepath != 0)
            free(fs->filepath);

        if (fs->filename != 0)
            free(fs->filename);

        if (fs->metadata.name != 0)
            free(fs->metadata.name);

        if (fs->metadata.artist != 0)
            free(fs->metadata.artist);

        if (fs->metadata.album != 0)
            free(fs->metadata.album);

        free(fs);
    }

    for (int i = 0; i < scan_ctx->threads_ctx->length; i++) {
        ThreadContext *ctx = vec_get_ref(scan_ctx->threads_ctx, i);
        vec_free(ctx->data);
        free(ctx);
    }

    vec_free(scan_ctx->data);
    vec_free(scan_ctx->threads);
    vec_free(scan_ctx->threads_ctx);

    mtx_destroy(&scan_ctx->mutex);
    free(scan_ctx);
}
