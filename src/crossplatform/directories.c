#include <crossplatform/directories.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <locale.h>
#include <wchar.h>
#include <windows.h>

Directory *directory_open(char *path) {
    Directory *directory = malloc(sizeof(Directory));

    if (directory == NULL)
        return NULL;

    directory->_file = malloc(sizeof(DirectoryEntry));
    if (directory->_file == NULL)
        return NULL;

    WIN32_FIND_DATAW file;
    HANDLE find = NULL;
    wchar_t wpath[2048];
    int result = MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, 2048);

    // Check for conversion errors
    if (result == 0) {
        DWORD error = GetLastError();
        wprintf("MultiByteToWideChar failed with error code %lu\n", error);
    }

    wcscat(wpath, L"\\*");

    // wprintf(L"path looking into: %ls\n", wpath);

    if ((find = FindFirstFileW(wpath, &file)) == INVALID_HANDLE_VALUE)
        return NULL;

    directory->_raw = find;

    // Convert the wide-character file name to UTF-8
    WideCharToMultiByte(CP_UTF8, 0, file.cFileName, -1, directory->_file->name,
                        sizeof(directory->_file->name), NULL, NULL);

    if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        directory->_file->type = ENTRY_TYPE_DIRECTORY;
    else
        directory->_file->type = ENTRY_TYPE_FILE;

    return directory;
}

DirectoryEntry *directory_read(Directory *dir) {
    HANDLE *find = dir->_raw;

    if (dir->_file->_raw != NULL) {
        dir->_file->_raw = NULL;
        return dir->_file;
    }

    WIN32_FIND_DATAW file;
    int ret = FindNextFileW(find, &file);
    if (ret == 0)
        return NULL;

    WideCharToMultiByte(CP_UTF8, 0, file.cFileName, -1, dir->_file->name,
                        sizeof(dir->_file->name), NULL, NULL);

    if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        dir->_file->type = ENTRY_TYPE_DIRECTORY;
    else
        dir->_file->type = ENTRY_TYPE_FILE;

    return dir->_file;
}

void directory_close(Directory *dir) {
    if (dir == NULL)
        return;

    FindClose(dir->_raw);
    free(dir->_file);
    free(dir);
}
#elif __unix__
#include <dirent.h>
#include <string.h>

Directory *directory_open(char *path) {
    Directory *directory = malloc(sizeof(Directory));

    if (directory == NULL)
        return NULL;

    DIR *dir = opendir(path);
    directory->_raw = dir;
    directory->_file = malloc(sizeof(DirectoryEntry));

    if (directory->_file == NULL)
        return NULL;

    return directory;
}

DirectoryEntry *directory_read(Directory *directory) {
    DIR *dir = directory->_raw;

    struct dirent *entry;

    if ((entry = readdir(dir)) == NULL)
        return NULL;

    memcpy(directory->_file->name, entry->d_name, 256);

    if (entry->d_type == DT_DIR)
        directory->_file->type = ENTRY_TYPE_DIRECTORY;
    else
        directory->_file->type = ENTRY_TYPE_FILE;

    return directory->_file;
}

void directory_close(Directory *dir) {
    if (dir == NULL)
        return;

    closedir(dir->_raw);
    free(dir->_file);
    free(dir);
}

#endif
