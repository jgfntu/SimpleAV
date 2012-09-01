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

// FIXME: hide this. it should be a blackbox to the client.
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

     // used for guessing PTS only; explained better in the source code of SA_seek.
     // (in short they are almost useless)
     double video_clock, a_clock;
} SAContext;

// Initialize libav. return 0 on success.
// FIXME: make it void?
int SA_init(void);

SAContext *SA_open(char *filename);

void SA_close(SAContext *sa_ctx);

void SA_dump_info(SAContext *sa_ctx);

SAAudioPacket *SA_get_ap(SAContext *sa_ctx);

SAVideoPacket *SA_get_vp(SAContext *sa_ctx);

void SA_free_ap(SAAudioPacket *ap);

void SA_free_vp(SAVideoPacket *vp);

// seek video to the last key frame before the requested second.
// return 0 on success; else return -1.
// 
// will seek to the beginning if seek_to is negative.
// FIXME: it will not do anything special if requested time is
//        after end of the video. (you could say it's a feature too)
// 
// obviously the seeking is not accurate; you could use SA_get_[av]p to
// implement accurate seeking -- keep asking for new ap/vp until the frame
// you want is out.
int SA_seek(SAContext *sa_ctx, double seek_to);

int SA_get_width(SAContext *sa_ctx);

int SA_get_height(SAContext *sa_ctx);

double SA_get_duration(SAContext *sa_ctx);

#endif

#ifdef __cplusplus
}
#endif
