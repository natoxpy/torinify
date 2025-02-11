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
        (ret = tf_init_db("../../sqlite.db", "../../migrations")) != T_SUCCESS)
        goto end;

    Music *music = NULL;

    Vec *search_ctx = NULL;
    s_vec_search_context_init(tgc->sqlite3, &search_ctx);

    Vec *search_results = NULL;

    s_process_search_multi(search_ctx, "grownups", &search_results, 0.0, 12);

    SearchResult *best = vec_get(search_results, 0);

    tf_set_src("/home/toxpy/Music/server/K_DA/MORE (2020)/K+DA - MORE - 01 - "
               "MORE.mp3");
    tf_play();

    scanf("hello");

    s_vec_search_result_free(search_results);
    s_vec_search_context_free(search_ctx);

    s_music_free(music);

end:
    tf_cleanup();

    error_print_all();
    if (ret != T_SUCCESS) {
        return -1;
    }

    return 0;
}
