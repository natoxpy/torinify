#include "./db.c"
#include "./sh.c"
#include "torinify/core.h"
#include "torinify/playback.h"
#include "torinify/search_engine.h"
#include <stdio.h>
#include <taglib/tag_c.h>
#include <unistd.h>

int main() {
    printf("====== [ TESTS ] ======\n");
    // db_tests();
    // sh_tests();

    tf_init();
    tf_init_db("../../mycollection.db");

    Vec *res;
    tf_search("Yearn N", 0.2, &res);
    int targetId = ((SearchResult *)vec_get(res, 0))->rowid;
    s_vec_search_result_free(res);

    Music *music;
    s_music_get(tgc->sqlite3, targetId, &music);

    Queue *q = pb_q_alloc();
    pb_add_q(tgc->playback, q);

    MusicQueue *mq = pb_musicq_alloc();

    mq->title = strdup(music->title);
    // "/home/toxpy/Downloads/broken.mp3"
    mq->fullpath = strdup(music->fullpath);
    mq->id = music->id;

    printf("AVERROR_INVALIDDATA: %d\n", AVERROR_INVALIDDATA);

    if (pb_q_add(q, mq) != T_SUCCESS) {
        pb_musicq_free(mq);
    }

    pb_q_set_volume(q, 0.1);
    pb_q_play(q);

    MusicQueue *mq1 = pb_q_get_active(q);
    char *song_name = mq1->title;

    // while (1) {
    //     printf("c %f/%f\n", pb_q_get_current_time(q), pb_q_get_duration(q));
    //     sleep(1);
    // }

    error_print_all();

    s_music_free(music);

    tf_cleanup();

    // TagLib_File *file = taglib_file_new(
    //     "/home/toxpy/Music/server/Mili/Grown-up's Paradise (2024)/Mili - "
    //     "Grown-up's Paradise - 01 - Grown-Up's Paradise.mp3");

    // TagLib_Complex_Property_Attribute ***properties =
    //     taglib_complex_property_get(file, "PICTURE");

    // TagLib_Complex_Property_Attribute ***propPtr = properties;
    // while (*propPtr) {
    //     TagLib_Complex_Property_Attribute **attrPtr = *propPtr;
    //     while (*attrPtr) {
    //         TagLib_Complex_Property_Attribute *attr = *attrPtr;
    //         switch (attr->value.type) {
    //         case TagLib_Variant_String:
    //             if (strcmp("mimeType", attr->key) == 0) {
    //                 printf("mimeType \"%s\"\n",
    //                 attr->value.value.stringValue);
    //             } else if (strcmp("description", attr->key) == 0) {
    //                 printf("Description \"%s\"\n",
    //                        attr->value.value.stringValue);
    //             } else if (strcmp("pictureType", attr->key) == 0) {
    //                 printf("pictureType \"%s\"\n",
    //                        attr->value.value.stringValue);
    //             }
    //             break;
    //         case TagLib_Variant_ByteVector:
    //             if (strcmp("data", attr->key) == 0) {
    //                 // picture->data = attr->value.value.byteVectorValue;
    //                 // picture->size = attr->value.size;
    //             }
    //             break;
    //         default:
    //             break;
    //         }
    //         ++attrPtr;
    //     }
    //     ++propPtr;
    // }

    // taglib_complex_property_free(properties);

    // taglib_file_free(file);

    return 0;
}
