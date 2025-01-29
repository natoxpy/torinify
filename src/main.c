/// Example: how to use Torinify

#include "audio/audio.h"
#include "migrations/migration.h"
#include <libavutil/dict.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <taglib/tag_c.h>
#include <torinify/core.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int ret = tf_init(); // Todo handle errors
    tf_sqlite3_init("../sqlite.db");

    ret = m_migrations(tgc->sqlite3);

    // tf_sqlite3_migrations("../migrations")

    // tf_register_pool("m")

    // Initialize a file handle for the mp3 file
    // TagLib_File *file = taglib_file_new("m/IronLotus.m4a");

    // // Check if the file was opened correctly
    // if (file != NULL) {
    //     // Retrieve the tag and audio properties
    //     TagLib_Tag *tag = taglib_file_tag(file);
    //     const TagLib_AudioProperties *properties =
    //         taglib_file_audioproperties(file);

    //     // Check if the tag is available
    //     if (tag != NULL) {
    //         printf("-- TAG (basic) --\n");
    //         printf("title   - \"%s\"\n", taglib_tag_title(tag));
    //         printf("artist  - \"%s\"\n", taglib_tag_artist(tag));
    //         printf("album   - \"%s\"\n", taglib_tag_album(tag));
    //         printf("year    - \"%u\"\n", taglib_tag_year(tag));
    //         printf("comment - \"%s\"\n", taglib_tag_comment(tag));
    //         printf("track   - \"%u\"\n", taglib_tag_track(tag));
    //         printf("genre   - \"%s\"\n", taglib_tag_genre(tag));
    //     }

    //     taglib_tag_free_strings();

    //     // Retrieve properties map and complex keys if needed
    //     // char **propertiesMap = taglib_property_keys(file);
    //     // char **complexKeys = taglib_complex_property_keys(file);

    //     // You can now do something with propertiesMap and complexKeys if
    //     needed

    //     // Free the file object to release resources
    //     taglib_file_free(file);
    // } else {
    //     printf("Error opening the file.\n");
    // }

    // Check if the file was opened correctly
    // if (file != NULL) {
    //     // Retrieve the tag
    //     TagLib_Tag *tag = taglib_file_tag(file);

    //     // Check if the tag is available
    //     if (tag != NULL) {
    //         // Print existing tag information (optional)
    //         printf("Original title: %s\n", taglib_tag_title(tag));

    //         // Modify the tag fields
    //         taglib_tag_set_title(tag, "New Title");
    //         taglib_tag_set_artist(tag, "New Artist");
    //         taglib_tag_set_album(tag, "New Album");
    //         taglib_tag_set_year(tag, 2025);
    //         taglib_tag_set_comment(tag, "This is a comment");
    //         taglib_tag_set_track(tag, 5);
    //         taglib_tag_set_genre(tag, "New Genre");

    //         // Save the changes to the file
    //         if (taglib_file_save(file)) {
    //             printf("Tags saved successfully!\n");
    //         } else {
    //             printf("Error saving tags to file.\n");
    //         }
    //     }

    //     taglib_tag_free_strings();

    //     // Free the file object to release resources
    //     taglib_file_free(file);
    // } else {
    //     printf("Error opening the file.\n");
    // }

    // AVFormatContext *fmt_ctx = NULL;
    // if (avformat_open_input(&fmt_ctx, "m/IronLotus.mp3", NULL, NULL) < 0) {
    //     printf("Failed to open file\n");
    //     return -1;
    // }

    // AVDictionary *meta = fmt_ctx->metadata;

    // av_dict_set(&meta, "artist", "Artist1", AV_DICT_APPEND);

    // if (avformat_write_header(fmt_ctx, NULL) < 0) {
    //     printf("Failed to write header.\n");
    //     return -1;
    // }

    // avformat_close_input(&fmt_ctx);

    // int ret = tf_init(); // Todo handle errors
    // tf_sqlite3_init("../sqlite.db");

    // uint8_t *data;
    // size_t size = f_read_file("m/IronLotus.wav", &data);

    // if (size < 0) {
    //     free(data);
    //     goto end;
    // }

    // AAudioContext *audio_ctx;
    // if (a_audio_context_init(data, size, &audio_ctx) != 0)
    //     fprintf(stderr, "audio context init");

    // audio_ctx->fmt_ctx->metadata;

    // a_audio_free_context(audio_ctx);

    /*c Play A Song
    ```
    SongID *song_id = tf_register("m/IronLotus.wav");

    PlaybackQueue *playback_queue = tf_playback_queue_init();

    tf_add_to_queue(playback_queue, song_id);

    tf_add_playback_queue(playback_queue);

    tf_set_volume(0.5);

    tf_play_head_queue();
    ```
    */

    /*c Play A Song
        ```
    SongID *song_id = tf_register("m/IronLotus.wav");

    SongMetadata *metadata = tf_read_song_metadata(song_id);

    // Metadata library
    mdl_set_title(metadata, "Hello world");
    mdl_set_artist(metadata, "Mili");
    mdl_add_tag(metadata, "Rock");

    tf_write_song_metadata(metadata);
    ```
    */

    // uint8_t *data;
    // size_t size = f_read_file("m/IronLotus.wav", &data);

    // if (size < 0) {
    //     free(data);
    //     goto end;
    // }

    // AAudioContext *audio_ctx;
    // if (a_audio_context_init(data, size, &audio_ctx) != 0)
    //     fprintf(stderr, "audio context init");

    // AAudioVector *au_vec;
    // if (a_audio_decode(audio_ctx, &au_vec) != 0)
    //     fprintf(stderr, "audio could not be decoded");

    // a_audio_free_context(audio_ctx);

end:
    // a_audio_vector_free(au_vec);

    tf_cleanup();

    // if (ret < 0) {
    //     fprintf(stderr, "Program failed!");
    //     return -1;
    // }

    // sqlite3 *db;

    // sqlite3_open("../sqlite.db", &db);

    // sqlite3_close(db);

    // if (argc <= 1) {
    //     printf("usage: %s <file>\n", argv[0]);
    //     return 0;
    // }

    // cl_init();
    // cl_set_audio_file(argv[1]);
    // while (true) {
    //     int cmd;
    //     print_cmd();
    //     scanf("%d", &cmd);
    //     if (cmd == 0) {
    //         break;
    //     }
    //     switch (cmd) {
    //     case 1:
    //         printf("current time: %f / %f\n",
    //                (float)cl_get_current_time() / 1000,
    //                (float)cl_get_duration() / 1000);
    //         break;
    //     case 2:
    //         printf("paused: %s \n", cl_get_paused() ? "true" : "false");
    //         break;
    //     case 3:
    //         printf("volume: %f\n", cl_get_volume());
    //         break;
    //     case 4:
    //         cl_pause();
    //         printf("paused: %s \n", cl_get_paused() ? "true" : "false");
    //         break;
    //     case 5:
    //         cl_play();
    //         printf("paused: %s \n", cl_get_paused() ? "true" : "false");
    //         break;
    //     case 6: {
    //         unsigned int ct = cl_get_current_time();
    //         unsigned int dur = cl_get_duration();
    //         if (dur >= ct + 5 * 1000) {
    //             cl_set_current_time(ct + 5 * 1000);
    //             printf("current time: %f / %f\n",
    //                    (float)cl_get_current_time() / 1000,
    //                    (float)cl_get_duration() / 1000);
    //         } else
    //             printf("Cannot advance 5s\n");

    //         break;
    //     }
    //     case 7: {
    //         unsigned int ct = cl_get_current_time();

    //         if (ct >= 5 * 1000) {
    //             ct -= 5 * 1000;
    //             cl_set_current_time(ct);

    //             printf("current time: %f / %f\n",
    //                    (float)cl_get_current_time() / 1000,
    //                    (float)cl_get_duration() / 1000);
    //         } else {

    //             printf("Cannot go back 5s\n");
    //         }

    //         break;
    //     }

    //     case 8: {

    //         float vol = cl_get_volume();

    //         if (vol > 1.0f - 0.1f) {
    //             vol = 1.0f;
    //         } else {
    //             vol += 0.1f;
    //         }

    //         cl_set_volume(vol);

    //         printf("volume: %f\n", cl_get_volume());
    //         break;
    //     }

    //     case 9: {
    //         float vol = cl_get_volume();

    //         if (vol <= 0.1) {

    //             vol = 0.0;
    //         } else {
    //             vol -= 0.1;
    //         }

    //         cl_set_volume(vol);

    //         printf("volume: %f\n", cl_get_volume());
    //         break;
    //     }
    //     }
    // }

    // cl_cleanup();

    return 0;
}
