#include "utils/str.h"
#include <locale.h>
#include <string.h>

char *ltrim(char *s) {
    if (s == NULL || *s == '\0')
        return s;

    while (*s == ' ')
        s++;
    return s;
}

char *rtrim(char *s) {
    if (s == NULL || *s == '\0')
        return s;

    char *back = s + strlen(s) - 1;

    while (*back == ' ') {
        back--;
    }

    *(back + 1) = '\0';

    return s;
}

char *trim(char *s) { return rtrim(ltrim(s)); }

// char *trim(char *str) {
//     size_t len = 0;
//     char *frontp = str;
//     char *endp = NULL;
//
//     if (str == NULL) {
//         return NULL;
//     }
//     if (str[0] == '\0') {
//         return str;
//     }
//
//     len = strlen(str);
//     endp = str + len;
//
//     /* Move the front and back pointers to address the first non-whitespace
//      * characters from each end.
//      */
//     while (isspace((unsigned char)*frontp)) {
//         ++frontp;
//     }
//     if (endp != frontp) {
//         while (isspace((unsigned char)*(--endp)) && endp != frontp) {
//         }
//     }
//
//     if (frontp != str && endp == frontp) {
//         // Empty string
//         *(isspace((unsigned char)*endp) ? str : (endp + 1)) = '\0';
//     } else if (str + len - 1 != endp)
//         *(endp + 1) = '\0';
//
//     /* Shift the string so that it starts at str so that if it's dynamically
//      * allocated, we can still free it on the returned pointer.  Note the
//      reuse
//      * of endp to mean the front of the string buffer now.
//      */
//     endp = str;
//     if (frontp != str) {
//         while (*frontp) {
//             *endp++ = *frontp++;
//         }
//         *endp = '\0';
//     }
//
//     return str;
// }

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

/// after printing all the character it continue to print all the whitespaces
/// left
void print_with_blanks(char *s, char blank, size_t limit) {
    int width = print_until_limit(s, limit);

    for (int i = 0; i < limit - width; i++) {
        printf("%c", blank);
    }
}
