#include "errors/errors.h"
#include "torinify/core.h"
#include "torinify/search_engine.h"
#include <stdio.h>
#include <stdlib.h>

void hide_cursor() { printf("\033[?25l"); }
void show_cursor() { printf("\033[?25h"); }

#define KEY_ESC 27

#ifdef _WIN32
#include <conio.h>
#include <windows.h>

#define SLEEP(ms) Sleep(ms);

void get_terminal_size(int *width, int *height) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        *width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        *height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    } else {
        *width = 80;
        *height = 24;
    }
}
void clear_screen() {
    HANDLE hStdOut;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    DWORD cellCount;
    COORD homeCoords = {0, 0};

    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdOut == INVALID_HANDLE_VALUE)
        return;

    /* Get the number of cells in the current buffer */
    if (!GetConsoleScreenBufferInfo(hStdOut, &csbi))
        return;
    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    /* Fill the entire buffer with spaces */
    if (!FillConsoleOutputCharacter(hStdOut, (TCHAR)' ', cellCount, homeCoords,
                                    &count))
        return;

    /* Fill the entire buffer with the current colors and attributes */
    if (!FillConsoleOutputAttribute(hStdOut, csbi.wAttributes, cellCount,
                                    homeCoords, &count))
        return;

    /* Move the cursor home */
    SetConsoleCursorPosition(hStdOut, homeCoords);
}
#elif __unix__
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
void clear_screen() { printf("\033[H\033[J"); }

#define SLEEP(ms) usleep((ms) * 1000);

void get_terminal_size(int *width, int *height) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        *width = w.ws_col;
        *height = w.ws_row;
    } else {
        *width = 80;
        *height = 24;
    }
}

void enableRawMode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);

    // Disable canonical mode and echo
    term.c_lflag &= ~(ICANON | ECHO);

    // Apply the settings
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    // Set the file descriptor to non-blocking
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

void disableRawMode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);

    // Restore canonical mode and echo
    term.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    // Restore blocking mode
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
}
#endif

#ifdef _WIN32
#include <conio.h>
void readch(char *c) { *c = _getch(); }
#else
#include <termios.h>
#include <unistd.h>
void readch(char *c) {
    int res = read(STDIN_FILENO, c, 1);
    if (res <= 0) {
        *c = 0; // No input
    }
}
#endif

struct String {
    char *ptr;
    unsigned int len;
};

struct SearchContext {
    Vec *results;
    struct String query;
    int selected;
};

struct Context {
    int page;
    int redirects;
    char *premsg;
};

int home_page();
int search_page(struct SearchContext *ctx);

int main() {
#ifdef __unix__
    enableRawMode();
#endif

    int ret = T_FAIL;

    if ((ret = tf_init()) != T_SUCCESS ||
        (ret = tf_init_db("../../sqlite.db", "../../migrations")) != T_SUCCESS)
        goto end;

    hide_cursor();

    struct SearchContext search_ctx = {
        .results = NULL,
        .selected = 0,
        .query = {.len = 0, .ptr = malloc(sizeof(char) * 1024)}};

    search_ctx.query.ptr[0] = '\0';

    struct Context ctx = {.page = 0, .redirects = 0, .premsg = 0};

    while (1) {
        clear_screen();
        printf("Torinify Demo Project - [ %d ] ", ctx.redirects);

        if (ctx.premsg != 0) {
            printf("[ %s ] ", ctx.premsg);
        }

        switch (ctx.page) {
        case 0: {
            printf("- Home Page\n");
            int np = home_page();
            if (np == 0)
                goto end;

            if (np == -1)
                break;

            if (np != ctx.page)
                ctx.redirects++;

            ctx.page = np;
            break;
        }
        case 1: {
            ctx.premsg = 0;
            printf("- Search: %s\033[48;5;15m \033[0m\n", search_ctx.query.ptr);

            int np = search_page(&search_ctx);
            if (np == -1)
                break;

            if (np != ctx.page)
                ctx.redirects++;

            ctx.page = np;
            break;
        }
        default: {
            ctx.premsg = "Page did not exist";
            ctx.page = 0;
            break;
        }
        }

        SLEEP(10);
    }
end:
    s_vec_search_result_free(search_ctx.results);
    free(search_ctx.query.ptr);
    clear_screen();

    tf_cleanup();

#ifdef __unix__
    disableRawMode();
#endif

    if (ret != T_SUCCESS) {
        printf("ended with errors");
    }

    show_cursor();
    return 0;
}

int home_page() {
    printf("[ESC]: exit\n");
    printf("1: Search\n");
    printf("2: Playlist\n");
    printf("3: Playback\n");

    char c = 0;
    readch(&c);
    if (c == 0)
        return -1;

    switch (c) {
    case KEY_ESC:
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    default:
        return 4;
    }

    return -1;
}

int search_page(struct SearchContext *ctx) {
    printf("[ESC]: return to home\n");

    int max = 12;
    int _null;
    get_terminal_size(&_null, &max);
    max = max - 3;

    int len = 0;
    if (ctx->results) {

        for (int i = 0; i < ctx->results->length; i++) {
            if (i >= max)
                break;

            len++;

            SearchResult *result = vec_get(ctx->results, i);
            if (i == ctx->selected ||
                ((ctx->selected >= max ||
                  ctx->selected >= ctx->results->length) &&
                 i == ctx->results->length - 1)) {

#ifdef _WIN32
                HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                SetConsoleTextAttribute(hConsole,
                                        BACKGROUND_BLUE | BACKGROUND_GREEN |
                                            BACKGROUND_RED); // Gray background
                printf("[%.2f] - %s", result->distance, result->title);
                SetConsoleTextAttribute(hConsole, 7); // Reset to default
                printf("\n");
#else
                // Use ANSI escape codes for Linux/macOS or modern Windows
                // Terminal
                printf("\033[48;5;0m[%d] - %s\033[0m\n", index + 1,
                       result->title);
#endif

            } else {
                printf("[%.2f] - %s\n", result->distance, result->title);
                // printf("[%d] - %s\n", i + 1, result->title);
            }
        }
    }

    char c = 0;
    readch(&c);
    if (c == 0)
        return -1;

#ifdef _WIN32
#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77
    if (c == KEY_ESC)
        return 0;

    // Handle arrow keys
    if (c == 0 || c == 224 || c == -32) { // Ignore prefix byte
        readch(&c);
    } else if (c == '\r') {
        c = '\n';
        goto handle_c;
    } else {
        goto handle_c;
    }

    switch (c) {
    case KEY_UP:
        if (ctx->selected > 0)
            ctx->selected--;
        else
            ctx->selected = len - 1;
        return -1;
    case KEY_DOWN:
        if (ctx->selected < len - 1)
            ctx->selected++;
        else
            ctx->selected = 0;
        return -1;
    case KEY_LEFT:
    case KEY_RIGHT:
        return -1;
    }

#else
    if (c == KEY_ESC) {
        char seq[2];
        readch(&seq[0]);
        readch(&seq[1]);

        switch (seq[1]) {
        case 'A': // Up Arrow
            if (ctx->selected > 0)
                ctx->selected--;
            else
                ctx->selected = len - 1;

            return -1;
        case 'B': // Down Arrow
            if (ctx->selected < len - 1)
                ctx->selected++;
            else
                ctx->selected = 0;

            return -1;
        case 'C':
        case 'D':
            return -1;
        }

        return 0;
    }
#endif

handle_c:
    switch (c) {
    case '\n':
        return -1;
    case 8:
    case 127:
        if (ctx->query.len > 0) {
            ctx->query.len--;
        }

        ctx->query.ptr[ctx->query.len] = '\0';
        break;
    default: {
        ctx->query.ptr[ctx->query.len] = c;
        ctx->query.ptr[ctx->query.len + 1] = '\0';
        ctx->query.len = ctx->query.len + 1;
    }
    }

    s_vec_search_result_free(ctx->results);
    tf_search(ctx->query.ptr, 0.3, &ctx->results);

    return -1;
}