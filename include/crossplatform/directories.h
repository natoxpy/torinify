#ifndef _CROSSPLATFORM_DIRECTORIES_H
#define _CROSSPLATFORM_DIRECTORIES_H

#define ENTRY_TYPE_DIRECTORY 1
#define ENTRY_TYPE_FILE 2

typedef struct {
    void *_raw;
    int type;
    char name[1012];
} DirectoryEntry;

typedef struct {
    void *_raw;
    DirectoryEntry *_file;
} Directory;

Directory *directory_open(char *path);
DirectoryEntry *directory_read(Directory *dir);
void directory_close(Directory *dir);

#endif