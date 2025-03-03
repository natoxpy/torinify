#include "utils/levenshtein.h"
#include <math.h>
#include <stdbool.h>
#include <string.h>

void test_levenshtein(bool *passed, char **name, char **log) {
    *name = "Levenshtein";

    char *s_arr[] = {"epistemology", "anarchy"};
    char *t_arr[] = {"Phenomenology", "monarchy"};
    int distance_arr[] = {7, 2};

    for (int i = 0; i < sizeof(s_arr) / sizeof(char *); i++) {
        char *s = s_arr[i];
        char *t = t_arr[i];
        int exp_dist = distance_arr[i];
        int dist = levenshtein_distance(s, strlen(s), t, strlen(t));
        if (exp_dist != dist) {
            *passed = false;
            *log = "Expected distance and distance returned did not match";
            return;
        }
    }

    *passed = true;
}

void test_similarity_score(bool *passed, char **name, char **log) {
    *name = "Similarity Score";

    char *s_arr[] = {"epistemology", "anarchy"};
    char *t_arr[] = {"Phenomenology", "monarchy"};
    double sim_arr[] = {0.46, 0.75};

    for (int i = 0; i < sizeof(s_arr) / sizeof(char *); i++) {
        char *s = s_arr[i];
        char *t = t_arr[i];
        double exp_sim = sim_arr[i];
        double sim = round((similarity_score(s, t) * 100)) / 100;

        if (exp_sim != sim) {
            *passed = false;
            *log = "Expected similarity score and similarity score returned "
                   "did not match";
            return;
        }
    }
    *passed = true;
}

void test_word_based_similarity_score(bool *passed, char **name, char **log) {
    *name = "Word Based Similarity";

    char *s_arr[] = {"In Hell We Live, Lament", "Taste of Death"};
    char *t_arr[] = {"End of a life", "Children of the City"};
    double sim_arr[] = {0.32, 0.47};

    for (int i = 0; i < sizeof(s_arr) / sizeof(char *); i++) {
        char *s = s_arr[i];
        char *t = t_arr[i];
        double exp_sim = sim_arr[i];
        double sim = round(word_based_similarity(s, t) * 100) / 100;

        if (exp_sim != sim) {
            *passed = false;
            *log = "Expected similarity score and similarity score returned "
                   "did not match";
            return;
        }
    }

    *passed = true;
}
