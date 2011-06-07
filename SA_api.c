#include "SA_api.h"

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>

#include <stdio.h>
#include <stdlib.h>

#define  TRUE     1
#define  FALSE    0

int SA_init(void)
{
     av_register_all();
     return 0; // FIXME
}

SAContext *SA_open(char *filename)
{
     int v_stream = -1, a_stream = -1, i;
     AVFormatContext *avfmt_ctx_ptr = NULL;
     AVCodecContext *v_codec_ctx = NULL, *a_codec_ctx = NULL;
     AVCodec *v_codec = NULL, *a_codec = NULL;

     /* allocating space for the context and temporary frame */
     SAContext *ctx_p = (SAContext *) malloc(sizeof(SAContext));
     if(ctx_p == NULL)
          goto OPEN_FAIL;
     ctx_p->vq_ctx = ctx_p->aq_ctx = NULL;
     ctx_p->avfmt_ctx_ptr = NULL;
     ctx_p->v_frame_t = NULL;
     ctx_p->audio_eof = ctx_p->video_eof = FALSE;
     
     /* init the queue for storing decoder's output */
     ctx_p->filename = filename;
     ctx_p->vq_ctx = SAQ_init();
     ctx_p->aq_ctx = SAQ_init();
     if(ctx_p->vq_ctx == NULL || ctx_p->aq_ctx == NULL)
          goto OPEN_FAIL;

     /* opening the file */
     if(av_open_input_file(&avfmt_ctx_ptr, filename, NULL, 0, NULL) != 0)
          goto OPEN_FAIL;
     ctx_p->avfmt_ctx_ptr = avfmt_ctx_ptr;
     if(av_find_stream_info(avfmt_ctx_ptr) < 0)
          goto OPEN_FAIL;

     /* FIXME: debug */
     dump_format(avfmt_ctx_ptr, 0, filename, 0);

     /* getting the video stream */
     for(i = 0; i < avfmt_ctx_ptr->nb_streams; i++)
          if(avfmt_ctx_ptr->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
               break;
     if(i == avfmt_ctx_ptr->nb_streams)
          goto OPEN_FAIL;
     ctx_p->v_stream = v_stream = i;

     /* getting the audio stream */
     for(i = 0; i < avfmt_ctx_ptr->nb_streams; i++)
          if(avfmt_ctx_ptr->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO)
               break;
     if(i == avfmt_ctx_ptr->nb_streams)
          goto OPEN_FAIL;
     ctx_p->a_stream = a_stream = i;

     // FIXME: setting discard
     for(i = 0; i < avfmt_ctx_ptr->nb_streams; i++)
          avfmt_ctx_ptr->streams[i]->discard = AVDISCARD_ALL;
     avfmt_ctx_ptr->streams[a_stream]->discard = AVDISCARD_DEFAULT;
     avfmt_ctx_ptr->streams[v_stream]->discard = AVDISCARD_DEFAULT;

     /* getting the codec */
     ctx_p->v_codec_ctx = v_codec_ctx = avfmt_ctx_ptr->streams[v_stream]->codec;
     ctx_p->v_codec = v_codec = avcodec_find_decoder(v_codec_ctx->codec_id);
     ctx_p->a_codec_ctx = a_codec_ctx = avfmt_ctx_ptr->streams[a_stream]->codec;
     ctx_p->a_codec = a_codec = avcodec_find_decoder(a_codec_ctx->codec_id);
     if(v_codec == NULL || a_codec == NULL)
     {
          fprintf(stderr, "Unsupported codec!\n");
          goto OPEN_FAIL;
     }

     /* set audio_st and video_st */
     ctx_p->audio_st = avfmt_ctx_ptr->streams[a_stream];
     ctx_p->video_st = avfmt_ctx_ptr->streams[v_stream];
     
     /* FIXME: downmix, but seems like a dirty hack. */
     ctx_p->a_codec_ctx->request_channels = FFMIN(2, ctx_p->a_codec_ctx->channels);

     if(avcodec_open(v_codec_ctx, v_codec) < 0 ||
        avcodec_open(a_codec_ctx, a_codec) < 0)
          goto OPEN_FAIL;

     /* set our userdata for calculating PTS */
     v_codec_ctx->get_buffer = _SA_get_buffer;
     v_codec_ctx->release_buffer = _SA_release_buffer;
     v_codec_ctx->opaque = (uint64_t *)malloc(sizeof(uint64_t));
     if(v_codec_ctx->opaque == NULL)
     {
          fprintf(stderr, "failed to allocate space for v_codec_ctx->opaque!\n");
          goto OPEN_FAIL;
     }
     *(uint64_t *)(v_codec_ctx->opaque) = AV_NOPTS_VALUE;
     
     /* setting other useful variables */
     ctx_p->v_width = ctx_p->v_codec_ctx->width;
     ctx_p->v_height = ctx_p->v_codec_ctx->height;
     ctx_p->video_clock = 0.0f;

     /* create the mutex needed to lock _SA_decode_packet(). */
     ctx_p->decode_lock = SDL_CreateMutex();

     /* evenything is ok. return the context. */
     return ctx_p;
     
OPEN_FAIL:
     SA_close(ctx_p);
     return NULL;
}

void SA_close(SAContext *sa_ctx)
{
     if(sa_ctx == NULL)
          return;
     if(sa_ctx->v_codec_ctx != NULL)
     {
          if(sa_ctx->v_codec_ctx->opaque != NULL)
               free(sa_ctx->v_codec_ctx->opaque);
          avcodec_close(sa_ctx->v_codec_ctx);
     }
     if(sa_ctx->a_codec_ctx != NULL)
          avcodec_close(sa_ctx->a_codec_ctx);
     if(sa_ctx->avfmt_ctx_ptr != NULL)
          av_close_input_file(sa_ctx->avfmt_ctx_ptr);
     
     void *ptr;
     if(sa_ctx->vq_ctx != NULL)
     {
          while((ptr = SAQ_pop(sa_ctx->vq_ctx)) != NULL)
          {
               av_free(((SAVideoPacket *)ptr)->frame_ptr);
               free(ptr);
          }
          free(sa_ctx->vq_ctx);
     }
     if(sa_ctx->aq_ctx != NULL)
     {
          while((ptr = SAQ_pop(sa_ctx->aq_ctx)) != NULL)
          {
               av_free(((SAAudioPacket *)ptr)->abuffer);
               free(ptr);
          }
          free(sa_ctx->aq_ctx);
     }

     if(sa_ctx->v_frame_t != NULL)
          av_free(sa_ctx->v_frame_t);

     if(sa_ctx->decode_lock != NULL)
          SDL_DestroyMutex(sa_ctx->decode_lock);
     
     free(sa_ctx);
     return;
}

SAVideoPacket *SA_get_vp(SAContext *sa_ctx)
{
     SAVideoPacket *ret = NULL;
     while((ret = SAQ_pop(sa_ctx->vq_ctx)) == NULL)
          if(_SA_decode_packet(sa_ctx) < 0)
               return NULL;
     return ret;
}

SAAudioPacket *SA_get_ap(SAContext *sa_ctx)
{
     SAAudioPacket *ret = NULL;
     while((ret = SAQ_pop(sa_ctx->aq_ctx)) == NULL)
          if(_SA_decode_packet(sa_ctx) < 0)
               return NULL;
     return ret;
}

void SA_seek(SAContext *sa_ctx, double seek_to, double delta)
{
     SDL_mutexP(sa_ctx->decode_lock);
     
     /* avformat_seek_file(); */
     /*
     int64_t pos = seek_to * AV_TIME_BASE; // is->seek_pos
     int64_t incr = delta * AV_TIME_BASE; // is->seek_rel
     int seek_flags = 0; // FIXME: seek_flags? 0?
     int64_t seek_min = incr > 0 ? pos - incr + 2 : INT64_MIN;
     int64_t seek_max = incr < 0 ? pos - incr - 2 : INT64_MAX; // FIXME: "+- 2"?
     if(avformat_seek_file(sa_ctx->avfmt_ctx_ptr, -1, seek_min, pos, seek_max, seek_flags) < 0)
          fprintf(stderr, "Error while seeking!\n"); // FIXME
     */
     
     /* av_seek_frame(); */
     int64_t pos = seek_to * AV_TIME_BASE;
     int seek_flags = delta < 0 ? AVSEEK_FLAG_BACKWARD : 0;
     int stream_index = sa_ctx->v_stream;
     int64_t seek_target = av_rescale_q(pos, AV_TIME_BASE_Q,
                                        sa_ctx->video_st->time_base);
     if(av_seek_frame(sa_ctx->avfmt_ctx_ptr, stream_index, seek_target, seek_flags) < 0)
          fprintf(stderr, "Error while seeking!\n"); // FIXME
     else
     {
          void *ptr;
          while((ptr = SAQ_pop(sa_ctx->vq_ctx)) != NULL)
          {
               av_free(((SAVideoPacket *)ptr)->frame_ptr);
               free(ptr);
          }
          while((ptr = SAQ_pop(sa_ctx->aq_ctx)) != NULL)
          {
               av_free(((SAAudioPacket *)ptr)->abuffer);
               free(ptr);
          }

          avcodec_flush_buffers(sa_ctx->a_codec_ctx);
          avcodec_flush_buffers(sa_ctx->v_codec_ctx);
     }

     sa_ctx->video_clock = seek_to;
     
     SDL_mutexV(sa_ctx->decode_lock);
}

int _SA_decode_packet(SAContext *sa_ctx)
{
     /* use mutex to lock this func. */
     SDL_mutexP(sa_ctx->decode_lock);

     void *ret = NULL;
     AVPacket packet;
     if(av_read_frame(sa_ctx->avfmt_ctx_ptr, &packet) < 0)
     {
          SDL_mutexV(sa_ctx->decode_lock);
          return -1;
     }
     
     if(packet.stream_index == sa_ctx->v_stream)
     {
          SAVideoPacket *sa_vp_ret;
          uint64_t t_pts;
          int frame_finished;
          AVFrame *v_frame = sa_ctx->v_frame_t;
          if(v_frame == NULL)
               v_frame = sa_ctx->v_frame_t = avcodec_alloc_frame();

          *(uint64_t *)(sa_ctx->v_codec_ctx->opaque) = packet.pts;
          
/*          avcodec_decode_video2(sa_ctx->v_codec_ctx, v_frame,
            &frame_finished, &packet);*/
          if(avcodec_decode_video2(sa_ctx->v_codec_ctx, v_frame, &frame_finished, &packet) <= 0)
               printf("Wow!\n");
          
          if(frame_finished)
          {
               ret = malloc(sizeof(SAVideoPacket));
               sa_vp_ret = (SAVideoPacket *)ret;
               if(ret == NULL)
               {
                    printf("malloc failed\n");
                    goto DECODE_FAILED; // FIXME: decoding error report
               }
               sa_vp_ret->frame_ptr = v_frame;

               /* calculate pts */
               if(packet.dts != AV_NOPTS_VALUE)
                    t_pts = packet.dts;
               else if(v_frame->opaque != NULL &&
                       *(uint64_t *)(v_frame->opaque) != AV_NOPTS_VALUE)
                    t_pts = *(uint64_t *)(v_frame->opaque);
               else
                    t_pts = 0;
               
               if(t_pts != 0)
                    sa_vp_ret->pts = t_pts * av_q2d(sa_ctx->video_st->time_base);
               else
                    sa_vp_ret->pts = sa_ctx->video_clock;
               
               double frame_delay = av_q2d(sa_ctx->video_st->codec->time_base);
               frame_delay += v_frame->repeat_pict * (frame_delay * 0.5);
               sa_ctx->video_clock = sa_vp_ret->pts + frame_delay;

               /* pts got. push it. */
               SAQ_push(sa_ctx->vq_ctx, ret);
               sa_ctx->v_frame_t = NULL;
          }
          av_free_packet(&packet);
     } else if(packet.stream_index == sa_ctx->a_stream)
     {
          AVPacket pkt_t;
          av_init_packet(&pkt_t);
          pkt_t.data = packet.data;
          pkt_t.size = packet.size;

          int decoded_size = 0, data_size;
          SAAudioPacket *sa_ap_ret;
          
          while(pkt_t.size > 0)
          {
               if(ret == NULL)
               {
                    ret = malloc(sizeof(SAAudioPacket));
                    sa_ap_ret = (SAAudioPacket *)ret;
                    if(sa_ap_ret != NULL)
                         sa_ap_ret->abuffer = av_malloc(sizeof(uint8_t) * SAABUFFER_SIZE);
               
                    if(ret == NULL || sa_ap_ret->abuffer == NULL)
                    {
                         if(ret != NULL)
                              free(ret);
                         printf("malloc for SAAudioPacket failed\n");
                         goto DECODE_FAILED;
                    }
               }

               data_size = sizeof(uint8_t) * SAABUFFER_SIZE;
               decoded_size = avcodec_decode_audio3(sa_ctx->a_codec_ctx,
                                                    (int16_t *)(sa_ap_ret->abuffer),
                                                    &data_size, &pkt_t);

               if(decoded_size <= 0) // FIXME: "if error, we skip the frame"
               {
                    av_free(sa_ap_ret->abuffer);
                    free(ret);
                    ret = NULL;
                    
                    // DEBUG
                    printf("skip this audio frame.\n");
                    
                    break;
               }
               
               pkt_t.data += decoded_size;
               pkt_t.size -= decoded_size;
               
               if(data_size <= 0)
               {
                    printf("got nothing?\n");
                    continue;
               }

               sa_ap_ret->len = data_size;

               SAQ_push(sa_ctx->aq_ctx, sa_ap_ret);
               ret = NULL;
          }
          av_free_packet(&packet);
     }
     
     /* unlock the mutex. */
     SDL_mutexV(sa_ctx->decode_lock);

     return 0; // FIXME: EOF detection needed

DECODE_FAILED:
     
     av_free_packet(&packet);
     SDL_mutexV(sa_ctx->decode_lock);
     
     return -1;
}

int _SA_get_buffer(struct AVCodecContext *c, AVFrame *pic)
{
     int ret = avcodec_default_get_buffer(c, pic);
     uint64_t *pts = av_malloc(sizeof(uint64_t));
     *pts = *(uint64_t *)(c->opaque);
     pic->opaque = pts;
     return ret;
}

void _SA_release_buffer(struct AVCodecContext *c, AVFrame *pic)
{
     if(pic)
          av_freep(&pic->opaque);
     avcodec_default_release_buffer(c, pic);
     return;
}
