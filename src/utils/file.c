#include <audio/audio.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

int f_read_file(char *filename, uint8_t **file_data) {
    uint8_t *data = NULL;
    int size = 0;

#ifdef _WIN32
    wchar_t wfilename[2048];
    MultiByteToWideChar(CP_UTF8, 0, filename, -1, wfilename, 2048);
    FILE *fp = _wfopen(wfilename, L"rb");
#elif __unix__
    FILE *fp = fopen(filename, "rb");
#endif

    if (fp == NULL) {
        return -3;
    }

    if (fseek(fp, 0L, SEEK_END) == 0) {
        long bufsize = ftell(fp);

        if (bufsize == -1)
            return -1;

        data = malloc(sizeof(uint8_t) * (bufsize + 1));

        if (fseek(fp, 0L, SEEK_SET) != 0)
            return -1;

        size = fread(data, sizeof(uint8_t), bufsize, fp);

        if (ferror(fp) != 0) {
            fputs("Error reading file\n", stderr);
        } else {
            data[size++] = '\0';
        }
    }

    fclose(fp);

    *file_data = data;

    return size;
}
