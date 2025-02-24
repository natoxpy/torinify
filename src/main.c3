#include "db/exec.h"
#include "db/helpers.h"
#include "db/tables.h"
#include "media/scan.h"
#include "utils/generic_vec.h"
#include <stdio.h>
#include <string.h>
#include <torinify/core.h>
#include <torinify/search_engine.h>

#ifndef _ENGINE_OS_SPECIFIC_MAIN_C
#define _ENGINE_OS_SPECIFIC_MAIN_C

void hide_cursor() { printf("\033[?25l"); }
void show_cursor() { printf("\033[?25h"); }

#define ARROW_LEFT 0
#define ARROW_RIGHT 1
#define ARROW_UP 2
#define ARROW_DOWN 3

#define KEY_STANDARD 0
#define KEY_SPECIAL 1
#define KEY_ARROW 2

typedef struct {
    uint32_t keytype;
    union {
        char standard;
        char arrow;
        char *special;
    } ch;
} Key;

int inline static is_arrow(char c) {
    return c == ARROW_LEFT || c == ARROW_RIGHT || c == ARROW_UP ||
           c == ARROW_DOWN;
}

int inline static is_enter(Key key) {
    return key.keytype == KEY_SPECIAL && strcmp(key.ch.special, "Enter") == 0;
}

/// Checks if input is standard and `y`
int inline static is_accepted(Key key) {
    return key.keytype == KEY_STANDARD && key.ch.standard == 'y';
}

/// Checks if input is standard and `n`
int inline static is_rejected(Key key) {
    return key.keytype == KEY_STANDARD && key.ch.standard == 'y';
}

int inline static is_esc(Key key) {
    return key.keytype == KEY_SPECIAL && strcmp(key.ch.special, "Esc") == 0;
}

#ifdef __unix__

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

void oss_print_with_background_white(char *text) {
    printf("\033[47;30m%s\033[0m", text);
}

void oss_get_terminal_size(int *width, int *height) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        if (width != NULL)
            *width = w.ws_col;
        if (height != NULL)
            *height = w.ws_row;
    } else {
        if (width != NULL)
            *width = 80;
        if (height != NULL)
            *height = 24;
    }
}

/// OS Specific Setup
void oss_setup() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    // Disable canonical mode and echo
    term.c_lflag &= ~(ICANON | ECHO);
    // Disable ctrl+c
    term.c_cc[VINTR] = 0;
    // Apply the settings
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

char oss_noblock_readch() { // Set STDIN to non-blocking mode
    char c = 0;
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    // Attempt to read a character
    if (read(STDIN_FILENO, &c, 1) <= 0)
        c = 0; // No input available

    // Restore original blocking mode
    fcntl(STDIN_FILENO, F_SETFL, flags);

    return c;
}

char oss_readch() {
    char c = 0;

    // Read a single character (blocking)
    while (read(STDIN_FILENO, &c, 1) <= 0) {
    };

    if (c == '\033') {
        oss_noblock_readch();
        switch (oss_noblock_readch()) {
        case 'A':
            return ARROW_UP;
        case 'B':
            return ARROW_DOWN;
        case 'C':
            return ARROW_RIGHT;
        case 'D':
            return ARROW_LEFT;
        }
    }

    return c;
}

void oss_cleanup() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);

    // Restore canonical mode and echo
    term.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void inline static oss_clean_screen() { printf("\033[H\033[J"); }

int oss_path_exists(char *path) {
    struct stat sb;
    return stat(path, &sb) == 0;
}

#endif
#ifdef _WIN32

#endif

#endif

#ifndef _ENGINE_MAIN_C
#define _ENGINE_MAIN_C
#include <stdint.h>

typedef struct {
    char *str;
    uint32_t len;
    uint32_t cap;
} Text;

/// Waits for input
Key readkey() {
    Key key = {.keytype = KEY_SPECIAL, {.standard = 0}};
    char c = oss_readch();

    if (is_arrow(c)) {
        key.keytype = KEY_ARROW;
        key.ch.arrow = c;
    } else if (c == '\n') {
        key.keytype = KEY_SPECIAL;
        key.ch.special = "Enter";
    } else if (c == 27) {
        key.keytype = KEY_SPECIAL;
        key.ch.special = "Esc";
    } else {
        key.keytype = KEY_STANDARD;
        key.ch.standard = c;
    }

    return key;
}

void component_render_inputbox(Text *text) {
    printf("%s\033[48;5;15m \033[0m\n", text->str);
}

void component_render_sources() {
    Vec *sources;
    DB_query_source_all(tgc->sqlite3, &sources);

    printf("============= \n");
    printf(" | SOURCES |  \n");

    for (int i = 0; i < sources->length; i++) {
        MediaSourceRow *row = vec_get_ref(sources, i);
        printf(" %d. %s\n", i + 1, row->path);
    }

    printf("============= \n");

    dbt_source_vec_rows_free(sources);
}

/// @return number of sources rendered
int component_render_sources_with_selection(int selected) {
    Vec *sources;
    DB_query_source_all(tgc->sqlite3, &sources);

    printf("============= \n");
    printf(" | SOURCES |  \n");

    int rendered = 0;

    for (int i = 0; i < sources->length; i++) {
        MediaSourceRow *row = vec_get_ref(sources, i);

        char str[256];
        sprintf(str, " %d. %s\n", i + 1, row->path);

        if (selected == i) {
            oss_print_with_background_white(str);
        } else {
            printf("%s", str);
        }

        rendered++;
    }

    printf("============= \n");

    dbt_source_vec_rows_free(sources);

    return rendered;
}

void setup() {
    hide_cursor();
    oss_setup();
    tf_init();
    tf_init_db("../../sqlite.db");

    return;
}

void inline static clean_screen() { oss_clean_screen(); }

void collect_text(Text *text, Key key) {
    if (key.keytype != KEY_STANDARD) {
        return;
    }

    if (key.ch.standard == 127) {
        if (text->len > 0)
            text->len--;

        text->str[text->len] = '\0';
    } else {
        if (text->len >= text->cap) {
            printf("too large\n");
            return;
        }

        text->str[text->len] = key.ch.standard;
        text->str[text->len + 1] = '\0';

        text->len++;
    }
}

void cleanup() {
    show_cursor();
    oss_cleanup();
    tf_cleanup();
    return;
}

#endif

#ifndef _APP_MAIN_C
#define _APP_MAIN_C
typedef struct {
    int page;
    int subpage;
    int selected;
    int max_selected;
    Text logmsg;
    Text search_query;
    Vec *search_results;
    Text general_use;
} AppContext;

AppContext init_app() {
    AppContext app_ctx = {
        .page = 1,
        .selected = 0,
        .max_selected = 0,
        .logmsg = {.len = 0, .cap = 1012, .str = malloc(sizeof(char *) * 1012)},
        .search_query = {.len = 0,
                         .cap = 255,
                         .str = malloc(sizeof(char *) * 255)},
        .general_use = {.len = 0,
                        .cap = 1012,
                        .str = malloc(sizeof(char *) * 1012)},
        .search_results = NULL,

    };

    app_ctx.search_query.str[0] = '\0';

    return app_ctx;
}

int handle_arrow_key(Key key, AppContext *app_ctx) {
    if (key.keytype != KEY_ARROW)
        return 0;

    switch (key.ch.arrow) {
    case ARROW_UP:

        app_ctx->selected--;

        if (app_ctx->selected < 0) {
            app_ctx->selected = app_ctx->max_selected;
        }

        break;
    case ARROW_DOWN:
        app_ctx->selected++;

        if (app_ctx->selected > app_ctx->max_selected) {
            app_ctx->selected = 0;
        }

        break;
    }
    return 1;
}

void text_empty(Text *text) {
    text->len = 0;
    text->str[0] = '\0';
}

void text_append(Text *text, char *str) {
    int extralen = strlen(str);
    if (text->len + extralen > text->cap) {
        return;
    }

    memcpy(text->str + text->len, str, extralen);
    text->len += extralen;
}

void text_copy(Text *text, char *str) {
    text->len = strlen(str);
    if (text->len > text->cap) {
        text_empty(text);
        return;
    }

    memcpy(text->str, str, text->len);

    text->str[text->len] = '\0';
}

int home_page();
int search_page(AppContext *app);
int media_page(AppContext *app);
#endif

void app_cleanup() {
    cleanup();
    clean_screen();
    return;
}

int main() {
    setup();
    atexit(app_cleanup);

    AppContext app_ctx = init_app();

    int redirect_to = 0;

    while (1) {
        clean_screen();

        printf("torinify - ");

        if (app_ctx.logmsg.len != 0) {
            printf("%s - ", app_ctx.logmsg.str);
            text_empty(&app_ctx.logmsg);
        }

        switch (app_ctx.page) {
        case 1:
            printf("Home\n");
            redirect_to = home_page();

            break;

        case 2:
            printf("Search: ");
            redirect_to = search_page(&app_ctx);
            break;

        case 3:
            printf("Media");
            redirect_to = media_page(&app_ctx);
            break;

        default:
            text_copy(&app_ctx.logmsg, "Page Not Found");
            redirect_to = 1;
        }

        if (redirect_to != 0 && redirect_to != app_ctx.page) {
            text_empty(&app_ctx.general_use);
            app_ctx.selected = 0;
            app_ctx.page = redirect_to;
        }

        if (redirect_to == -1)
            break;
    }

    free(app_ctx.search_query.str);
    free(app_ctx.general_use.str);
    free(app_ctx.logmsg.str);

    s_vec_search_result_free(app_ctx.search_results);

    return 0;
}

/// === HOME PAGE ===

int home_page() {
    printf("[Esc] Exit\n");
    printf("[1] Search\n");
    printf("[2] Scan\n");

    Key key = readkey();
    if (is_esc(key))
        return -1;

    if (key.keytype == KEY_STANDARD) {
        switch (key.ch.standard) {
        case '1':
            return 2;
        case '2':
            return 3;
        default:
            return -2;
        }
    }

    return 0;
}

/// === SEARCH PAGE ===

int search_page(AppContext *app) {
    component_render_inputbox(&app->search_query);
    printf("[Esc] Home\n");

    int max = 10;
    oss_get_terminal_size(NULL, &max);
    max = max - 3;

    if (app->search_results) {

        for (int i = 0; i < app->search_results->length; i++) {
            if (i >= max)
                break;

            SearchResult *result = vec_get(app->search_results, i);

            printf("- %s\n", result->title);
        }
    }

    Key key = readkey();
    if (is_esc(key))
        return 1;

    collect_text(&app->search_query, key);

    s_vec_search_result_free(app->search_results);
    tf_search(app->search_query.str, 0.2, &app->search_results);

    return 0;
}

/// === MEDIA PAGE ===

// TODO: finish this page
int media_add_source_subpage(AppContext *app_ctx) {
    printf(" - New Directory: ");
    component_render_inputbox(&app_ctx->general_use);
    printf("[Esc] Return\n");

    component_render_sources();

    Key key = readkey();
    if (is_esc(key)) {
        return -1;
    }

    if (is_enter(key)) {

        if (!oss_path_exists(app_ctx->general_use.str)) {
            text_copy(&app_ctx->logmsg, "directory doesn't exist");
            return 0;
        }

        while (1) {
            printf("accept adding new source ([Y]/n)?\n");

            key = readkey();

            if (is_enter(key) || is_accepted(key)) {
                char *realpath = malloc(sizeof(char *) * 255);
                dbh_realpath(app_ctx->general_use.str, realpath);
                text_append(&app_ctx->logmsg, "Added new source ");
                text_append(&app_ctx->logmsg, realpath);

                int ret = DB_insert_source_row(tgc->sqlite3, realpath);

                free(realpath);
                return -1;
            }

            if (is_rejected(key)) {
                text_copy(&app_ctx->logmsg, "did not added new media source");
                return -1;
            }

            text_copy(&app_ctx->logmsg, "only input valid characters");
        }
    }

    collect_text(&app_ctx->general_use, key);

    return 0;
}

int media_scan_media_subpage(AppContext *app_ctx) {
    printf(" - Scan ");

    printf("\n[Esc] Return - ");
    printf("Select a media source, then [enter] to continue\n");

    app_ctx->max_selected =
        component_render_sources_with_selection(app_ctx->selected) - 1;

    Key key = readkey();
    if (is_esc(key))
        return -1;

    if (key.keytype == KEY_ARROW) {
        handle_arrow_key(key, app_ctx);
        return 0;
    }

    if (!is_enter(key)) {
        return -1;
    }

    printf(" - Scan Options - \n");
    printf("[1] Only scan for new music (default)\n");
    printf("[2] Scan new and existing music\n");

    do {
        key = readkey();
        if (is_esc(key))
            return -1;
        if (is_enter(key) || key.keytype == KEY_STANDARD)
            break;

    } while (1);

    Vec *sources;
    Vec *sources_for_scan = vec_init(sizeof(char *));
    DB_query_source_all(tgc->sqlite3, &sources);

    MediaSourceRow *row = vec_get_ref(sources, app_ctx->selected);
    vec_push(sources_for_scan, &row->path);

    clean_screen();

    int threads = 15;
    ScanContext *scan_ctx = start_scan(tgc->sqlite3, sources_for_scan, threads);

    int offset = 0;
    int width;

    int final_rerender = 0;
    int ret = 0;

    while ((ret = lock_scan(scan_ctx)) == 0 || final_rerender) {
        if (ret == 1)
            final_rerender = 0;

        clean_screen();
        oss_get_terminal_size(&width, NULL);

        double dk =
            (double)scan_ctx->processed / (double)scan_ctx->data->length;

        char buff[255];
        sprintf(buff, "(%d / %d)", scan_ctx->processed, scan_ctx->data->length);
        printf("%s[", buff);

        int uwidth = width - (2 + strlen(buff));

        for (int i = 0; i < uwidth; i++) {
            double bk = (double)i / (double)uwidth;

            if (bk >= dk)
                printf(" ");
            else
                printf("|");
        }

        printf("]\n");
        unlock_scan(scan_ctx);
    }

    finalize_scan(scan_ctx);

    dbt_source_vec_rows_free(sources);
    vec_free(sources_for_scan);

    printf("Press any key to continue\n");

    readkey();

    return -1;
}

int media_rescan_media_subpage(AppContext *app_ctx) {
    printf("\n[Esc] Return\n");

    Key key = readkey();
    if (is_esc(key)) {
        return -1;
    }

    return 0;
}

int media_page(AppContext *app_ctx) {

    if (app_ctx->subpage > 0) {
        int newpage = 0;

        switch (app_ctx->subpage) {
        case 1:
            newpage = media_add_source_subpage(app_ctx);
            break;
        case 2:
            newpage = media_scan_media_subpage(app_ctx);
            break;
        case 3:
            newpage = media_rescan_media_subpage(app_ctx);
            break;
        }

        if (newpage != 0) {
            app_ctx->subpage = newpage;
            text_empty(&app_ctx->general_use);
        }

        return 0;
    }

    printf("\n[Esc] Return | [m] Add Media Source | [r] Scan Selected for new "
           "media | [R] "
           "Rescan for all media | [d] Delete Selected Media Source\n");

    component_render_sources();

    Key key = readkey();
    if (is_esc(key))
        return 1;

    if (key.keytype != KEY_STANDARD)
        return 0;

    switch (key.ch.standard) {
    case 'm':
        app_ctx->subpage = 1;
        break;
    case 'r':
        app_ctx->subpage = 2;
        break;
    case 'R':
        app_ctx->subpage = 3;
        break;
    }

    return 0;
}
