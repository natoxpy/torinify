#include <db/exec.h>
#include <errors/errors.h>
#include <limits.h>
#include <media/media.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <taglib/tag_c.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#ifdef _WIN32
#include <windows.h>
#define realpath(N, R) _fullpath((R), (N), PATH_MAX)
#else
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#endif

/// char fullpath[PATH_MAX];
T_CODE M_realpath(const char *filepath, char *fullpath) {

    if (realpath(filepath, fullpath) == NULL) {
        error_log("Could not get realpath of \"%s\"", filepath);
        return T_FAIL;
    }

    return T_SUCCESS;
}

T_CODE M_scan(sqlite3 *db) {
    int ret = T_SUCCESS;
    MediaSources *media_srcs = NULL;
    if ((ret = M_get_sources(db, &media_srcs)) != T_SUCCESS)
        goto cleanup;

    for (int i = 0; i < media_srcs->size; i++) {
        ret = M_scan_dir(db, media_srcs->sources[i].source,
                         media_srcs->sources[i].id);

        if (ret != T_SUCCESS) {
            error_log("Could not scan source \"%s\" with id \"%d\" ",
                      media_srcs->sources[i].source, media_srcs->sources[i].id);

            goto cleanup;
        }
    }

cleanup:
    M_free_sources(media_srcs);

    return ret;
}

T_CODE M_scan_dir(sqlite3 *db, char *dirpath, int source_id) {
    int ret = T_SUCCESS;

    char path[PATH_MAX];

    if ((ret = M_realpath(dirpath, path)) != T_SUCCESS)
        return ret;

#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    // Append the wildcard to search for all files and directories
    char search_path[PATH_MAX];
    snprintf(search_path, PATH_MAX, "%s\\*", path);

    hFind = FindFirstFile(search_path, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        error_log("Could not open directory \"%s\"", path);
        return T_FAIL;
    }

    do {
        // Skip '.' and '..' entries
        if (strcmp(findFileData.cFileName, ".") == 0 ||
            strcmp(findFileData.cFileName, "..") == 0)
            continue;

        char entry_path[PATH_MAX];
        snprintf(entry_path, PATH_MAX, "%s\\%s", path, findFileData.cFileName);

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            ret = M_scan_dir(db, entry_path, source_id);
            if (ret != T_SUCCESS)
                break;
        } else {
            if (M_supported_music_file(entry_path)) {
                ret = M_scan_file(db, entry_path, source_id);
                if (ret != T_SUCCESS) {
                    error_log("Could not scan file \"%s\"", entry_path);
                    break;
                }
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
#else
    DIR *dp = opendir(path);
    struct dirent *entry;

    if (dp == NULL) {
        error_log("Could not open path \"%s\"", path);
        return T_FAIL;
    }

    while ((entry = readdir(dp)) != NULL) {
        // Ignore hidden folders of files and '..' or '.'
        if (entry->d_name[0] == '.')
            continue;

        char entry_path[PATH_MAX];
        snprintf(entry_path, PATH_MAX, "%s/%s", path, entry->d_name);

        if (entry->d_type == DT_DIR) {
            ret = M_scan_dir(db, entry_path, source_id);
            if (ret != T_SUCCESS)
                break;
        } else if (entry->d_type == DT_REG &&
                   M_supported_music_file(entry_path)) {
            ret = M_scan_file(db, entry_path, source_id);
            if (ret != T_SUCCESS) {
                error_log("Could not scan file \"%s\"", entry_path);
                break;
            }
        }
    }

    closedir(dp);
#endif

    return ret;
}

T_CODE M_scan_file(sqlite3 *db, char *filepath, int source) {
    TagLib_File *taglib_file = NULL;
    int ret;
    char fullpath[PATH_MAX];

    if (realpath(filepath, fullpath) == NULL) {
        error_log("Could not get realpath of \"%s\"", filepath);
        ret = T_FAIL;
        goto cleanup;
    }

    MusicRow *music_row;

    ret = DB_query_music_single(
        db, &music_row,
        (SQLQuery){.by = DB_QUERY_BY_FULLPATH, {.fullpath = fullpath}});

    if (ret != TDB_SUCCESS || music_row != NULL) {
        DB_music_row_free(music_row);
        goto cleanup;
    }

    taglib_file = taglib_file_new(filepath);
    TagLib_Tag *taglib_tag = taglib_file_tag(taglib_file);

    char *title = taglib_tag_title(taglib_tag);
    char *album = taglib_tag_album(taglib_tag);
    char *artist = taglib_tag_artist(taglib_tag);
    char *genre = taglib_tag_genre(taglib_tag);
    unsigned int year = taglib_tag_year(taglib_tag);

    MetadataRow metadata = {-1, artist, genre, year};

    if ((ret = DB_insert_music_repl(db, title, &metadata, fullpath, source,
                                    -1)) != TDB_SUCCESS) {
        ret = T_FAIL;
        goto cleanup;
    }

    ret = T_SUCCESS;
cleanup:

    if (taglib_file)
        taglib_file_free(taglib_file);

    return ret;
}
