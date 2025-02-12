#include "utils/generic_vec.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <storage/music.h>
#include <string.h>
#include <time.h>
#include <torinify/search_engine.h>
#include <utils/levenshtein.h>

typedef struct {
    Vec *search_ctxs;
    char *query;
    double threshold;
} ThreadProcessArg;

void *thread_compute_search_process(void *search_ctx_data);

SearchContext *s_search_context_alloc() {
    SearchContext *search_ctx = malloc(sizeof(SearchContext));

    if (search_ctx == NULL)
        return NULL;

    search_ctx->rowid = -1;
    search_ctx->title = "";
    search_ctx->album = "";
    search_ctx->alt_titles = NULL;
    search_ctx->artists = NULL;

    return search_ctx;
}

// TODO: Implement properly
void s_search_context_free(SearchContext *search_ctx) {
    if (search_ctx == NULL)
        return;

    // TODO: Implement proper vector free

    free(search_ctx);
}

SearchResult *s_search_result_alloc() {
    SearchResult *result = malloc(sizeof(SearchResult));
    if (!result)
        return NULL;

    result->rowid = -1;
    result->distance = 0;
    result->title = NULL;

    return result;
}

void s_search_result_free(SearchResult *search_result) {
    if (!search_result)
        return;

    if (search_result->title)
        free(search_result->title);

    free(search_result);
}

void s_vec_search_context_init(sqlite3 *db, Vec **search_ctx_out) {
    Vec *musics;
    s_music_get_all(db, &musics);

    Vec *search_ctx = vec_init(sizeof(SearchContext));

    for (int i = 0; i < musics->length; i++) {
        Music *music = vec_get(musics, i);
        SearchContext sctx = {music->id, music->title, NULL, NULL, NULL};

        vec_push(search_ctx, &sctx);

        music->title = NULL;
    }

    *search_ctx_out = search_ctx;

    s_vec_music_free(musics);
}

void s_vec_search_context_free(Vec *search_ctx_vec) {
    if (search_ctx_vec == NULL)
        return;

    if (search_ctx_vec->data) {
        for (int i = 0; i < search_ctx_vec->length; i++) {
            SearchContext *search_ctx = vec_get(search_ctx_vec, i);

            free(search_ctx->title);
        }
    }

    vec_free(search_ctx_vec);
}

void s_vec_search_result_free(Vec *results) {
    if (!results)
        return;

    if (results->data) {
        for (int i = 0; i < results->length; i++) {
            SearchResult *result = vec_get(results, i);

            free(result->title);
        }
    }

    vec_free(results);
}

double process_search(char *query, SearchContext *ctx) {
    return word_based_similarity(ctx->title, query);
}

int result_compare(const void *a, const void *b) {
    SearchResult *as = (void *)a;
    SearchResult *bs = (void *)b;

    if (as->distance < bs->distance)
        return 1;
    else
        return -1;
}

void sort_result_vec(Vec *results) {
    qsort(results->data, results->length, results->element_size,
          result_compare);
}

void s_process_search(Vec *search_ctx, char *query, Vec **out_results,
                      double threshold) {
    ThreadProcessArg params = {search_ctx, query, threshold};
    *out_results = thread_compute_search_process(&params);
    sort_result_vec(*out_results);
}

void vec_split(Vec *src, Vec **dest_out, Vec **dest2_out) {
    int dest_len = src->length / 2;
    int dest2_len = src->length - dest_len;

    Vec *dest = vec_init_with_capacity(src->element_size, dest_len);
    Vec *dest2 = vec_init_with_capacity(src->element_size, dest2_len);

    memcpy(dest->data, src->data, dest_len * src->element_size);
    memcpy(dest2->data, src->data + (dest_len * src->element_size),
           dest2_len * src->element_size);

    dest->length = dest_len;
    dest2->length = dest2_len;

    *dest_out = dest;
    *dest2_out = dest2;
}

void *thread_compute_search_process(void *search_ctx_data) {

    ThreadProcessArg *params = search_ctx_data;

    Vec *search_ctx = params->search_ctxs;
    char *query = params->query;
    double threshold = params->threshold;

    Vec *results =
        vec_init_with_capacity(sizeof(SearchResult), search_ctx->length);

    for (int i = 0; i < search_ctx->length; i++) {
        SearchContext *ctx = vec_get(search_ctx, i);

        double comfy = process_search(query, ctx);

        if (comfy < threshold)
            continue;

        SearchResult result = {ctx->rowid, comfy, strdup(ctx->title)};

        vec_push(results, &result);
    }

    return results;
}

void s_process_search_multi(Vec *search_ctx, char *query, Vec **out_results,
                            double threshold, int threads) {
    Vec *all_results =
        vec_init_with_capacity(sizeof(SearchResult), search_ctx->length);

    Vec *all_ctx[threads];
    vec_n_split(search_ctx, all_ctx, threads);

    pthread_t threads_id[threads];
    ThreadProcessArg *params[threads];
    Vec *results[threads];

    for (int i = 0; i < threads; i++) {
        ThreadProcessArg *param = malloc(sizeof(ThreadProcessArg));

        pthread_t thread_id;

        param->search_ctxs = all_ctx[i];
        param->query = query;
        param->threshold = threshold;

        pthread_create(&thread_id, NULL, thread_compute_search_process,
                       (void *)param);

        threads_id[i] = thread_id;
        params[i] = param;
    }

    for (int i = 0; i < threads; i++) {
        Vec *result;
        pthread_join(threads_id[i], (void **)&result);

        results[i] = result;
        free(params[i]);
    }

    for (int i = 0; i < threads; i++) {
        Vec *result = results[i];

        vec_join(all_results, result);
        vec_free(result);
    }

    sort_result_vec(all_results);

    *out_results = all_results;

cleanup:
    for (int i = 0; i < threads; i++) {
        Vec *ctx_vec = all_ctx[i];
        vec_free(ctx_vec);
    }
}
