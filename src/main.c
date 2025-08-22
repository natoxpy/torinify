
///
/// main.c - Implementation of torinify client
/// Copyright (c) 2025 Author. All Rights Reserved.
/// License: MIT
///
/// Description:
///     Initial implementation of a TUI focused on implementing all of the
///     features the torinify library offers.
///

#include "db/helpers.h"
#include "db/tables.h"
#include "media/scan.h"
#include "storage/album.h"
#include "storage/artist.h"
#include "storage/music.h"
#include "torinify/playback.h"
#include "torinify/scanner.h"
#include "utils/generic_vec.h"
#include "utils/str.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <torinify/core.h>
#include <torinify/search_engine.h>
#include <wchar.h>

typedef unsigned long int_navigation;

#define BACKPAGE -1
#define HOME_PAGE 1
#define SEARCH_PAGE 2
#define PLAYBACK_PAGE 3
#define MEDIA_PAGE 4
#define DISCOGRAPHY_PAGE 5
#define ORGANIZATION_PAGE 6

size_t count_utf8_code_points(const char *s) {
    size_t count = 0;
    while (*s) {
        count += (*s++ & 0xC0) != 0x80;
    }
    return count;
}

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

int inline static is_char(Key key, char expected_key) {
    return key.keytype == KEY_STANDARD && key.ch.standard == expected_key;
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

void oss_usleep(useconds_t ms) { usleep(ms * 1000); }

double oss_get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

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

void oss_usleep(double ms) { Sleep(ms); }

double oss_get_time() {
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / freq.QuadPart;
}

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

void oss_setup() { SetConsoleOutputCP(CP_UTF8); }
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

    ret = tf_init_db("../../mycollection.db");
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

    // selected0 and selected1 mostly exist for the discography page
    int selected;

    // used to determine which selector to use
    int selected_cursor;
    int selected1;
    int selected2;

    int max_selected;
    Text logmsg;
    Text search_query;
    Vec *search_results;
    Text general_use;
    Vec *search_ctx;
    bool show_help;
    int return_page;
} AppContext;

AppContext init_app() {
    Queue *core_q = pb_q_alloc();
    pb_add_q(tgc->playback, core_q);

    AppContext app_ctx = {
        .page = 1,
        .selected = 0,
        .selected_cursor = 0,
        .selected1 = 0,
        .selected2 = 0,
        .max_selected = 0,
        .logmsg = {.len = 0, .cap = 1012, .str = malloc(sizeof(char *) * 1012)},
        .search_query = {.len = 0,
                         .cap = 255,
                         .str = malloc(sizeof(char *) * 255)},
        .general_use = {.len = 0,
                        .cap = 1012,
                        .str = malloc(sizeof(char *) * 1012)},
        .search_results = NULL,
        .search_ctx = NULL,
        .show_help = false,
        .return_page = HOME_PAGE,
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

int home_page(AppContext *app);
int search_page(AppContext *app);
int discography_page(AppContext *app);
int media_page(AppContext *app);
int playback_page(AppContext *app);
int organization_page(AppContext *app);
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
    freopen("null", "w", stderr);
#elif __unix__
    freopen("/dev/null", "w", stderr);
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
        if (app_ctx.page != 3) {
            clean_screen();
            printf("torinify - ");
        }

        if (app_ctx.logmsg.len != 0) {
            printf("%s - ", app_ctx.logmsg.str);
        }

        switch (app_ctx.page) {
        case HOME_PAGE:
            printf("Home\n");
            redirect_to = home_page(&app_ctx);

            break;

        case SEARCH_PAGE:
            redirect_to = search_page(&app_ctx);
            break;

        case PLAYBACK_PAGE:
            redirect_to = playback_page(&app_ctx);

#ifdef _WIN32
            oss_usleep(100);
#elif __unix__
            oss_usleep(25);
#endif

            break;

        case MEDIA_PAGE:
            printf("Media");
            redirect_to = media_page(&app_ctx);
            break;

        case DISCOGRAPHY_PAGE:
            redirect_to = discography_page(&app_ctx);
            break;

        case ORGANIZATION_PAGE:
            redirect_to = organization_page(&app_ctx);
            break;

        default:
            text_copy(&app_ctx.logmsg, "Page Not Found");
            redirect_to = 1;
        }

        if (app_ctx.page != redirect_to && redirect_to == 2) {
            s_vec_search_context_free(app_ctx.search_ctx);
            app_ctx.search_ctx = NULL;
            s_vec_search_context_init(tgc->sqlite3, &app_ctx.search_ctx);
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
    s_vec_search_context_free(app_ctx.search_ctx);

    return 0;
}

/// === HOME PAGE ===

int home_page(AppContext *app_ctx) {
    printf("[Esc] Exit\n");
    printf("[1] Search\n");
    printf("[2] Playback\n");
    printf("[3] Scan [awip] \n");
    // printf("[4] Discography\n");
    // printf("[5] Organization\n");
    // printf("[6] Metadata\n");

    Key key = readkey();
    if (is_esc(key)) {
        bool confirm_exit = false;

        while (confirm_exit == false) {
            printf("Exit (y)/n?\n");
            Key confirm = readkey();

            confirm_exit = is_enter(confirm);

            if (confirm_exit == false)
                confirm_exit = confirm.keytype == KEY_STANDARD &&
                               confirm.ch.standard == 'y';

            if (confirm.keytype == KEY_STANDARD && confirm.ch.standard == 'n')
                return 0;
        }

        return -1;
    }

    app_ctx->return_page = HOME_PAGE;

    if (key.keytype == KEY_STANDARD) {
        switch (key.ch.standard) {
        case '1':
            return SEARCH_PAGE;
        case '2':
            return PLAYBACK_PAGE;
        case '3':
            return MEDIA_PAGE;
        case '4':
            return DISCOGRAPHY_PAGE;
        case '5':
            return ORGANIZATION_PAGE;
        default:
            return -2;
        }
    }

    return 0;
}

/// === SEARCH PAGE ===

int search_page(AppContext *app) {
    printf("Search");

    if (app->search_results != NULL && app->search_results->length == 0)
        printf("\n[Esc] Return\n");
    else
        printf("\n[Esc] Return | [Enter] Add to queue\n");

    component_render_inputbox(&app->search_query);

    int tab_width = 0;
    int max = 10;
    oss_get_terminal_size(&tab_width, &max);
    max = max - 5;
    tab_width = tab_width / 3 - 3;

    char *tabs[3] = {"-|Music", "Album", "Artists"};

    for (int t = 0; t < 3; t++) {
        char *tab = tabs[t];

        int used_width = print_until_limit(tab, tab_width);

        for (int i = 0; i < tab_width - used_width; i++) {
            printf("-");
        }

        if (t != 2)
            printf("|");
        else
            printf("\n");
    }

    if (app->search_results) {
        Queue *q = get_core_queue();

        for (int i = 0; i < app->search_results->length; i++) {
            if (i >= max)
                break;

            SearchResult *result = vec_get(app->search_results, i);
            char *album_name = strdup("<No Album>");
            char *artist_name = strdup("<No Artist>");

            bool in_queue = false;
            for (int j = 0; j < q->songs->length; j++) {
                MusicQueue *mq = pb_q_get(q, j);
                if (result->rowid == mq->id) {
                    in_queue = true;
                    break;
                }
            }

            Vec *albums;
            s_music_get_all_albums(tgc->sqlite3, result->rowid, &albums);

            if (albums->length != 0) {
                Album *album_ref = vec_get_ref(albums, 0);
                free(album_name);
                album_name = strdup(album_ref->title);

                Vec *artists;
                s_album_get_all_artists(tgc->sqlite3, album_ref->id, &artists);

                if (artists->length != 0) {
                    Artist *artist_ref = vec_get_ref(artists, 0);
                    free(artist_name);
                    artist_name = strdup(artist_ref->name);
                }

                s_artist_vec_free(artists);
            }

            s_album_vec_free(albums);

            char *cursor = "  ";

            if (in_queue && i == app->selected) {
                cursor = ">*";
            } else if (in_queue && i != app->selected) {
                cursor = " *";
            } else if (i == app->selected) {
                cursor = "> ";
            }

            printf("%s", cursor);

            int tab_title_width = print_until_limit(result->title, tab_width);

            for (int _ = 0; _ < tab_width - (tab_title_width + 1); _++) {
                printf(" ");
            }

            int tab_album_width = print_until_limit(album_name, tab_width);

            for (int _ = 0; _ < tab_width - tab_album_width + 1; _++) {
                printf(" ");
            }

            int tab_artist_width = print_until_limit(artist_name, tab_width);

            for (int _ = 0; _ < tab_width - tab_artist_width + 1; _++) {
                printf(" ");
            }

            printf("\n");

            // printf(" %s%s ║ %s ║ %s\n", cursor, result->title, album_name,
            //        artist_name);

            free(album_name);
            free(artist_name);
        }
    }

    app->max_selected = max - 1;

    Key key = readkey();
    if (is_esc(key))
        return app->return_page;

    if (key.keytype == KEY_ARROW) {
        handle_arrow_key(key, app);
        return 0;
    }

    if (app->selected < max && is_enter(key) && app->search_results != NULL &&
        app->search_results->length != 0) {

        SearchResult *result = vec_get(app->search_results, app->selected);

        Music *music;
        s_music_get(tgc->sqlite3, result->rowid, &music);

        Queue *q = get_core_queue();

        for (int j = 0; j < q->songs->length; j++) {
            MusicQueue *mq = pb_q_get(q, j);
            if (result->rowid == mq->id) {
                text_copy(&app->logmsg, "Already in Q");
                s_music_free(music);
                return 0;
            }
        }

        MusicQueue *mq = pb_musicq_alloc();

        mq->title = strdup(music->title);
        mq->fullpath = strdup(music->fullpath);
        mq->id = music->id;

        if (pb_q_add(q, mq) != T_SUCCESS) {
            text_copy(&app->logmsg, "Error while adding music to queue");
            pb_musicq_free(mq);
            s_music_free(music);
            return 0;
        }

        if (music) {
            char txt[255];
            sprintf(txt, "Added to playback '%s'", music->title);
            text_copy(&app->logmsg, txt);
        } else {
            text_copy(&app->logmsg, "rown't");
        }

        s_music_free(music);
        return 0;
    }

    collect_text(&app->search_query, key);
    app->selected = 0;

    s_vec_search_result_free(app->search_results);

    double start_time = oss_get_time();

    s_process_search(app->search_ctx, app->search_query.str,
                     &app->search_results, 0.2);

    double diff_time = oss_get_time() - start_time;

    char txt[255];
    sprintf(txt, "search took %d milliseconds", (int)(diff_time * 1000));
    text_copy(&app->logmsg, txt);

    return 0;
}

/// === DISCOGRAPHY PAGE ===

void discography_page_panels_headers(AppContext *app, int artist_panel_width,
                                     int albums_panel_width,
                                     int songs_panel_width) {

    int artist_panel_width_use =
        print_until_limit("    [== Artist ==]", artist_panel_width);

    for (int y = 0; y < artist_panel_width - artist_panel_width_use + 3; y++) {
        printf(" ");
    }
    printf("|");

    int album_panel_width_use =
        print_until_limit("   [== Albums ==]", albums_panel_width);
    for (int y = 0; y < albums_panel_width - album_panel_width_use + 2; y++) {
        printf(" ");
    }
    printf("|");

    print_until_limit("   [== Songs ==]", songs_panel_width);

    printf("\n");
}

void discography_page_artists_panel(AppContext *app, Vec *artists, int y,
                                    int width) {
    if (y > artists->length - 1) {
        for (int i = 0; i < width + 3; i++) {
            if (i == artists->length) {
                printf("-");
            } else {
                printf(".");
            }
        }

        printf("|");

        return;
    }

    Artist *artist = vec_get_ref(artists, y);

    if (app->selected_cursor == 0 && app->selected == y) {
        printf("|>");
    } else if (app->selected_cursor != 0 && app->selected == y) {
        printf("|[");
    } else {
        printf("  ");
    }

    int printed = print_until_limit(artist->name, width);
    int until_limit = width - printed;

    for (int y = 0; y < until_limit; y++) {
        printf(" ");
    }

    if (app->selected_cursor == 0 && app->selected == y) {
        printf("<");
    } else if (app->selected_cursor != 0 && app->selected == y) {
        printf("]");
    } else {
        printf(" ");
    }

    printf("|");
}

void discography_page_albums_panel(AppContext *app, Vec *albums, int y,
                                   int width) {
    if (albums == NULL || y > albums->length - 1) {
        for (int i = 0; i < width + 2; i++) {
            if (albums != NULL && albums->length == y) {
                printf("-");
            } else {
                printf(".");
            }
        }

        printf("|");
        return;
    }

    Album *album = vec_get_ref(albums, y);

    if (app->selected_cursor == 1 && app->selected1 == y) {
        printf(">");
    } else if (app->selected_cursor != 1 && app->selected1 == y) {
        printf("[");
    } else {
        printf(" ");
    }

    int printed = print_until_limit(album->title, width);
    int until_limit = width - printed;

    for (int y = 0; y < until_limit; y++) {
        printf(" ");
    }

    if (app->selected_cursor == 1 && app->selected1 == y) {
        printf("<");
    } else if (app->selected_cursor != 1 && app->selected1 == y) {
        printf("]");
    } else {
        printf(" ");
    }

    printf("|");
}

void discography_page_songs_panel(AppContext *app, Vec *musics, int y,
                                  int width) {
    if (musics == NULL || y > musics->length - 1) {
        for (int i = 0; i < width + 2; i++) {
            if (musics != NULL && musics->length == y) {
                printf("-");
            } else {
                printf(".");
            }
        }

        printf("\n");
        return;
    }

    Music *music = vec_get_ref(musics, y);
    if (app->selected2 == y) {
        printf(">");
    } else {
        printf(" ");
    }

    int printed = print_until_limit(music->title, width);
    int until_limit = width - printed;

    for (int i = 0; i < until_limit; i++) {
        printf(" ");
    }

    if (app->selected2 == y) {
        printf("<|");
    } else {
        printf("  ");
    }

    printf("\n");
}

void discography_page_panels(AppContext *app) {
    int height;
    int width;
    oss_get_terminal_size(&width, &height);

    Vec *artists;
    char *artist_selected_name = "";
    s_artist_get_all(tgc->sqlite3, &artists);

    app->max_selected = artists->length - 1;

    Vec *albums = NULL;

    if (app->selected_cursor > 0 && !(app->selected > artists->length - 1)) {
        Artist *artist = vec_get_ref(artists, app->selected);
        artist_selected_name = artist->name;
        s_artist_get_all_albums(tgc->sqlite3, artist->id, &albums);
        app->max_selected = albums->length - 1;
    }

    Vec *musics = NULL;

    if (app->selected_cursor > 1 && !(app->selected1 > albums->length - 1)) {
        Album *album = vec_get_ref(albums, app->selected1);
        s_album_get_all_musics(tgc->sqlite3, album->id, &musics);
        app->max_selected = musics->length - 1;
    }

    int artists_n_albums_panel_width = width / 3 - 3;
    int songs_panel_width = width / 3 - 4;

    discography_page_panels_headers(app, artists_n_albums_panel_width,
                                    artists_n_albums_panel_width,
                                    songs_panel_width);

    int artist_offset = 0;
    int album_offset = 0;
    int songs_offset = 0;

    int mid = height / 2 - 6;

    if (app->selected > mid) {
        artist_offset = app->selected - mid;
    }

    if (app->selected1 > mid) {
        album_offset = app->selected1 - mid;
    }

    if (app->selected2 > mid) {
        songs_offset = app->selected2 - mid;
    }

    for (int i = 0; i < height - 4; i++) {
        discography_page_artists_panel(app, artists, i + artist_offset,
                                       artists_n_albums_panel_width);
        discography_page_albums_panel(app, albums, i + album_offset,
                                      artists_n_albums_panel_width);
        discography_page_songs_panel(app, musics, i + songs_offset,
                                     songs_panel_width);
    }

    s_album_vec_free(albums);
    s_artist_vec_free(artists);
    s_music_vec_free(musics);
}

int discography_page(AppContext *app) {
    printf("Discography - \n");
    printf("[Esc] Return\n");

    discography_page_panels(app);

    Key key = readkey();

    if (is_esc(key))
        return 1;

    if (is_enter(key)) {
        app->selected_cursor++;
        return 0;
    }

    switch (key.ch.arrow) {
    case ARROW_RIGHT:
        if (app->selected_cursor >= 2) {
            break;
        }

        app->selected_cursor++;
        break;
    case ARROW_LEFT:
        if (app->selected_cursor == 0)
            break;

        app->selected_cursor--;

        break;

    case ARROW_UP:
        if (app->selected_cursor == 0) {
            app->selected--;
            if (app->selected < 0) {
                app->selected = 0;
                break;
            }
            app->selected1 = 0;
            app->selected2 = 0;
        } else if (app->selected_cursor == 1) {
            app->selected1--;
            if (app->selected1 < 0) {
                app->selected1 = 0;
                break;
            }
            app->selected2 = 0;
        } else if (app->selected_cursor == 2) {
            app->selected2--;
            if (app->selected2 < 0)
                app->selected2 = 0;
        }

        break;
    case ARROW_DOWN:
        if (app->selected_cursor == 0) {
            app->selected++;
            if (app->selected > app->max_selected) {
                app->selected = app->max_selected;
                break;
            }
            app->selected1 = 0;
            app->selected2 = 0;
        } else if (app->selected_cursor == 1) {
            app->selected1++;
            if (app->selected1 > app->max_selected) {
                app->selected1 = app->max_selected;
                break;
            }
            app->selected2 = 0;
        } else if (app->selected_cursor == 2) {
            app->selected2++;
            if (app->selected2 > app->max_selected) {
                app->selected2 = app->max_selected;
            }
        }

        break;
    }

    return 0;
}

/// === Organization Page ===

int organization_page(AppContext *app_ctx) {
    printf("Organization\n");
    printf("[Esc] Return\n");

    Key key = readkey();
    if (is_esc(key))
        return 1;

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

/// === MEDIA PAGE - SCAN ===

// int_navigation media_scan_opt_select() {
//     printf(" - Scan Options - \n");
//     printf("[1] Only scan for new music (default)\n");
//     printf("[2] Scan new and existing music\n");
//
//     Key key;
//
//     for (;;) {
//         key = readkey();
//
//         if (is_esc(key))
//             return BACKPAGE;
//
//         if (is_char(key, '1'))
//             return MEDIA_SCAN_OPT_1;
//
//         if (is_char(key, '2'))
//             return MEDIA_SCAN_OPT_2;
//     }
// }

/*
int_navigation media_scan_select_media(AppContext *app_ctx) {
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

    return 0;
}
*/

bool media_scan_util_selected(Vec *v, int selected, int *out_index) {
    for (int i = 0; i < v->length; i++) {
        int s = *(int *)vec_get(v, i);

        if (s == selected) {
            if (out_index != NULL)
                *out_index = i;
            return true;
        }
    }

    return false;
}

int_navigation media_scan_select_sources(AppContext *app_ctx,
                                         Vec **out_sources) {
    Vec *selected_sources = vec_init(sizeof(int));
    Vec *sources;

    DB_query_source_all(tgc->sqlite3, &sources);

    app_ctx->max_selected = sources->length;
    app_ctx->selected = 0;

    int page = 0;

    for (;;) {
        clean_screen();
        printf("Media Sources Select\n");
        printf("[Space] Select/Deselect, [a] Select All/Deselect All, [Enter] "
               "Continue\n");

        for (int i = 0; i < sources->length; i++) {
            MediaSourceRow *row = vec_get_ref(sources, i);

            bool selected =
                media_scan_util_selected(selected_sources, row->id, NULL);
            bool hover = app_ctx->selected == i;

            if (selected && hover)
                printf("[x]");
            else if (!selected && hover)
                printf("[ ]");
            else if (selected && !hover)
                printf("(x)");
            else
                printf("( )");

            printf("%s\n", row->path);
        }

        Key key = readkey();

        if (is_esc(key)) {
            page = BACKPAGE;
            break;
        }

        if (is_enter(key))
            break;

        if (is_space(key)) {
            MediaSourceRow *row = vec_get_ref(sources, app_ctx->selected);

            int index;

            bool selected =
                media_scan_util_selected(selected_sources, row->id, &index);

            if (!selected)
                vec_push(selected_sources, (void *)(uintptr_t)&row->id);
            else
                vec_remove(selected_sources, index);
        }

        if (is_char(key, 'a')) {
            bool currently_full = selected_sources->length == sources->length;

            if (currently_full)
                for (int i = 0; i < sources->length; i++) {
                    vec_pop(selected_sources);
                }
            else
                for (int i = 0; i < sources->length; i++) {
                    MediaSourceRow *row = vec_get_ref(sources, i);
                    if (!media_scan_util_selected(selected_sources, row->id,
                                                  NULL))
                        vec_push(selected_sources, (void *)(uintptr_t)&row->id);
                }
        }

        switch (key.ch.arrow) {
        case ARROW_UP:
            app_ctx->selected--;
            if (app_ctx->selected <= 0)
                app_ctx->selected = 0;

            break;
        case ARROW_DOWN:
            app_ctx->selected++;
            if (app_ctx->selected >= app_ctx->max_selected - 1)
                app_ctx->selected = app_ctx->max_selected - 1;
            break;
        }
    }

    Vec *out_dec = vec_init(sizeof(MediaSourceRow *));

    for (int i = 0; i < sources->length; i++) {
        MediaSourceRow *row = vec_get_ref(sources, i);
        bool selected =
            media_scan_util_selected(selected_sources, row->id, NULL);

        if (selected) {
            MediaSourceRow *scrow = malloc(sizeof(MediaSourceRow));

            scrow->id = row->id;
            scrow->path = strdup(row->path);

            vec_push(out_dec, &scrow);
        }
    }

    dbt_source_vec_rows_free(sources);
    vec_free(selected_sources);

    if (page != BACKPAGE)
        *out_sources = out_dec;

    return page;
}

int_navigation media_scan_perform_scan_page(AppContext *app_ctx, Vec *sources,
                                            ScannerContext **scanner_ctx_out) {
    clean_screen();
    int page = 0;
    if (sources == NULL || sources != NULL && sources->length == 0) {
        text_copy(&app_ctx->logmsg, "No sources selected");
        return BACKPAGE;
    }

    ScannerContext *scanner_ctx = sc_scan_start_from_vec(tgc->sqlite3, sources);

    int ret, width, final_rerender = 1;

    while ((ret = sc_lock_scan(scanner_ctx)) == 0 || final_rerender) {
        if (ret != 0)
            final_rerender = 0;

        clean_screen();
        oss_get_terminal_size(&width, NULL);

        float dk = (float)scanner_ctx->scan_ctx->processed /
                   scanner_ctx->scan_ctx->data->length;

        char buff[255];
        sprintf(buff, "(%d / %d)", scanner_ctx->scan_ctx->processed,
                scanner_ctx->scan_ctx->data->length);
        printf("%s[", buff);

        int uwidth = width - (2 + strlen(buff));

        for (int i = 0; i < uwidth; i++) {
            float bk = (float)i / uwidth;

            if (bk >= dk)
                printf(" ");
            else
                printf("|");
        }

        printf("]\n");

        sc_unlock_scan(scanner_ctx);

        oss_usleep(1000);
    }

    *scanner_ctx_out = scanner_ctx;

    return page;
}

void media_scan_component_file_state_name(char *cursor, char *name) {
    int width;
    oss_get_terminal_size(&width, NULL);

    char buff_title[255];
    snprintf(buff_title, 255, "%s%s", cursor, name);

    print_with_blanks(buff_title, ' ', width / 3);
    printf("|");
}

void media_scan_component_file_state_album(char *cursor, char *album) {
    int width;
    oss_get_terminal_size(&width, NULL);

    char buff_album[255];
    snprintf(buff_album, 255, "%s%s", cursor, album);

    print_with_blanks(buff_album, ' ', width / 3 - 1);
    printf("|");
}

void media_scan_component_file_state_artists(int i, FileState *file_state,
                                             int x_cursor, int y_cursor,
                                             int *x_cursor_max,
                                             char *static_cursor,
                                             char *static_no_cursor) {
    int width;
    oss_get_terminal_size(&width, NULL);

    char *cursor = static_no_cursor;
    char buff_artists[512] = "\0";

    int buff_artists_len = 0;

    int artists_length = file_state->metadata.artists->length;

    if (y_cursor == i)
        *x_cursor_max = artists_length + 1;

    for (int y = 0; y < artists_length; y++) {
        char *artist = vec_get_ref(file_state->metadata.artists, y);

        if (y_cursor == i && x_cursor == 2 + y)
            cursor = static_cursor;
        else
            cursor = static_no_cursor;

        strncat(buff_artists, "", 512 - 1);
        strncat(buff_artists, cursor, 512 - 1);
        strncat(buff_artists, artist, 512 - 1);

        if (artists_length - 1 != y)
            strncat(buff_artists, " / ", 512 - 1);
    }

    print_with_blanks(buff_artists, ' ', width / 3 - 1);
}

void media_scan_component_file_state(FileState *file_state, int i, int x_cursor,
                                     int y_cursor, int *x_cursor_max) {

    int width;
    oss_get_terminal_size(&width, NULL);

    char *static_cursor = "|> ";
    char *static_no_cursor = "";

    char *cursor = static_no_cursor;
    if (y_cursor == i && x_cursor == 0)
        cursor = static_cursor;

    media_scan_component_file_state_name(cursor, file_state->metadata.name);

    cursor = static_no_cursor;
    if (y_cursor == i && x_cursor == 1)
        cursor = static_cursor;

    media_scan_component_file_state_album(cursor, file_state->metadata.album);

    media_scan_component_file_state_artists(i, file_state, x_cursor, y_cursor,
                                            x_cursor_max, static_cursor,
                                            static_no_cursor);

    printf("\n");
}

void media_scan_input_multitask_component(Text *text, int state) {
    if (state == 0)
        return;

    switch (state) {
    case 0:
        return;
    case 1:
        printf("edit name: ");
        break;
    case 2:
        printf("edit album: ");
        break;
    case 3:
        printf("edit artist: ");
        break;
    case 4:
        printf("new artist: ");
        break;
    }

    component_render_inputbox(text);
}

#define INPUT_USAGE_NOT_ACTIVE 0
#define INPUT_USAGE_RENAME_NAME 1
#define INPUT_USAGE_RENAME_ALBUM 2
#define INPUT_USAGE_RENAME_ARTIST 3
#define INPUT_USAGE_ADD_ARTIST 4

int_navigation
media_scan_media_precommit_modifications_page(AppContext *app_ctx,
                                              ScannerContext *scanner_ctx) {
    int x_cursor = 0;
    int y_cursor = 0;
    int y_cursor_max = scanner_ctx->scan_ctx->data->length - 1;

    Text input = {.len = 0, .cap = 255, .str = malloc(sizeof(char *) * 255)};
    text_empty(&input);

    // 0 = not active, 1 edit name, 2 edit album, 3 edit artist, 4 add artist
    int input_usage_state = INPUT_USAGE_NOT_ACTIVE;
    int input_usage_target = 0;
    int input_usage_x_target = 0;

    while (1) {
        if (y_cursor <= 0)
            y_cursor = 0;

        clean_screen();
        int height, width;
        oss_get_terminal_size(&width, &height);

        printf("Scan Found Page - %d results",
               scanner_ctx->scan_ctx->data->length);

        if (x_cursor == 0 && input_usage_state != INPUT_USAGE_NOT_ACTIVE)
            printf(" | [Enter] Rename Title | [ESC] Cancel Rename");
        else if (x_cursor == 0)
            printf(" | [r] Rename Title");

        if (x_cursor == 1 && input_usage_state != INPUT_USAGE_NOT_ACTIVE)
            printf(" | [Enter] Rename Album | [ESC] Cancel Rename");
        else if (x_cursor == 1)
            printf(" | [r] Rename Album");

        if (x_cursor >= 2 && input_usage_state == INPUT_USAGE_NOT_ACTIVE)
            printf(" | [r] Rename Artist");
        else if (x_cursor >= 2)
            printf(" | [Enter] Rename Artist | [ESC] Cancel Rename");

        if (input_usage_state == INPUT_USAGE_NOT_ACTIVE)
            printf(" | [a] Add Artist");

        if (x_cursor >= 2 && input_usage_state == INPUT_USAGE_NOT_ACTIVE)
            printf(" | [d] Delete Artist\n");
        else
            printf("\n");

        media_scan_input_multitask_component(&input, input_usage_state);

        print_with_blanks("[== Title ==]", '-', width / 3);
        print_with_blanks("[== Album ==]", '-', width / 3);
        print_with_blanks("[== Artists ==]", '-', width / 3);

        printf("\n");

        int cursor_offset = y_cursor;
        int x_cursor_max = 1;

        for (int i = cursor_offset; i < scanner_ctx->scan_ctx->data->length;
             i++) {
            if (i - cursor_offset > height - (4 + 1))
                break;

            FileState *file_state = vec_get_ref(scanner_ctx->scan_ctx->data, i);
            media_scan_component_file_state(file_state, i, x_cursor, y_cursor,
                                            &x_cursor_max);
        }

        Key key = readkey();
        if (input_usage_state != 0)
            collect_text(&input, key);

        if (is_esc(key) && input_usage_state != INPUT_USAGE_NOT_ACTIVE) {
            input_usage_state = 0;
            text_empty(&input);
        } else if (is_esc(key)) {
            // sc_scan_context_free(scanner_ctx);
            free(input.str);
            return BACKPAGE;
        }

        if (is_char(key, 'a')) {
            input_usage_state = 4;
            input_usage_target = y_cursor;
            continue;
        }

        if (is_char(key, 'd') && x_cursor >= 2) {
            FileState *file_state =
                vec_get_ref(scanner_ctx->scan_ctx->data, y_cursor);

            char *artist =
                vec_get_ref(file_state->metadata.artists, x_cursor - 2);
            free(artist);

            vec_remove(file_state->metadata.artists, x_cursor - 2);
        }

        if (is_char(key, 'r') && input_usage_state == INPUT_USAGE_NOT_ACTIVE) {
            input_usage_target = y_cursor;

            FileState *file_state =
                vec_get_ref(scanner_ctx->scan_ctx->data, y_cursor);

            if (x_cursor == 0) {
                input_usage_state = INPUT_USAGE_RENAME_NAME;
                text_copy(&input, file_state->metadata.name);
            }

            if (x_cursor == 1) {
                input_usage_state = INPUT_USAGE_RENAME_ALBUM;
                text_copy(&input, file_state->metadata.album);
            }

            if (x_cursor >= 2) {
                input_usage_state = INPUT_USAGE_RENAME_ARTIST;
                input_usage_x_target = x_cursor - 2;
                char *artist = vec_get_ref(file_state->metadata.artists,
                                           input_usage_x_target);

                text_copy(&input, artist);
            }

        } else if (is_enter(key)) {

            char *input_str = trim(strdup(input.str));

            FileState *file_state =
                vec_get_ref(scanner_ctx->scan_ctx->data, input_usage_target);

            if (input_usage_state == INPUT_USAGE_RENAME_NAME) {
                free(file_state->metadata.name);
                file_state->metadata.name = strdup(input_str);
            } else if (input_usage_state == INPUT_USAGE_RENAME_ALBUM) {
                free(file_state->metadata.album);
                file_state->metadata.album = strdup(input_str);
            } else if (input_usage_state == INPUT_USAGE_RENAME_ARTIST) {
                char *artist = vec_get_ref(file_state->metadata.artists,
                                           input_usage_x_target);

                strcpy(artist, input_str);
            } else if (input_usage_state == INPUT_USAGE_ADD_ARTIST) {
                char *artist = strdup(input_str);
                vec_push(file_state->metadata.artists, &artist);
            }

            text_empty(&input);
            free(input_str);
            input_usage_state = INPUT_USAGE_NOT_ACTIVE;
        }

        if (input_usage_state == INPUT_USAGE_NOT_ACTIVE) {
            switch (key.ch.arrow) {
            case ARROW_DOWN:
                y_cursor++;
                if (y_cursor > y_cursor_max)
                    y_cursor = y_cursor_max;
                break;
            case ARROW_UP:
                y_cursor--;
                if (y_cursor < 0)
                    y_cursor = 0;
                break;
            case ARROW_LEFT:
                x_cursor--;
                if (x_cursor < 0)
                    x_cursor = 0;
                break;
            case ARROW_RIGHT:
                x_cursor++;
                if (x_cursor > x_cursor_max)
                    x_cursor = x_cursor_max;
                break;
            }
        }

        if (x_cursor >= 2) {
            FileState *file_state =
                vec_get_ref(scanner_ctx->scan_ctx->data, y_cursor);

            if (x_cursor > file_state->metadata.artists->length + 1) {
                x_cursor = file_state->metadata.artists->length + 1;
            }
        }
    }

    free(input.str);

    // sc_scan_context_free_and_commit(scanner_ctx);

    printf("Press any key to continue\n");

    readkey();

    return BACKPAGE;
}

int_navigation media_scan_media_subpage(AppContext *app_ctx) {
    Vec *sources = NULL;
    int nav;

    nav = media_scan_select_sources(app_ctx, &sources);
    if (nav == BACKPAGE)
        return BACKPAGE;

    ScannerContext *scanner_ctx = NULL;

    nav = media_scan_perform_scan_page(app_ctx, sources, &scanner_ctx);
    dbt_source_vec_rows_free(sources);

    if (nav == BACKPAGE)
        return BACKPAGE;

    nav = media_scan_media_precommit_modifications_page(app_ctx, scanner_ctx);

    if (nav == BACKPAGE) {
        sc_scan_context_free_and_cancel(scanner_ctx);
    }

    return BACKPAGE;
}

int media_scan_media_subpage_old(AppContext *app_ctx) {
    // app_ctx->max_selected =
    //     component_render_sources_with_selection(app_ctx->selected) - 1;

    // Key key = readkey();
    // if (is_esc(key))
    //     return -1;

    // if (key.keytype == KEY_ARROW) {
    //     handle_arrow_key(key, app_ctx);
    //     return 0;
    // }

    // if (!is_enter(key)) {
    //     return -1;
    // }

    // printf(" - Scan Options - \n");
    // printf("[1] Only scan for new music (default)\n");
    // printf("[2] Scan new and existing music\n");

    // do {
    //     key = readkey();
    //     if (is_esc(key))
    //         return -1;
    //     if (is_enter(key) || key.keytype == KEY_STANDARD)
    //         break;

    // } while (1);

    // Vec *sources;
    // DB_query_source_all(tgc->sqlite3, &sources);

    // if (sources->length == 0) {
    //     text_copy(&app_ctx->logmsg, "No sources are avaiable at the moment");
    //     return -1;
    // }

    // MediaSourceRow *row = vec_get_ref(sources, app_ctx->selected);

    // ScannerContext *scanner_ctx =
    //     sc_scan_start_single_root(tgc->sqlite3, row->id);

    // dbt_source_vec_rows_free(sources);

    // int ret, width, final_rerender = 1;

    // while ((ret = sc_lock_scan(scanner_ctx)) == 0 || final_rerender) {
    //     if (ret != 0)
    //         final_rerender = 0;

    //     clean_screen();
    //     oss_get_terminal_size(&width, NULL);

    //     float dk = (float)scanner_ctx->scan_ctx->processed /
    //                scanner_ctx->scan_ctx->data->length;

    //     char buff[255];
    //     sprintf(buff, "(%d / %d)", scanner_ctx->scan_ctx->processed,
    //             scanner_ctx->scan_ctx->data->length);
    //     printf("%s[", buff);

    //     int uwidth = width - (2 + strlen(buff));

    //     for (int i = 0; i < uwidth; i++) {
    //         float bk = (float)i / uwidth;

    //         if (bk >= dk)
    //             printf(" ");
    //         else
    //             printf("|");
    //     }

    //     printf("]\n");

    //     sc_unlock_scan(scanner_ctx);

    //     oss_usleep(1000);
    // }

    // int x_cursor = 0;
    // int y_cursor = 0;

    // while (1) {
    //     if (y_cursor <= 0)
    //         y_cursor = 0;

    //     clean_screen();
    //     int height;
    //     oss_get_terminal_size(&width, &height);

    //     printf("Scan Found Page - %d results\n",
    //            scanner_ctx->scan_ctx->data->length);

    //     printf("\n\n\n\n");

    //     print_with_blanks("[== Title ==]", '-', width / 3);
    //     print_with_blanks("[== Album ==]", '-', width / 3);
    //     print_with_blanks("[== Artists ==]", '-', width / 3);

    //     printf("\n");

    //     int cursor_offset = y_cursor;
    //     int x_cursor_max = 1;

    //     for (int i = cursor_offset; i < scanner_ctx->scan_ctx->data->length;
    //          i++) {
    //         if (i - cursor_offset > height - (4 + 4))
    //             break;

    //         FileState *file_state = vec_get_ref(scanner_ctx->scan_ctx->data,
    //         i);

    //         char *static_cursor = "|> ";
    //         char *static_no_cursor = "";

    //         char *cursor = "";

    //         if (y_cursor == i && x_cursor == 0)
    //             cursor = static_cursor;
    //         else
    //             cursor = static_no_cursor;

    //         char buff_title[255];
    //         snprintf(buff_title, 255, "%s%s", cursor,
    //                  file_state->metadata.name);

    //         if (y_cursor == i && x_cursor == 1)
    //             cursor = static_cursor;
    //         else
    //             cursor = static_no_cursor;

    //         char buff_album[255];
    //         snprintf(buff_album, 255, "%s%s", cursor,
    //                  file_state->metadata.album);

    //         char buff_artists[512] = "\0";

    //         int buff_artists_len = 0;

    //         int artists_length = file_state->metadata.artists->length;

    //         if (y_cursor == i)
    //             x_cursor_max = artists_length + 1;

    //         for (int y = 0; y < artists_length; y++) {
    //             char *artist = vec_get_ref(file_state->metadata.artists, y);

    //             if (y_cursor == i && x_cursor == 2 + y)
    //                 cursor = static_cursor;
    //             else
    //                 cursor = static_no_cursor;

    //             strncat(buff_artists, "(+)", 512 - 1);
    //             strncat(buff_artists, cursor, 512 - 1);
    //             strncat(buff_artists, artist, 512 - 1);

    //             if (artists_length - 1 != y)
    //                 strncat(buff_artists, " / ", 512 - 1);
    //         }

    //         print_with_blanks(buff_title, ' ', width / 3);
    //         printf("|");

    //         print_with_blanks(buff_album, ' ', width / 3 - 1);
    //         printf("|");

    //         print_with_blanks(buff_artists, ' ', width / 3 - 1);

    //         printf("\n");
    //     }

    //     Key key = readkey();
    //     if (is_esc(key)) {
    //         sc_scan_context_free(scanner_ctx);
    //         return -1;
    //     }

    //     switch (key.ch.arrow) {
    //     case ARROW_DOWN:
    //         y_cursor++;
    //         break;
    //     case ARROW_UP:
    //         y_cursor--;
    //         break;
    //     case ARROW_LEFT:
    //         x_cursor--;
    //         if (x_cursor < 0)
    //             x_cursor = 0;
    //         break;
    //     case ARROW_RIGHT:
    //         x_cursor++;
    //         if (x_cursor > x_cursor_max)
    //             x_cursor = x_cursor_max;
    //         break;
    //     }
    // }

    // sc_scan_context_free_and_commit(scanner_ctx);

    // printf("Press any key to continue\n");

    // readkey();

    return -1;
}

// int media_rescan_final_preview_subpage(AppContext *app_ctx) {}

// int media_rescan_media_subpage(AppContext *app_ctx) {
//     printf("\n[Esc] Return\n");
//
//     app_ctx->max_selected =
//         component_render_sources_with_selection(app_ctx->selected) - 1;
//
//     Key key = readkey();
//     if (is_esc(key))
//         return -1;
//
//     if (key.keytype == KEY_ARROW) {
//         handle_arrow_key(key, app_ctx);
//         return 0;
//     }
//
//     return 0;
// }

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
           "media | [d] Delete Selected Media Source\n");

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
    case 'd':
        app_ctx->subpage = 3;
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
        if (q->feed == NULL)
            break;

        MusicQueue *mq = pb_q_get(q, i);
        char cursor = '-';
        char *suffix = "";

        if (app_ctx->selected == i)
            cursor = '>';

        if (q->active == i && q->feed->paused)
            suffix = "(paused)";
        else if (q->active == i && !q->feed->paused)
            suffix = "(playing)";

        printf(" %c %s %s\n", cursor, mq->title, suffix);
    }
}

void playback_volume_component(AppContext *app_ctx) {
    Queue *q = get_core_queue();

    if (q->feed) {
        printf(" volume   :   ");

        int volume = q->volume * 10;

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
        long duration_s = a_get_duration(q->feed);
        long current_time_s = a_get_current_time(q->feed);

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

    if (q->feed == NULL)
        return;

    bool state = q->feed->paused;

    switch (key.ch.arrow) {
    case ARROW_LEFT: {
        pb_q_set_current_time(q, pb_q_get_current_time(q) - 3);
        break;
    }

    case ARROW_RIGHT: {
        pb_q_set_current_time(q, pb_q_get_current_time(q) + 3);
        break;
    }
    }
}

int playback_page(AppContext *app_ctx) {

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

    printf("torinify - Playback - %s - %s \n", song_status, song_name);
    if (app_ctx->show_help) {
        printf("[Esc] Return \n");

        printf(
            "[p] Play Song | [<Space>] Play/Pause | [< | >] Seek | [<[> | <]>] "
            "Volume | "
            "[r] Remove from queue | [l] toggle loop options | [/] Search "
            "page\n");
    }

    printf("----- Queue -----");

    switch (q->loopstyle) {
    case P_LOOP_NONE:
        printf("\n");
        break;
    case P_LOOP_SINGLE:
        printf("- repeat \n");
        break;
    case P_LOOP_QUEUE:
        printf("- repeat queue \n");
    }

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

    if (q->songs->length > 0 &&
        (is_enter(key) ||
         (key.keytype == KEY_STANDARD && key.ch.standard == 'p'))) {
        pb_q_set_active(q, app_ctx->selected);
        pb_q_play(q);
    }

    if (key.keytype != KEY_STANDARD)
        return 0;

    if (key.ch.standard == 'r' && q->songs->length > 0) {
        pb_q_remove(q, app_ctx->selected);
        if (app_ctx->selected != 0)
            app_ctx->selected--;

        if (q->songs->length != 0) {
            pb_q_set_active(q, app_ctx->selected);
        }
    }

    if (key.ch.standard == '[' && q->feed) {
        float vol = q->volume * 10;
        pb_q_set_volume(q, (vol - 1) / 10);
    }

    if (key.ch.standard == ']' && q->feed) {
        float vol = q->volume * 10;
        pb_q_set_volume(q, (vol + 1) / 10);
    }

    if (key.ch.standard == 'l') {
        uint8_t l = q->loopstyle + 1;

        if (l > P_LOOP_SINGLE)
            l = P_LOOP_NONE;

        q->loopstyle = l;
    }

    if (key.ch.standard == 'h') {
        app_ctx->show_help = !app_ctx->show_help;
    }

    if (key.ch.standard == '/') {
        app_ctx->return_page = PLAYBACK_PAGE;
        return SEARCH_PAGE;
    }

    return 0;
}
