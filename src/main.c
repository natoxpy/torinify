#include "media/scan.h"
#include "torinify/playback.h"
#include "utils/generic_vec.h"
#include <db/exec.h>
#include <db/exec/music_table.h>
#include <db/helpers.h>
#include <db/sql.h>
#include <db/tables.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <torinify/core.h>
#include <torinify/search_engine.h>

#ifndef _ENGINE_OS_SPECIFIC_MAIN_C
#define _ENGINE_OS_SPECIFIC_MAIN_C

void print_with_background_white(char *text) {
    printf("\033[47;30m%s\033[0m", text);
}

#define ARROW_LEFT 1
#define ARROW_RIGHT 2
#define ARROW_UP 3
#define ARROW_DOWN 4

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

int inline static no_input(Key key) {
    return key.keytype == KEY_STANDARD && key.ch.standard == 0;
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

int inline static is_space(Key key) {
    return key.keytype == KEY_SPECIAL && strcmp(key.ch.special, "Space") == 0;
}

#ifdef __unix__

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

void oss_hide_cursor() { printf("\033[?25l"); }
void oss_show_cursor() { printf("\033[?25h"); }

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

#include <windows.h>

void oss_print_with_background_white(char *text) {
    printf("\033[47;30m%s\033[0m", text);
}

int oss_path_exists(char *path) {
    DWORD dwAttrib = GetFileAttributes(path);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
            (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void oss_hide_cursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info = {0};
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

void oss_show_cursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info = {0};
    info.bVisible = TRUE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

void oss_get_terminal_size(int *width, int *height) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int columns, rows;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    if (width != NULL)
        *width = columns;
    if (height != NULL)
        *height = rows;
}

char oss_noblock_readch() {
    HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);
    DWORD dwMode;
    GetConsoleMode(hConsole, &dwMode);
    SetConsoleMode(hConsole,
                   dwMode & ~ENABLE_LINE_INPUT &
                       ~ENABLE_ECHO_INPUT); // Disable line input and echo

    INPUT_RECORD ir;
    DWORD dwNumRead;
    char ch = 0;

    if (PeekConsoleInput(hConsole, &ir, 1, &dwNumRead) && dwNumRead > 0) {
        if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown) {
            ch = ir.Event.KeyEvent.uChar.AsciiChar;

            // Check for arrow keys by looking at wVirtualKeyCode
            switch (ir.Event.KeyEvent.wVirtualKeyCode) {
            case VK_UP:
                ch = ARROW_UP;
                break;
            case VK_DOWN:
                ch = ARROW_DOWN;
                break;
            case VK_LEFT:
                ch = ARROW_LEFT;
                break;
            case VK_RIGHT:
                ch = ARROW_RIGHT;
                break;
            }
        }

        // Remove the input from the buffer
        FlushConsoleInputBuffer(hConsole);
    }

    return ch;
}

char oss_readch() {
    char ch = 0;
    while ((ch = oss_noblock_readch()) == 0) {
    };

    return ch;
}

void oss_setup() {}
void inline static oss_clean_screen() { printf("\033[H\033[J"); }
void oss_cleanup() {}

#endif

#endif

#ifndef _ENGINE_MAIN_C
#define _ENGINE_MAIN_C
#include <stdint.h>

void hide_cursor() { oss_hide_cursor(); }
void show_cursor() { oss_show_cursor(); }

typedef struct {
    char *str;
    uint32_t len;
    uint32_t cap;
} Text;

/// Non-blocking input
Key readkey_nb() {
    Key key = {.keytype = KEY_STANDARD, {.standard = 0}};
    char c = oss_noblock_readch();

    if (c == 0)
        return key;

    if (is_arrow(c)) {
        key.keytype = KEY_ARROW;
        key.ch.arrow = c;
    } else if (c == '\r' || c == '\n') {
        key.keytype = KEY_SPECIAL;
        key.ch.special = "Enter";
    } else if (c == 27) {
        key.keytype = KEY_SPECIAL;
        key.ch.special = "Esc";
    } else if (c == 32) {
        key.keytype = KEY_SPECIAL;
        key.ch.special = "Space";
    } else {
        key.keytype = KEY_STANDARD;
        key.ch.standard = c;
    }

    return key;
}

/// Waits for input
Key readkey() {
    Key key = {.keytype = KEY_SPECIAL, {.standard = 0}};
    char c = oss_readch();

    if (is_arrow(c)) {
        key.keytype = KEY_ARROW;
        key.ch.arrow = c;
    } else if (c == '\r' || c == '\n') {
        key.keytype = KEY_SPECIAL;
        key.ch.special = "Enter";
    } else if (c == 27) {
        key.keytype = KEY_SPECIAL;
        key.ch.special = "Esc";
    } else if (c == 32) {
        key.keytype = KEY_SPECIAL;
        key.ch.special = "Space";
    } else {
        key.keytype = KEY_STANDARD;
        key.ch.standard = c;
    }

    return key;
}

void component_render_inputbox(Text *text) {
    printf("'%s\033[48;5;15m \033[0m'\n", text->str);
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

T_CODE setup() {
    int ret = T_SUCCESS;
    hide_cursor();
    oss_setup();
    ret = tf_init();
    if (ret != T_SUCCESS)
        return ret;

    ret = tf_init_db("../../sqlite.db");
    if (ret != T_SUCCESS)
        return ret;

    return ret;
}

void inline static clean_screen() { oss_clean_screen(); }

void collect_text(Text *text, Key key) {
    if (is_space(key)) {
        if (text->len >= text->cap) {
            printf("too large\n");
            return;
        }

        text->str[text->len] = ' ';
        text->str[text->len + 1] = '\0';
        text->len++;
    }

    if (key.keytype != KEY_STANDARD) {
        return;
    }

    if (key.ch.standard == 127 || key.ch.standard == '\b') {
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
    Queue *core_q = pb_q_alloc();
    pb_add_q(tgc->playback, core_q);

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
    default:
        return 0;
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
int playback_page(AppContext *app);
#endif

Queue *get_core_queue() { return vec_get_ref(tgc->playback->queues, 0); }

void app_cleanup() {
    cleanup();
    clean_screen();
#ifdef _WIN32
#elif __unix__
    freopen("/dev/tty", "w", stderr);
#endif
    return;
}

int main() {

#ifdef _WIN32
    int _ = freopen("null", "w", stderr);
#elif __unix__
    // freopen("/dev/null", "w", stderr);
#endif

    atexit(app_cleanup);
    if (setup() != T_SUCCESS) {
        error_log("unable to start due to setup fail\n");
        error_print_all();
        return -1;
    }

    AppContext app_ctx = init_app();

    int redirect_to = 0;

    while (1) {
        clean_screen();

        printf("torinify - ");

        if (app_ctx.logmsg.len != 0) {
            printf("%s - ", app_ctx.logmsg.str);
            // text_empty(&app_ctx.logmsg);
        }

        switch (app_ctx.page) {
        case 1:
            printf("Home\n");
            redirect_to = home_page();

            break;

        case 2:
            redirect_to = search_page(&app_ctx);
            break;

        case 3:
            printf("Media");
            redirect_to = media_page(&app_ctx);
            break;

        case 4:
            redirect_to = playback_page(&app_ctx);
            break;

        default:
            text_copy(&app_ctx.logmsg, "Page Not Found");
            redirect_to = 1;
        }

        if (redirect_to != 0 && redirect_to != app_ctx.page) {
            text_empty(&app_ctx.general_use);
            text_empty(&app_ctx.logmsg);
            text_empty(&app_ctx.search_query);
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
    printf("[3] Playback\n");

    Key key = readkey();
    if (is_esc(key))
        return -1;

    if (key.keytype == KEY_STANDARD) {
        switch (key.ch.standard) {
        case '1':
            return 2;
        case '2':
            return 3;
        case '3':
            return 4;
        default:
            return -2;
        }
    }

    return 0;
}

/// === SEARCH PAGE ===

int search_page(AppContext *app) {
    if (app->search_results != NULL && app->search_results->length == 0)
        printf("\n[Esc] Return\n");
    else
        printf("\n[Esc] Return | [Enter] Add to queue\n");

    component_render_inputbox(&app->search_query);

    int max = 10;
    oss_get_terminal_size(NULL, &max);
    max = max - 4;

    if (app->search_results) {
        Queue *q = get_core_queue();

        for (int i = 0; i < app->search_results->length; i++) {
            if (i >= max)
                break;

            SearchResult *result = vec_get(app->search_results, i);

            bool in_queue = false;
            for (int j = 0; j < q->songs->length; j++) {
                MusicQueue *mq = pb_q_get(q, j);
                if (result->rowid == mq->id) {
                    in_queue = true;
                    break;
                }
            }

            if (in_queue) {
                if (i == app->selected)
                    printf(" >| %s\n", result->title);
                else
                    printf(" | %s\n", result->title);
                continue;
            }

            if (i == app->selected) {
                printf(" > %s\n", result->title);
            } else {
                printf(" - %s\n", result->title);
            }
        }
    }

    app->max_selected = max - 1;

    Key key = readkey();
    if (is_esc(key))
        return 1;

    if (key.keytype == KEY_ARROW) {
        handle_arrow_key(key, app);
        return 0;
    }

    if (app->selected < max && is_enter(key) && app->search_results != NULL &&
        app->search_results->length != 0) {

        SearchResult *result = vec_get(app->search_results, app->selected);

        MusicRow *row;
        DB_query_music_single(tgc->sqlite3, result->rowid, &row);

        Queue *q = get_core_queue();

        for (int j = 0; j < q->songs->length; j++) {
            MusicQueue *mq = pb_q_get(q, j);
            if (result->rowid == mq->id) {
                text_copy(&app->logmsg, "Already in Q");
                dbt_music_row_free(row);
                return 0;
            }
        }

        MusicQueue *mq = pb_musicq_alloc();

        mq->title = strdup(row->title);
        mq->fullpath = strdup(row->fullpath);
        mq->id = row->id;

        pb_q_add(q, mq);

        if (row) {
            char txt[255];
            sprintf(txt, "Added to playback '%s'", row->title);
            text_copy(&app->logmsg, txt);
        } else {
            text_copy(&app->logmsg, "rown't");
        }

        dbt_music_row_free(row);
        return 0;
    }

    collect_text(&app->search_query, key);
    app->selected = 0;

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

    if (sources->length == 0) {
        text_copy(&app_ctx->logmsg, "No sources are avaiable at the moment");
        return -1;
    }

    MediaSourceRow *row = vec_get_ref(sources, app_ctx->selected);
    vec_push(sources_for_scan, &row->path);

    clean_screen();

    int threads = 1;
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
#ifdef _WIN32
        Sleep(1);
#endif
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

    app_ctx->max_selected =
        component_render_sources_with_selection(app_ctx->selected) - 1;

    Key key = readkey();
    if (is_esc(key))
        return -1;

    if (key.keytype == KEY_ARROW) {
        handle_arrow_key(key, app_ctx);
        return 0;
    }

    return 0;
}

int media_delete_media_subpage(AppContext *app_ctx) {
    printf("\n[Esc] Return | [Y/Enter] Delete \n");

    app_ctx->max_selected =
        component_render_sources_with_selection(app_ctx->selected) - 1;

    Key key = readkey();
    if (is_esc(key))
        return -1;

    if (key.keytype == KEY_ARROW) {
        handle_arrow_key(key, app_ctx);
        return 0;
    }

    if (key.keytype == KEY_SPECIAL && is_enter(key) ||
        key.keytype == KEY_STANDARD && key.ch.standard == 'y') {

        Vec *sources;
        DB_query_source_all(tgc->sqlite3, &sources);

        MediaSourceRow *row = vec_get_ref(sources, app_ctx->selected);

        char text[255];

        if (DB_delete_source(tgc->sqlite3, row->id) != TDB_SUCCESS) {
            sprintf(text, "Failed to delete %s", row->path);
            text_copy(&app_ctx->logmsg, text);
        } else {
            sprintf(text, "Deleted %s", row->path);
            text_copy(&app_ctx->logmsg, text);
        }

        dbt_source_vec_rows_free(sources);

        app_ctx->selected = 0;
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
        case 4:
            newpage = media_delete_media_subpage(app_ctx);
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
    case 'd':
        app_ctx->subpage = 4;
        break;
    }

    return 0;
}

/// === PLAYBACK PAGE ===

void playback_queue_list_component(AppContext *app_ctx) {
    Queue *q = get_core_queue();

    if (q->songs->length == 0) {
        printf(" No Songs Yet\n");
    }

    for (int i = 0; i < q->songs->length; i++) {
        MusicQueue *mq = pb_q_get(q, i);
        char cursor = '-';

        if (app_ctx->selected == i)
            cursor = '>';

        printf(" %c %s\n", cursor, mq->title);
    }
}

void playback_volume_component(AppContext *app_ctx) {
    Queue *q = get_core_queue();

    if (q->feed) {
        printf(" volume   :   ");

        int volume = q->feed->volume * 10;

        if (volume == 10)
            printf("%d", volume);
        else
            printf("0%d", volume);

        printf(" [");

        for (int i = 0; i < 10; i++) {
            if (volume > i)
                printf("░");
            else
                printf(" ");
        }

        printf("]");

        printf("\n");
    } else {
        printf(" volume   :    -- [          ]   \n");
    }
}

void playback_timeline_component(AppContext *app_ctx) {
    Queue *q = get_core_queue();

    if (q->feed) {
        long duration_s = a_get_duration(q->feed) / 1000;
        long current_time_s = a_get_current_time(q->feed) / 1000;

        long displaym = current_time_s / 60;
        long displays = current_time_s % 60;

        long d_displaym = duration_s / 60;
        long d_displays = duration_s % 60;

        printf(" timeline : ");

        printf("%ld", displaym);
        printf(":");
        if (displays >= 10)
            printf("%ld", displays);
        else
            printf("0%ld", displays);

        printf(" [");

        float ratio = (float)current_time_s / (float)duration_s;

        for (int i = 0; i < 30; i++) {
            if ((int)(ratio * 30) > i) {
                printf("░");
            } else
                printf(" ");
        }

        printf("] ");

        printf("%ld", d_displaym);
        printf(":");
        if (d_displays >= 10)
            printf("%ld", d_displays);
        else
            printf("0%ld", d_displays);

        printf("\n");
    } else
        printf(" timeline : --:-- [                   ] --:--\n");
}

void playback_handle_arrow_input(AppContext *app_ctx, Key key) {
    handle_arrow_key(key, app_ctx);
    Queue *q = get_core_queue();

    bool state = q->feed->paused;

    pb_q_pause(q);
    switch (key.ch.arrow) {
    case ARROW_LEFT: {
        long ct = a_get_current_time(q->feed);
        long nt = ct - 1000 * 3;

        if (nt < 0)
            nt = 0;

        a_set_current_time(q->feed, nt);
        break;
    }

    case ARROW_RIGHT:

    {
        long d = a_get_duration(q->feed);
        long ct = a_get_current_time(q->feed);
        long nt = ct + 1000 * 3;

        if (nt > d)
            nt = d;

        a_set_current_time(q->feed, nt);
        break;
    }
    }

    if (state == false)
        pb_q_play(q);
}

int playback_page(AppContext *app_ctx) {
    usleep(35 * 1000);
    clean_screen();

    Queue *q = get_core_queue();

    if (q == NULL)
        return 1;

    char *song_status = "[]";
    char *song_name = "[]";

    if (q->feed != NULL && q->feed->paused) {
        song_status = "Paused";
    } else if (q->feed != NULL && !q->feed->paused) {
        song_status = "Playing";
    }

    if (q->feed) {
        MusicQueue *mq = pb_q_get_active(q);
        song_name = mq->title;
    }

    int width;
    oss_get_terminal_size(&width, NULL);

    printf("Playback - %s - %s \n", song_status, song_name);
    printf("[Esc] Return \n");

    // [l] toggle loop options "
    printf("[p] Play Song | [<Space>] Play/Pause | [< | >] Seek | [<[> | <]>] "
           "Volume | "
           "[r] Remove from queue \n");

    printf("----------- \n");
    printf(" Q - loop single \n");

    if (q->songs->length > 0)
        app_ctx->max_selected = q->songs->length - 1;

    playback_queue_list_component(app_ctx);

    printf("----------- \n");
    playback_volume_component(app_ctx);
    printf("----------- \n");
    playback_timeline_component(app_ctx);
    printf("----------- \n");

    Key key = readkey_nb();
    if (no_input(key))
        return 0;

    if (is_esc(key))
        return 1;

    if (is_space(key)) {
        if (pb_q_paused(q))
            pb_q_play(q);
        else
            pb_q_pause(q);
    }

    if (key.keytype == KEY_ARROW) {
        playback_handle_arrow_input(app_ctx, key);
        return 0;
    }

    if (key.keytype != KEY_STANDARD)
        return 0;

    if (key.ch.standard == 'r' && q->songs->length > 0) {
        pb_q_remove(q, app_ctx->selected);
        if (app_ctx->selected == 0)
            return 0;
        app_ctx->selected--;
    }

    if (key.ch.standard == 'p' && q->songs->length > 0) {
        pb_q_set_active(q, app_ctx->selected);
        pb_q_play(q);
    }

    if (key.ch.standard == '[' && q->feed) {
        q->feed->volume -= 0.1;
        if (q->feed->volume < 0)
            q->feed->volume = 0;
    }

    if (key.ch.standard == ']' && q->feed) {
        q->feed->volume += 0.1;
        if (q->feed->volume > 1)
            q->feed->volume = 1;
    }

    return 0;
}
