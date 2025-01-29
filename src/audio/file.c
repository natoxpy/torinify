#include <audio/audio.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int f_read_file(char *filename, uint8_t **file_data) {
    uint8_t *data = NULL;
    int size = 0;

    FILE *fp = fopen(filename, "r");

    if (fp != NULL) {
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
    } else {
        return -1;
    }

    *file_data = data;

    return size;
}
