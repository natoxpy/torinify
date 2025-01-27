// #include <libavutil/samplefmt.h>
// #include <priv/playback.h>
// #include <priv/simple_playback.h>
// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>

// int cl_init() {
//      g_pb_ctx = cl_playback_alloc();
//     return 0;
// }
//
// void cl_cleanup() {
//     if (!g_pb_ctx)
//         return;
//
//     ma_device_uninit(g_pb_ctx->ma_device);
//     free(g_pb_ctx->ma_device);
//     cl_playback_ready_context_free(g_pb_ctx->ctx);
//     free(g_pb_ctx);
// }
//
// int cl_set_audio_file(char *filename) {
//     int ret = 0;
//     FileContext *file_ctx;
//     ret = cl_open_input_file(&file_ctx, filename);
//
//     if (ret < 0) {
//         fprintf(stderr, "Unable to Open File\n");
//         return 0;
//     }
//
//     DecodedFileContext *decoded_ctx;
//
//     ret = cl_decoded_file_ctx(file_ctx, &decoded_ctx);
//
//     if (ret < 0) {
//         fprintf(stderr, "Unable to Decode File\n");
//         return 0;
//     }
//
//     PlaybackReadyContext *pb_ready_ctx =
//         cl_playback_ready_context_init(file_ctx, decoded_ctx);
//
//     g_pb_ctx->ctx = pb_ready_ctx;
//
//     unsigned int mili_sample_rate = g_pb_ctx->ctx->sample_rate / 1000;
//
//     g_pb_ctx->duration =
//         (unsigned int)(g_pb_ctx->ctx->nb_samples / mili_sample_rate);
//
//     cl_playback(g_pb_ctx);
//
//     g_pb_ctx->paused = false;
//
//     return 0;
// }
//
// void cl_set_current_time(unsigned int miliseconds) {
//     cl_set_audio_current_time(g_pb_ctx, miliseconds);
// }
//
// void cl_play() {
//     cl_start_device(g_pb_ctx->ma_device);
//     g_pb_ctx->paused = false;
// }
// void cl_pause() {
//     cl_stop_device(g_pb_ctx->ma_device);
//     g_pb_ctx->paused = true;
// }
//
// bool cl_get_paused() { return g_pb_ctx->paused; }
//
// unsigned int cl_get_current_time() {
//     unsigned int mili_sample_rate = g_pb_ctx->ctx->sample_rate / 1000;
//     return g_pb_ctx->samples_consumed / mili_sample_rate;
// }
//
// unsigned int cl_get_duration() { return g_pb_ctx->duration; }
//
// float cl_get_volume() { return g_pb_ctx->volume; }
//
// void cl_set_volume(float volume) { g_pb_ctx->volume = volume; }
