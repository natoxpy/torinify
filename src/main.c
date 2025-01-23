/// Example: how to use Torinify

#include <stdbool.h>
#include <stdio.h>
#include <torinify_core.h>
#include <unistd.h>

void print_cmd() {
    printf("\n0: exit\n");
    printf("1: print current time\n");
    printf("2: print paused state\n");
    printf("3: print volume\n");
    printf("4: pause\n");
    printf("5: play\n");
    printf("6: 5s+\n");
    printf("7: 5s-\n");
    printf("8: vol 10%%+\n");
    printf("9: vol 10%%-\n");
}

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf("usage: %s <file>\n", argv[0]);
        return 0;
    }

    cl_init();
    cl_set_audio_file(argv[1]);
    while (true) {
        int cmd;
        print_cmd();
        scanf("%d", &cmd);
        if (cmd == 0) {
            break;
        }
        switch (cmd) {
        case 1:
            printf("current time: %f / %f\n",
                   (float)cl_get_current_time() / 1000,
                   (float)cl_get_duration() / 1000);
            break;
        case 2:
            printf("paused: %s \n", cl_get_paused() ? "true" : "false");
            break;
        case 3:
            printf("volume: %f\n", cl_get_volume());
            break;
        case 4:
            cl_pause();
            printf("paused: %s \n", cl_get_paused() ? "true" : "false");
            break;
        case 5:
            cl_play();
            printf("paused: %s \n", cl_get_paused() ? "true" : "false");
            break;
        case 6: {
            unsigned int ct = cl_get_current_time();
            unsigned int dur = cl_get_duration();
            if (dur >= ct + 5 * 1000) {
                cl_set_current_time(ct + 5 * 1000);
                printf("current time: %f / %f\n",
                       (float)cl_get_current_time() / 1000,
                       (float)cl_get_duration() / 1000);
            } else
                printf("Cannot advance 5s\n");

            break;
        }
        case 7: {
            unsigned int ct = cl_get_current_time();

            if (ct >= 5 * 1000) {
                ct -= 5 * 1000;
                cl_set_current_time(ct);

                printf("current time: %f / %f\n",
                       (float)cl_get_current_time() / 1000,
                       (float)cl_get_duration() / 1000);
            } else {

                printf("Cannot go back 5s\n");
            }

            break;
        }

        case 8: {

            float vol = cl_get_volume();

            if (vol > 1.0f - 0.1f) {
                vol = 1.0f;
            } else {
                vol += 0.1f;
            }

            cl_set_volume(vol);

            printf("volume: %f\n", cl_get_volume());
            break;
        }

        case 9: {
            float vol = cl_get_volume();

            if (vol <= 0.1) {

                vol = 0.0;
            } else {
                vol -= 0.1;
            }

            cl_set_volume(vol);

            printf("volume: %f\n", cl_get_volume());
            break;
        }
        }
    }

    cl_cleanup();

    return 0;
}
