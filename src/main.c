/// Example: how to use Torinify
#include <audio/audio.h>
#include <db/exec.h>
#include <errors/errors.h>
#include <libavutil/dict.h>
#include <media/media.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <taglib/tag_c.h>
#include <torinify/core.h>
#include <unistd.h>

struct ABC {
    int *a;
    char *b;
    long *c;
};

int main(int argc, char *argv[]) {
    int ret = T_SUCCESS;

    if ((ret = tf_init()) != T_SUCCESS ||
        (ret = tf_init_db("../sqlite.db", "../migrations")) != T_SUCCESS)
        goto end;

    M_register_source(tgc->sqlite3, "/home/toxpy/Music/server");

    M_scan(tgc->sqlite3);
end:
    tf_cleanup();

    if (ret != T_SUCCESS) {
        error_print_all();
        return -1;
    }

    return 0;
}
