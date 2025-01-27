/// Example: how to use Torinify

#include "audio/audio.h"
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <torinify/core.h>
#include <unistd.h>

// return negative for errors
long read_file(char *filename, uint8_t **file_data) {
    uint8_t *data = NULL;
    long size = 0;

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

int main(int argc, char *argv[]) {
    int ret = 0;
    // torinify_init();

    uint8_t *data;
    int size = read_file("m/IronLotus.wav", &data);

    AAudioContext *audio_ctx;
    if (a_audio_context_init(data, size, &audio_ctx) != 0)
        fprintf(stderr, "audio context init");

    AAudioVector *au_vec;
    if (a_audio_decode(audio_ctx, &au_vec) != 0)
        fprintf(stderr, "audio could not be decoded");

    a_audio_free_context(audio_ctx);
    free(data);

end:
    // torinify_cleanup();

    if (ret < 0) {
        fprintf(stderr, "Program failed!");
        return -1;
    }

    // sqlite3 *db;

    // sqlite3_open("../sqlite.db", &db);

    // sqlite3_close(db);

    // if (argc <= 1) {
    //     printf("usage: %s <file>\n", argv[0]);
    //     return 0;
    // }

    // cl_init();
    // cl_set_audio_file(argv[1]);
    // while (true) {
    //     int cmd;
    //     print_cmd();
    //     scanf("%d", &cmd);
    //     if (cmd == 0) {
    //         break;
    //     }
    //     switch (cmd) {
    //     case 1:
    //         printf("current time: %f / %f\n",
    //                (float)cl_get_current_time() / 1000,
    //                (float)cl_get_duration() / 1000);
    //         break;
    //     case 2:
    //         printf("paused: %s \n", cl_get_paused() ? "true" : "false");
    //         break;
    //     case 3:
    //         printf("volume: %f\n", cl_get_volume());
    //         break;
    //     case 4:
    //         cl_pause();
    //         printf("paused: %s \n", cl_get_paused() ? "true" : "false");
    //         break;
    //     case 5:
    //         cl_play();
    //         printf("paused: %s \n", cl_get_paused() ? "true" : "false");
    //         break;
    //     case 6: {
    //         unsigned int ct = cl_get_current_time();
    //         unsigned int dur = cl_get_duration();
    //         if (dur >= ct + 5 * 1000) {
    //             cl_set_current_time(ct + 5 * 1000);
    //             printf("current time: %f / %f\n",
    //                    (float)cl_get_current_time() / 1000,
    //                    (float)cl_get_duration() / 1000);
    //         } else
    //             printf("Cannot advance 5s\n");

    //         break;
    //     }
    //     case 7: {
    //         unsigned int ct = cl_get_current_time();

    //         if (ct >= 5 * 1000) {
    //             ct -= 5 * 1000;
    //             cl_set_current_time(ct);

    //             printf("current time: %f / %f\n",
    //                    (float)cl_get_current_time() / 1000,
    //                    (float)cl_get_duration() / 1000);
    //         } else {

    //             printf("Cannot go back 5s\n");
    //         }

    //         break;
    //     }

    //     case 8: {

    //         float vol = cl_get_volume();

    //         if (vol > 1.0f - 0.1f) {
    //             vol = 1.0f;
    //         } else {
    //             vol += 0.1f;
    //         }

    //         cl_set_volume(vol);

    //         printf("volume: %f\n", cl_get_volume());
    //         break;
    //     }

    //     case 9: {
    //         float vol = cl_get_volume();

    //         if (vol <= 0.1) {

    //             vol = 0.0;
    //         } else {
    //             vol -= 0.1;
    //         }

    //         cl_set_volume(vol);

    //         printf("volume: %f\n", cl_get_volume());
    //         break;
    //     }
    //     }
    // }

    // cl_cleanup();

    return 0;
}
