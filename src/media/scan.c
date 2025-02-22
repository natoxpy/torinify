#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <media/scan.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <taglib/tag_c.h>
#include <time.h>
#include <unistd.h>
#include <utils/generic_vec.h>

/* msleep(): Sleep for the requested number of milliseconds. */
int msleep(long msec) {
    struct timespec ts;
    int res;

    if (msec < 0) {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

int supported_music_file(char *fullpath) {
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

FileState *file_state_init() {
    FileState *fs = malloc(sizeof(FileState));

    fs->state = FILE_STATE_FOUND;
    fs->filepath = 0;
    fs->filename = 0;
    fs->metadata.name = 0;
    fs->metadata.artist = 0;
    fs->metadata.album = 0;

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
        DIR *dir;
        struct dirent *entry;

        Vec *dirs = vec_init(sizeof(DIR *));
        Vec *paths = vec_init(sizeof(char *));

        dir = opendir(path);

        vec_push(dirs, &dir);
        vec_push(paths, &path);

        while (1) {

            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_name[0] == '.')
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

                sprintf(np, "%s/%s", np, entry->d_name);

                if (entry->d_type == DT_DIR) {
                    dir = opendir(np);

                    if (dir != NULL) {
                        vec_push(dirs, &dir);
                        char *name = entry->d_name;
                        vec_push(paths, &name);
                    } else {
                        dir = vec_get_ref(dirs, dirs->length - 1);
                    }
                }
                if (entry->d_type == DT_REG) {
                    if (supported_music_file(np)) {
                        char *name = strdup(np);

                        FileState *fs = file_state_init();
                        fs->filepath = name;
                        fs->filename = strdup(entry->d_name);

                        vec_push(files, &fs);
                    }

                    continue;
                }
            }

            if (paths->length <= 1)
                break;
            else {
                char *s = vec_pop_ref(paths);
                DIR *top = vec_pop_ref(dirs);
                closedir(top);
                dir = vec_get_ref(dirs, dirs->length - 1);
            }
        }

        for (int i = 0; i < dirs->length; i++) {
            closedir(vec_get_ref(dirs, i));
        }

        vec_free(dirs);
        vec_free(paths);
    }

    *out = files;
}

void scan_file(char *fullpath, MusicContext *music_ctx) {
    int stderr_fd = dup(STDERR_FILENO); // Save original stderr
    int devnull = open("/dev/null", O_WRONLY);
    dup2(devnull, STDERR_FILENO);
    close(devnull);

    TagLib_File *file = taglib_file_new(fullpath);

    // Restore stderr
    dup2(stderr_fd, STDERR_FILENO);
    close(stderr_fd);

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

void *scan_thread(void *arg) {
    ThreadContext *thread_ctx = arg;

    for (int i = 0; i < thread_ctx->data->length; i++) {
        FileState *file_state = vec_get_ref(thread_ctx->data, i);

        MusicContext mc = {0, 0, 0};

        pthread_mutex_lock(thread_ctx->mutex);

        scan_file(file_state->filepath, &mc);

        file_state->metadata = mc;

        thread_ctx->working_index = i;
        (*thread_ctx->processed)++;

        pthread_mutex_unlock(thread_ctx->mutex);
    }

    return NULL;
}

void *_wait_scan(void *args) {

    ScanContext *scan_ctx = (ScanContext *)args;

    for (int i = 0; i < scan_ctx->threads->length; i++) {
        pthread_t *thread = vec_get(scan_ctx->threads, i);
        pthread_join(*thread, NULL);
    }

    pthread_mutex_lock(&scan_ctx->mutex);
    scan_ctx->finalized = 1;
    pthread_mutex_unlock(&scan_ctx->mutex);

    return NULL;
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
    scan_ctx->threads = vec_init(sizeof(pthread_t *));
    scan_ctx->threads_ctx = vec_init(sizeof(ThreadContext *));
    scan_ctx->processed = 0;

    pthread_mutex_init(&scan_ctx->mutex, NULL);

    for (int i = 0; i < threads; i++) {
        ThreadContext *ctx = malloc(sizeof(ThreadContext));

        ctx->data = vec_get_ref(split_files, i);
        ctx->mutex = &scan_ctx->mutex;
        ctx->working_index = 0;
        ctx->processed = &scan_ctx->processed;

        pthread_t thread;

        pthread_create(&thread, NULL, scan_thread, (void *)ctx);

        vec_push(scan_ctx->threads_ctx, &ctx);
        vec_push(scan_ctx->threads, (void *)thread);
    }

    vec_free(split_files);

    pthread_create(&scan_ctx->joins_thread, NULL, _wait_scan, scan_ctx);

    return scan_ctx;
}

int lock_scan(ScanContext *scan_ctx) {
    pthread_mutex_lock(&scan_ctx->mutex);
    return scan_ctx->finalized;
};

void unlock_scan(ScanContext *scan_ctx) {
    pthread_mutex_unlock(&scan_ctx->mutex);
};

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

    pthread_mutex_destroy(&scan_ctx->mutex);
    free(scan_ctx);
}
