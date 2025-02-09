/// Example: how to use Torinify

#include <audio/audio.h>
#include <db/exec.h>
#include <errors/errors.h>
#include <libavutil/dict.h>
#include <media/media.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <storage/music.h>
#include <taglib/tag_c.h>
#include <termios.h>
#include <time.h>
#include <torinify/core.h>
#include <torinify/playback.h>
#include <torinify/search_engine.h>
#include <unistd.h>
#include <utils/levenshtein.h>

int main(int argc, char *argv[]) {
    int ret;

    if ((ret = tf_init()) != T_SUCCESS ||
        (ret = tf_init_db("../sqlite.db", "../migrations")) != T_SUCCESS)
        goto end;

    Music *music = NULL;

    struct timespec start, end;
    double elapsed_time;

    Vec *search_ctx = NULL;
    s_vec_search_context_init(tgc->sqlite3, &search_ctx);

    clock_gettime(CLOCK_MONOTONIC, &start);
    Vec *search_results = NULL;

    printf("--- Non-Threaded ---\n");
    s_process_search(search_ctx, "hello world", &search_results, 0.0);
    clock_gettime(CLOCK_MONOTONIC, &end);

    elapsed_time = (double)(end.tv_sec - start.tv_sec) +
                   (double)(end.tv_nsec - start.tv_nsec) / 1e9;

    printf("non threaded: %.3f seconds\n", elapsed_time);

    printf("--- Threaded ---\n");

    s_vec_search_result_free(search_results);

    clock_gettime(CLOCK_MONOTONIC, &start);
    search_results = NULL;
    s_process_search_multi(search_ctx, "hello world", &search_results, 0.0, 12);

    clock_gettime(CLOCK_MONOTONIC, &end);

    elapsed_time = (double)(end.tv_sec - start.tv_sec) +
                   (double)(end.tv_nsec - start.tv_nsec) / 1e9;

    printf("threaded: %.3f seconds\n", elapsed_time);

    printf("total results %d\n", search_results->length);

    // for (int i = search_results->length - 1; i > 0; i--) {
    //     SearchResult *res = vec_get(search_results, i);
    //     printf("(%f) %s\n", res->distance, res->title);
    // }

    s_vec_search_result_free(search_results);
    s_vec_search_context_free(search_ctx);

    // end = clock();

    // double time_duration = (double)(end - start) / CLOCKS_PER_SEC;

    // printf("%dms\n", (int)(time_duration * 1000));

    s_music_free(music);

end:
    tf_cleanup();

    error_print_all();
    if (ret != T_SUCCESS) {
        return -1;
    }

    return 0;
}
