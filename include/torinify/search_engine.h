#ifndef _TORINIFY_SEARCH_ENGINE_H
#define _TORINIFY_SEARCH_ENGINE_H
#include <sqlite3.h>
#include <utils/generic_vec.h>

typedef struct {
    uint32_t rowid;
    char *title;
    char *album;
    Vec *alt_titles;
    Vec *artists;
} SearchContext;

typedef struct {
    uint32_t rowid;
    double distance;
    char *title;
} SearchResult;

SearchContext *s_search_context_alloc();
void s_search_context_free(SearchContext *search_ctx);

SearchResult *s_search_result_alloc();
void s_search_result_free(SearchResult *search_result);

/// `search_ctx` vec contains type `SearchContext`
void s_vec_search_context_init(sqlite3 *db, Vec **search_ctx);
void s_vec_search_context_free(Vec *search_ctx);
void s_vec_search_result_free(Vec *results);

/// `out_results` vec contains type `SearchResults`
void s_process_search(Vec *search_ctx, char *query, Vec **out_results,
                      double threshold);

void s_process_search_multi(Vec *search_ctx, char *query, Vec **out_results,
                            double threshold, int threads);

#endif
