#ifndef __SIMPLEAV_CORE_H__DEFINED__
#define __SIMPLEAV_CORE_H__DEFINED__

#include "SimpleAV/core/util/SAMutex.h"
#include "SimpleAV/core/util/SAQueue.h"

#include <SDL/SDL.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>

#define  SAABUFFER_SIZE ((AVCODEC_MAX_AUDIO_FRAME_SIZE) * 3 / 2)

typedef struct SAVideoPacket {
     AVFrame *frame_ptr;
     double pts;
} SAVideoPacket;

typedef struct SAAudioPacket {
     uint8_t *abuffer;
     int len;
     double pts;
} SAAudioPacket;

typedef struct SAContext {
     char *filename;

     SAMutex aq_lock_real, vpq_lock_real, apq_lock_real, packet_lock_real;
     
     SAMutex *aq_lock; // FIXME: remove SA's dependency of SDL.
     SAQContext *aq_ctx;
     SAMutex *vpq_lock, *apq_lock;
     SAQContext *vpq_ctx, *apq_ctx;
     SAMutex *packet_lock;
     
     AVPacket pkt_temp;
     
     AVFormatContext *avfmt_ctx_ptr;
     AVCodecContext *a_codec_ctx, *v_codec_ctx;
     AVStream *audio_st, *video_st;
     AVCodec *a_codec, *v_codec;
     int v_stream, a_stream;
     
     int v_width, v_height;
     int audio_eof, video_eof;
     double video_clock;

     void *lib_data;
} SAContext;

int SA_init(void);

SAContext *SA_open(char *);

void SA_close(SAContext *);

SAAudioPacket *SA_get_ap(SAContext *);

SAVideoPacket *SA_get_vp(SAContext *);

int SA_seek(SAContext *, double, double);

int _SA_read_packet(SAContext *);

int _SA_get_buffer(AVCodecContext *, AVFrame *);

void _SA_release_buffer(AVCodecContext *, AVFrame *);

int SA_get_width(SAContext *);

int SA_get_height(SAContext *);

double SA_get_clock(void);

#endif
