// #include <audio/audio.h>
// #include <libavformat/avformat.h>
// #include <libavutil/error.h>
// #include <stdint.h>

// AAudioContext *a_audio_alloc_context() {
//     AAudioContext *audio_ctx = malloc(sizeof(AAudioContext));
//
//     if (!audio_ctx)
//         return NULL;
//
//     audio_ctx->fmt_ctx = NULL;
//     audio_ctx->codec_ctx = NULL;
//     audio_ctx->avio_ctx = NULL;
//     audio_ctx->stream_index = -1;
//
//     return audio_ctx;
// }
//
// void a_audio_free_context(AAudioContext *audio_ctx) {
//     if (!audio_ctx)
//         return;
//
//     if (audio_ctx->fmt_ctx != NULL)
//         avformat_close_input(&audio_ctx->fmt_ctx);
//
//     if (audio_ctx->codec_ctx != NULL)
//         avcodec_free_context(&audio_ctx->codec_ctx);
//
//     if (audio_ctx->avio_ctx) {
//         av_free(&audio_ctx->avio_ctx->buffer);
//         avio_context_free(&audio_ctx->avio_ctx);
//     }
//
//     free(audio_ctx);
// }
//
// static int read_packet(void *opaque, uint8_t *buf, int buf_size) {
//     AAudioVector *bd = (AAudioVector *)opaque;
//     buf_size = FFMIN(buf_size, (int)bd->length);
//
//     if (!buf_size)
//         return AVERROR_EOF;
//
//     printf("ptr:%p size:%zu\n", bd->ptr, bd->length);
//
//     /* copy internal buffer data to buf */
//     memcpy(buf, bd->ptr, buf_size);
//     bd->ptr += buf_size;
//     bd->length -= buf_size;
//
//     return buf_size;
// }
//
// AVIOContext *attach_avio_to_fmt_ctx(AVFormatContext **_fmt_ctx,
//                                     AAudioVector *au_vec) {
//     int ret = 0;
//
//     AVFormatContext *fmt_ctx = avformat_alloc_context();
//
//     if (fmt_ctx == NULL) {
//         ret = -1;
//         goto end;
//     }
//
//     int avio_ctx_buffer_size = 4096;
//     uint8_t *avio_ctx_buffer = av_malloc(avio_ctx_buffer_size);
//
//     // if (!avio_ctx_buffer) {
//     //     ret = -1;
//     //     goto end;
//     // }
//
//     AVIOContext *avio_ctx;
//
//     if (!(avio_ctx = avio_alloc_context(avio_ctx_buffer,
//     avio_ctx_buffer_size,
//                                         0, au_vec, &read_packet, NULL,
//                                         NULL))) {
//         ret = -1;
//         goto end;
//     }
//
//     fmt_ctx->pb = avio_ctx;
//
//     if ((ret = avformat_open_input(&fmt_ctx, NULL, NULL, NULL)) < 0)
//         goto end;
//
//     if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
//         goto end;
//     }
// end:
//     if (ret < 0) {
//         return NULL;
//     }
//
//     return avio_ctx;
// }

// int codec_info(AVFormatContext *fmt_ctx, int *out_index_size,
//                AVCodecContext **codec_ctx) {
//     int ret = 0;
//     const AVCodec *dec;
//
//     int stream_index =
//         av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &dec, 0);
//
//     if (stream_index < 0) {
//         return -1;
//     }
//
//     AVCodecContext *dec_ctx;
//     if (!(dec_ctx = avcodec_alloc_context3(dec))) {
//         return -1;
//     }
//
//     avcodec_parameters_to_context(dec_ctx,
//                                   fmt_ctx->streams[stream_index]->codecpar);
//
//     if ((ret = avcodec_open2(dec_ctx, dec, NULL)) < 0) {
//         return -1;
//     }
//
//     *codec_ctx = dec_ctx;
//     *out_index_size = stream_index;
//
//     return 0;
// }

/// `in_data` won't be free after copying to `audio_ctx`
/// return less than zero indicate errors
// int a_audio_context_init(uint8_t *in_data, size_t in_size,
//                          AAudioContext **audio_ctx) {
//     // printf("gura %zu\n", in_size);
//
//     AAudioContext *au_ctx = a_audio_alloc_context();
//     AVFormatContext *fmt_ctx = NULL;
//
//     // AAudioVector *au_vec = malloc(sizeof(AAudioVector));
//     // au_vec->length = in_size;
//     // au_vec->ptr = in_data;
//
//     // au_ctx->avio_ctx = attach_avio_to_fmt_ctx(&fmt_ctx, au_vec);
//
//     // free(au_vec);
//
//     *audio_ctx = au_ctx;
//
//     // AAudioVector *au_vec = malloc(sizeof(AAudioVector));
//     // au_vec->length = in_size;
//     // au_vec->ptr = in_data;
//
//     // au_ctx->avio_ctx = attach_avio_to_fmt_ctx(&fmt_ctx, au_vec);
//
//     // free(au_vec);
//
//     // if (au_ctx->avio_ctx == NULL)
//     //     return -1;
//
//     // AVCodecContext *codec_ctx = NULL;
//
//     // codec_info(fmt_ctx, &au_ctx->stream_index, &codec_ctx);
//
//     // au_ctx->fmt_ctx = fmt_ctx;
//
//     // au_ctx->codec_ctx = codec_ctx;
//
//     // *audio_ctx = au_ctx;
//
//     // --- PEAK ---
//
//     // int ret = 0;
//     // AVFormatContext *fmt_ctx = avformat_alloc_context();
//     // AAudioContext *au_ctx = NULL;
//
//     // if (!(au_ctx = a_audio_alloc_context()))
//     //     return -1;
//
//     // // buffer_data bd = {in_data, in_size};
//
//     // AAudioVector au_vec = {in_data, in_size, in_size};
//
//     // int avio_ctx_buffer_size = 4096;
//     // uint8_t *avio_ctx_buffer = av_malloc(avio_ctx_buffer_size);
//
//     // AVIOContext *avio_ctx =
//     //     avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 0,
//     &au_vec,
//     //                        &read_packet, NULL, NULL);
//
//     // fmt_ctx->pb = avio_ctx;
//
//     // if (avformat_open_input(&fmt_ctx, NULL, NULL, NULL) < 0) {
//     //     fprintf(stderr, "Could not open input\n");
//     // }
//
//     // if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
//     //     fprintf(stderr, "Could not open input\n");
//     // }
//
//     // au_ctx->fmt_ctx = fmt_ctx;
//
//     // if (avio_ctx)
//     //     av_freep(&avio_ctx->buffer);
//
//     // avio_context_free(&avio_ctx);
//
//     // // Dec
//
//     // const AVCodec *dec;
//
//     // int stream_index =
//     //     av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &dec, 0);
//
//     // if (stream_index < 0) {
//     // }
//
//     // AVCodecContext *dec_ctx = avcodec_alloc_context3(dec);
//     // if (!dec_ctx) {
//     //     return AVERROR(ENOMEM);
//     // }
//
//     // avcodec_parameters_to_context(dec_ctx,
//     // fmt_ctx->streams[stream_index]->codecpar);
//
//     // if ((ret = avcodec_open2(dec_ctx, dec, NULL)) < 0) {
//     //     av_log(NULL, AV_LOG_ERROR, "Cannon open audio decoder \n");
//     //     return ret;
//     // }
//
//     // au_ctx->codec_ctx = dec_ctx;
//     // au_ctx->stream_index = stream_index;
//
//     // *audio_ctx = au_ctx;
//
//     return 0;
// }
