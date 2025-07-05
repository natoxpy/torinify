#include "utils/str.h"
#include <locale.h>

int utf8_display_width(const char *s) {
    setlocale(LC_CTYPE, "");
    mbstate_t st = {0};
    wchar_t wc;
    size_t len;
    int total_width = 0;

    while ((len = mbrtowc(&wc, s, MB_CUR_MAX, &st)) > 0) {
        int w = wcwidth(wc);
        if (w < 0)
            w = 0;
        total_width += w;
        s += len;
    }
    return total_width;
}

int utf8_str_get(const char *s, char *s_out, size_t index) {
    mbstate_t st = {0};
    wchar_t wc;
    size_t len;
    size_t s_index = 0;

    while ((len = mbrtowc(&wc, s, MB_CUR_MAX, &st)) > 0) {
        if (s_index == index) {
            int l = wcrtomb(s_out, wc, &st);
            if (l < 0)
                return -1;
            s_out[l] = '\0';
            return l;
        }

        s_index += 1;
        s += len;
    }

    return -1;
}

/// @returns number of characters printed
int print_until_limit(char *s, size_t limit) {
    int width_accumulated = 0;
    int string_width = utf8_display_width(s);

    for (int y = 0; y < string_width; y++) {
        char mb_char[15];
        utf8_str_get(s, mb_char, y);
        int temporary_width = utf8_display_width(mb_char);
        if (width_accumulated + temporary_width > limit)
            break;

        width_accumulated += temporary_width;
        printf("%s", mb_char);
    }

    return width_accumulated;
}

