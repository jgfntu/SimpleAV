/* Copyright (C) 2011 by Chenguang Wang(wecing)
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 */

#include "SimpleAV.h"
#include <SDL/SDL.h>

#include <libswscale/swscale.h>
#include <stdio.h>
#include <stdlib.h>

// FIXME: unportable?
#include <unistd.h>

// unquote this line to use SDL_Surface. Sometimes it will enhance the performance.
// #define SAPLAYER_OLD_USE_SDL_SURFACE 1

#define SDL_AUDIO_BUFFER_SIZE 512

SAContext *sa_ctx = NULL;
struct SwsContext *img_convert_ctx;


#ifndef SAPLAYER_OLD_USE_SDL_SURFACE
void show_frame(AVFrame *frame, SDL_Overlay *overlay)
{
     int w = sa_ctx->v_width, h = sa_ctx->v_height;
     SDL_LockYUVOverlay(overlay);
     
     AVPicture pict;
     pict.data[0] = overlay->pixels[0];
     pict.data[1] = overlay->pixels[2];
     pict.data[2] = overlay->pixels[1];

     pict.linesize[0] = overlay->pitches[0];
     pict.linesize[1] = overlay->pitches[2];
     pict.linesize[2] = overlay->pitches[1];

     sws_scale(img_convert_ctx, (const uint8_t *const *)(frame->data),
               frame->linesize, 0, h, pict.data, pict.linesize);
	
     SDL_UnlockYUVOverlay(overlay);
	
     SDL_Rect rect;

     rect.x = 0;
     rect.y = 0;
     rect.w = w;
     rect.h = h;
     SDL_DisplayYUVOverlay(overlay, &rect);
}
#else
void avframe_to_sdlsurface(AVFrame *frame, SDL_Surface *surface)
{
     int h = sa_ctx->v_height;
     SDL_LockSurface(surface);
     
     AVPicture pict;
     pict.data[0] = surface->pixels; 
     pict.linesize[0] = surface->pitch;
     
     sws_scale(img_convert_ctx, (const uint8_t * const *)(frame->data),
               frame->linesize, 0, h, pict.data, pict.linesize);
     
     SDL_UnlockSurface(surface);
}
#endif

void audio_callback(void *data, uint8_t *stream, int len)
{
     static SAAudioPacket *sa_ap = NULL;
     static unsigned int audio_buf_index = 0;
     unsigned int size_to_copy = 0;

     if(sa_ctx->audio_eof)
          return;

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
          
          size_to_copy = len < (sa_ap->len - audio_buf_index) ?
               len : (sa_ap->len - audio_buf_index);
          memcpy(stream, sa_ap->abuffer + audio_buf_index, size_to_copy);

          len -= size_to_copy;
          stream += size_to_copy;
          audio_buf_index += size_to_copy;

          if(audio_buf_index >= sa_ap->len)
          {
               SA_free_ap(sa_ap);
               sa_ap = NULL;
               audio_buf_index = 0;
          }
     }
}

int main(int argc, char *argv[])
{
     if(argc != 2)
     {
          printf("Usage:\ntest <filename>\n");
          return 0;
     }
     
     SDL_Init(SDL_INIT_EVERYTHING);

     SA_init();
     sa_ctx = SA_open(argv[1]);
     if(sa_ctx == NULL)
     {
          printf("failed opening %s...\n", argv[1]);
          return 0;
     }

     SDL_Surface *screen = SDL_SetVideoMode(sa_ctx->v_width, sa_ctx->v_height, 32, 0);
#ifndef SAPLAYER_OLD_USE_SDL_SURFACE
     SDL_Overlay *overlay = SDL_CreateYUVOverlay(sa_ctx->v_width, sa_ctx->v_height,
                                                 SDL_YV12_OVERLAY, screen);
#endif

     int w = sa_ctx->v_width, h = sa_ctx->v_height;
     
#ifndef SAPLAYER_OLD_USE_SDL_SURFACE
     img_convert_ctx = sws_getContext(w, h, sa_ctx->v_codec_ctx->pix_fmt,
                                      w, h, PIX_FMT_YUV420P, SWS_BICUBIC,
                                      NULL, NULL, NULL);
#else
     img_convert_ctx = sws_getContext(w, h, sa_ctx->v_codec_ctx->pix_fmt,
                                      w, h, PIX_FMT_RGB32, SWS_BICUBIC,
                                      NULL, NULL, NULL);
#endif
     
     if(img_convert_ctx == NULL)
     {
          printf("Failed to get struct Swscontext!\n");
          goto PROGRAM_QUIT;
     }

     SDL_AudioSpec wanted_spec, spec;
     wanted_spec.freq = sa_ctx->a_codec_ctx->sample_rate;
     wanted_spec.format = AUDIO_S16SYS; // FIXME: wtf is this?
     wanted_spec.channels = sa_ctx->a_codec_ctx->channels;
     wanted_spec.silence = 0;
     wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
     wanted_spec.callback = audio_callback;
     wanted_spec.userdata = NULL;

     if(SDL_OpenAudio(&wanted_spec, &spec) < 0)
     {
          printf("SDL_OpenAudio: %s\n", SDL_GetError());
          goto PROGRAM_QUIT;
     }

     SDL_PauseAudio(0);

     int64_t start_time = av_gettime();

     SDL_Event event;
     double w_clock;
     SAVideoPacket *vp = NULL;
     while(!(sa_ctx->video_eof && sa_ctx->audio_eof))
     {
          if(sa_ctx->video_eof)
               goto SKIP_VIDEO;

          while(vp == NULL || vp->pts <= (double)(av_gettime() - start_time) / (double)1000000.0f)
          {
               if(vp != NULL)
                    SA_free_vp(vp);
               
               vp = SA_get_vp(sa_ctx);
               if(vp == NULL)
               {
                    sa_ctx->video_eof = 1;
                    goto SKIP_VIDEO; // FIXME: EOF encountered?
               }
          }
          
          w_clock = vp->pts - (double)(av_gettime() - start_time) / (double)1000000.0f;
          while (w_clock >= 0.0)
          {
               usleep(w_clock * 1000 + 1);
               w_clock = vp->pts - (double)(av_gettime() - start_time) / (double)1000000.0f;
          }
          
#ifndef SAPLAYER_OLD_USE_SDL_SURFACE
          show_frame(vp->frame_ptr, overlay);
#else
          avframe_to_sdlsurface(vp->frame_ptr, screen);
          SDL_Flip(screen);
#endif

     SKIP_VIDEO:

          while(SDL_PollEvent(&event))
               if(event.type == SDL_QUIT)
               {
                    sa_ctx->video_eof = sa_ctx->audio_eof = 1;
                    SDL_PauseAudio(1);
                    break;
               } else if(event.type == SDL_KEYDOWN)
               {
                    double delta;
                    switch(event.key.keysym.sym)
                    {
                    case SDLK_LEFT:
                         delta = -10.0;
                         break;
                    case SDLK_RIGHT:
                         delta = 10.0;
                         break;
                    case SDLK_UP:
                         delta = -60.0;
                         break;
                    case SDLK_DOWN:
                         delta = 60.0;
                         break;
                    default:
                         goto IGNORE_KEY;
                    }

                    SA_seek(sa_ctx, (double)(av_gettime() - start_time) / (double)1000000.0f + delta, delta);

                    // FIXME: should call this only when EOF encountered?
                    SDL_PauseAudio(0);

                    if(vp != NULL)
                    {
                         SA_free_vp(vp);
                         vp = NULL;
                    }

                    vp = SA_get_vp(sa_ctx);
                    if(vp == NULL)
                    {
                         sa_ctx->video_eof = 1;
                         goto SKIP_VIDEO; // FIXME: eof?
                    }

#ifndef SAPLAYER_OLD_USE_SDL_SURFACE
                    show_frame(vp->frame_ptr, overlay);
#else
                    avframe_to_sdlsurface(vp->frame_ptr, screen);
                    SDL_Flip(screen);
#endif
                    
                    start_time = av_gettime() - (int64_t)((vp->pts) * 1000000);

               IGNORE_KEY:;
               }
     }

     if(vp != NULL)
          SA_free_vp(vp);

PROGRAM_QUIT:

     SA_close(sa_ctx);
     SDL_CloseAudio();
#ifndef SAPLAYER_OLD_USE_SDL_SURFACE
     SDL_FreeYUVOverlay(overlay);
#endif
     SDL_Quit();
     
     return 0;
}
