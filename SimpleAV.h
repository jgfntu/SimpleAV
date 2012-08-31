/* Copyright (C) 2011 by Chenguang Wang(wecing)
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __SIMPLEAV_H__DEFINED__
#define __SIMPLEAV_H__DEFINED__

// ***** SAMutex and SAQueue starts *****

#if defined(__unix__) || defined(__MACH__)
     #include <pthread.h>
     #define SAMutex pthread_mutex_t
#elif defined(_WIN32)
     #include <windows.h>
     #include <process.h>
     #define SAMutex HANDLE
#endif

int SAMutex_init(SAMutex *);
int SAMutex_lock(SAMutex *);
int SAMutex_unlock(SAMutex *);
int SAMutex_destroy(SAMutex *);

typedef struct _SAQNode {
     void *data;
     struct _SAQNode *next;
} _SAQNode;

typedef struct SAQContext {
     _SAQNode *head, *tail;
     int nb;
} SAQContext;

SAQContext *SAQ_init(void);

int SAQ_push(SAQContext *, void *);

void *SAQ_pop(SAQContext *);

// ***** SAMutex and SAQueue ends *****

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
     
     SAMutex *aq_lock;
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

     AVFrame *frame;
     
     int v_width, v_height;
     int audio_eof, video_eof;
     double video_clock, a_clock;
} SAContext;

int SA_init(void);

SAContext *SA_open(char *);

void SA_close(SAContext *);

void SA_dump_info(SAContext *);

SAAudioPacket *SA_get_ap(SAContext *);

SAVideoPacket *SA_get_vp(SAContext *);

void SA_free_ap(SAAudioPacket *);

void SA_free_vp(SAVideoPacket *);

int SA_seek(SAContext *, double, double);

int _SA_read_packet(SAContext *);

int _SA_get_buffer(AVCodecContext *, AVFrame *);

void _SA_release_buffer(AVCodecContext *, AVFrame *);

int SA_get_width(SAContext *);

int SA_get_height(SAContext *);

double SA_get_duration(SAContext *);

#endif

#ifdef __cplusplus
}
#endif
