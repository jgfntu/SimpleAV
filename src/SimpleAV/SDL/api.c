/*
 * api.c
 * Copyright (C) 2011 by Chenguang Wang(wecing)
 *
 * This file is part of SimpleAV.
 *
 * SimpleAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SimpleAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SimpleAV. If not, see <http://www.gnu.org/licenses/>.
 */

#include "SimpleAV/core/core.h"
#include "SimpleAV/SDL/api.h"

#include <stdlib.h>
#include <libswscale/swscale.h>

int SASDL_init(void)
{
     SA_init();
     return 0;
}

SAContext *SASDL_open(char *filename)
{
     SAContext *sa_ctx = SA_open(filename);
     if(sa_ctx == NULL)
     {
          fprintf(stderr, "SA_open failed.\n");
          return NULL;
     }

     SASDLContext *sasdl_ctx = malloc(sizeof(SASDLContext));
     if(sasdl_ctx == NULL)
     {
          SA_close(sa_ctx);
          fprintf(stderr, "malloc for sasdl_ctx failed!\n");
          return NULL;
     } else
     {
          memset(sasdl_ctx, 0, sizeof(SASDLContext));
          sasdl_ctx->status = SASDL_is_stopped;
          sasdl_ctx->video_start_at = 0.0f;
          sasdl_ctx->start_time = 0.0f;
          sasdl_ctx->last_pts = 0.0f;
          sa_ctx->lib_data = sasdl_ctx;
     }
     
     int width = SASDL_get_width(sa_ctx);
     int height = SASDL_get_height(sa_ctx);
     struct SwsContext *swsctx = sws_getContext(width, height, sa_ctx->v_codec_ctx->pix_fmt,
                                                width, height, PIX_FMT_RGB32, SWS_BICUBIC,
                                                NULL, NULL, NULL);
     if(swsctx == NULL)
     {
          SASDL_close(sa_ctx);
          fprintf(stderr, "sws_getContext failed!\n");
          return NULL;
     } else
          sasdl_ctx->swsctx = swsctx;

     SDL_AudioSpec wanted_spec;
     wanted_spec.freq = sa_ctx->a_codec_ctx->sample_rate;
     wanted_spec.format = AUDIO_S16SYS;
     wanted_spec.channels = sa_ctx->a_codec_ctx->channels;
     wanted_spec.silence = 0;
     wanted_spec.samples = 512;
     wanted_spec.callback = SASDL_audio_callback;
     wanted_spec.userdata = sa_ctx;

     if(SDL_OpenAudio(&wanted_spec, NULL) < 0)
     {
          fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
          SASDL_close(sa_ctx);
          return NULL;
     }

     return sa_ctx;
}

int SASDL_close(SAContext *sa_ctx)
{
     if(sa_ctx == NULL)
          return -1;

     SDL_PauseAudio(1);
     
     SASDLContext *sasdl_ctx = sa_ctx->lib_data;
     if(sasdl_ctx != NULL)
          free(sasdl_ctx);
     
     SA_close(sa_ctx);
     return 0;
}

void SASDL_play(SAContext *sa_ctx)
{
     SASDLContext *sasdl_ctx = sa_ctx->lib_data;
     sasdl_ctx->status = SASDL_is_playing;
     sasdl_ctx->start_time = SA_get_clock() - sasdl_ctx->video_start_at;
     sasdl_ctx->last_pts = sasdl_ctx->video_start_at;
     SDL_PauseAudio(0);
}

void SASDL_pause(SAContext *sa_ctx)
{
     SASDLContext *sasdl_ctx = sa_ctx->lib_data;
     sasdl_ctx->video_start_at = SASDL_get_video_clock(sa_ctx);
     sasdl_ctx->status = SASDL_is_paused;
     SDL_PauseAudio(1);
     
     SAVideoPacket *vp = sasdl_ctx->vp;
     if(vp != NULL)
     {
          av_free(vp->frame_ptr);
          free(vp);
          sasdl_ctx->vp = NULL;
     }
}

int SASDL_stop(SAContext *sa_ctx)
{
     int ret;
     SASDLContext *sasdl_ctx = sa_ctx->lib_data;
     SAVideoPacket *vp = sasdl_ctx->vp;
     if(vp != NULL)
     {
          av_free(vp->frame_ptr);
          free(vp);
          sasdl_ctx->vp = NULL;
     }
     
     sasdl_ctx->status = SASDL_is_stopped;
     sasdl_ctx->video_start_at = 0.0f;
     ret = SASDL_seek(sa_ctx, 0.0f);
     SDL_PauseAudio(1);
     return ret;
}

int SASDL_seek(SAContext *sa_ctx, double seek_dst)
{
     if(seek_dst < 0)
          seek_dst = 0;
     
     int ret;
     SASDLContext *sasdl_ctx = sa_ctx->lib_data;
     SAVideoPacket *vp = sasdl_ctx->vp;
     if(vp != NULL)
     {
          av_free(vp->frame_ptr);
          free(vp);
          sasdl_ctx->vp = NULL;
     }

     ret = SA_seek(sa_ctx, seek_dst,
                   seek_dst - sasdl_ctx->last_pts);
     
     vp = sasdl_ctx->vp = SA_get_vp(sa_ctx);
     if(vp == NULL)
          return -1;

     sasdl_ctx->last_pts = vp->pts;
     sasdl_ctx->video_start_at = vp->pts;
     if(sasdl_ctx->status == SASDL_is_playing)
          sasdl_ctx->start_time = SA_get_clock() - vp->pts;
     
     // SDL_PauseAudio(0);
     return ret;
}

int SASDL_draw(SAContext *sa_ctx, SDL_Surface *surface)
{
     SASDLContext *sasdl_ctx = sa_ctx->lib_data;
     if(sasdl_ctx->status != SASDL_is_playing)
          return 0;
     
     SAVideoPacket *vp = sasdl_ctx->vp;

     if(vp == NULL)
          if((sasdl_ctx->vp = vp = SA_get_vp(sa_ctx)) == NULL)
               return -1;

     // FIXME: this would fail on long-lasting frames.
     //        only one SDL_Surface is not enough.
     if(vp->pts <= SASDL_get_video_clock(sa_ctx))
     {
          AVFrame *frame = vp->frame_ptr;
          int h = SASDL_get_height(sa_ctx);
     
          SDL_LockSurface(surface);
          AVPicture pict;
          pict.data[0] = surface->pixels; 
          pict.linesize[0] = surface->pitch;
          sws_scale(sasdl_ctx->swsctx, (const uint8_t * const *)(frame->data),
                    frame->linesize, 0, h, pict.data, pict.linesize);
          SDL_UnlockSurface(surface);

          sasdl_ctx->last_pts = vp->pts;
          sasdl_ctx->vp = NULL;
          av_free(vp->frame_ptr);
          free(vp);
     }
     
     return 0;
}

int SASDL_delay(SAContext *sa_ctx)
{
     SASDLContext *sasdl_ctx = sa_ctx->lib_data;
     if(sasdl_ctx->status != SASDL_is_playing)
          return 0;
     
     SAVideoPacket *vp = sasdl_ctx->vp;
     if(vp == NULL)
          if((vp = SA_get_vp(sa_ctx)) == NULL)
               return -1;
     sasdl_ctx->vp = vp;

     double w_time = vp->pts - SASDL_get_video_clock(sa_ctx);
     while(w_time >= 0.005) // FIXME: I *hate* magic numbers.
     {
          SDL_Delay(w_time * 1000);
          w_time = vp->pts - SASDL_get_video_clock(sa_ctx);
     }
     
     sasdl_ctx->last_pts = vp->pts;
     return 0;
}

int SASDL_get_width(SAContext *sa_ctx)
{
     return SA_get_width(sa_ctx);
}

int SASDL_get_height(SAContext *sa_ctx)
{
     return SA_get_height(sa_ctx);
}

double SASDL_get_video_clock(SAContext *sa_ctx)
{
     SASDLContext *sasdl_ctx = sa_ctx->lib_data;
     switch (sasdl_ctx->status) {
     case SASDL_is_stopped:
          return 0.0f;
     case SASDL_is_paused:
          return sasdl_ctx->video_start_at;
     case SASDL_is_playing:
          return SA_get_clock() - sasdl_ctx->start_time;
     }
     return -1.0f;
}

void SASDL_audio_callback(void *data, uint8_t *stream, int len)
{
     SAContext *sa_ctx = data;
     static SAAudioPacket *sa_ap = NULL;
     static unsigned int audio_buf_index = 0;
     unsigned int size_to_copy = 0;
     double size_per_sec = 2 * sa_ctx->a_codec_ctx->channels *
                           sa_ctx->a_codec_ctx->sample_rate;
     if(sa_ctx->audio_eof)
     {
          SDL_PauseAudio(1);
          return;
     }

     while(len > 0)
     {
          if(sa_ap == NULL)
               sa_ap = SA_get_ap(sa_ctx);

          if(sa_ap == NULL)
          {
               sa_ctx->audio_eof = 1;
               memset(stream, 0, len);
               SDL_PauseAudio(1);
               return; // FIXME: *MAYBE* eof encountered. what if... ?
          }

          // DEBUG
          // double t = sa_ap->pts - SASDL_get_video_clock(sa_ctx);
          // if(t < 0) t = -t;
          // printf("%f\n", t);
          
          double delay = sa_ap->pts - SASDL_get_video_clock(sa_ctx);
          if(-SASDL_AUDIO_ADJUST_THRESHOLD <= delay &&
             delay <= SASDL_AUDIO_ADJUST_THRESHOLD)
               delay = 0.0f;
          int delay_size = delay * size_per_sec;
          if(delay_size > 0) // 'wait' for the external clock
          {
               int silent_size = len < delay_size ? len : delay_size;
               memset(stream, 0, silent_size);
               len -= silent_size;
               stream += silent_size;
               continue;
          } else if(delay_size < 0) // shrink the buffer
          {
               audio_buf_index -= delay_size;
               
               // copy the code directly to prevent infinite looping
               if(audio_buf_index >= sa_ap->len)
               {
                    av_free(sa_ap->abuffer);
                    free(sa_ap);
                    sa_ap = NULL;
                    audio_buf_index = 0;
                    continue;
               }
          }
          
          size_to_copy = len < (sa_ap->len - audio_buf_index) ?
               len : (sa_ap->len - audio_buf_index);
          memcpy(stream, sa_ap->abuffer + audio_buf_index, size_to_copy);

          len -= size_to_copy;
          stream += size_to_copy;
          audio_buf_index += size_to_copy;

          if(audio_buf_index >= sa_ap->len)
          {
               av_free(sa_ap->abuffer);
               free(sa_ap);
               sa_ap = NULL;
               audio_buf_index = 0;
          }
     }
}

enum SASDLVideoStatus SASDL_get_video_status(SAContext *sa_ctx)
{
     SASDLContext *sasdl_ctx = sa_ctx->lib_data;
     return sasdl_ctx->status;
}
