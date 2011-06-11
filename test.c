#include "SA_api.h"

#include <libswscale/swscale.h>
#include <stdio.h>
#include <stdlib.h>

// FIXME: unportable?
#include <unistd.h>

#define SDL_AUDIO_BUFFER_SIZE 512

// DEBUG
int p_id = 0;
char p_filename[512];
#define SAVE_FRAME(vp)                                                \
     {                                                                \
          sprintf(p_filename, "dbg/%04d_PTS%f.bmp", ++p_id,vp->pts);  \
          SDL_SaveBMP(screen, p_filename);                            \
     }

SAContext *sa_ctx = NULL;
struct SwsContext *img_convert_ctx;

void show_frame(AVFrame *frame, SDL_Overlay *overlay)
{
     // "Show" frame
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

void audio_callback(void *data, uint8_t *stream, int len)
{
     // DEBUG
     // printf("in\n");
     
     static SAAudioPacket *sa_ap = NULL;
     static unsigned int audio_buf_index = 0;
     unsigned int size_to_copy = 0;

     if(sa_ctx->audio_eof)
          return;

     while(len > 0)
     {
          // DEBUG
          // printf("getting...\n");
          
          if(sa_ap == NULL)
               sa_ap = SA_get_ap(sa_ctx);

          // DEBUG
          // printf("got.\n");

          if(sa_ap == NULL) // cannot get SAAudioPacket: maybe EOF encountered.
          {
               sa_ctx->audio_eof = 1;
     // DEBUG
     // printf("audio_eof: %d\n", sa_ctx->audio_eof);

               memset(stream, 0, len);
               
               SDL_PauseAudio(1);

     // DEBUG
     // printf("out\n");
     

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
               av_free(sa_ap->abuffer);
               free(sa_ap);
               sa_ap = NULL;
               audio_buf_index = 0;
          }
     }

     // DEBUG
     // printf("out\n");
     
     // DEBUG
     /*
     while(sa_ctx->vq_ctx->nb < 3 || sa_ctx->aq_ctx->nb < 3)
     {
          printf("vq:%d aq:%d\n", sa_ctx->vq_ctx->nb, sa_ctx->aq_ctx->nb);
          _SA_decode_packet(sa_ctx);
     }
     */
}

double get_clock(void)
{
     return av_gettime() / 1000000.0f;
}

int main(int argc, char *argv[])
{
     // DEBUG
     // freopen("log.txt", "w", stdout);
     
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
     SDL_Overlay *overlay = SDL_CreateYUVOverlay(sa_ctx->v_width, sa_ctx->v_height,
                                                 SDL_YV12_OVERLAY, screen);

     int w = sa_ctx->v_width, h = sa_ctx->v_height;
     img_convert_ctx = sws_getContext(w, h, sa_ctx->v_codec_ctx->pix_fmt,
                                      w, h, PIX_FMT_YUV420P, SWS_BICUBIC,
                                      NULL, NULL, NULL);
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

     // DEBUG
     /*
     while(sa_ctx->vq_ctx->nb < 3 || sa_ctx->aq_ctx->nb < 3)
     {
          printf("vq:%d aq:%d\n", sa_ctx->vq_ctx->nb, sa_ctx->aq_ctx->nb);
          _SA_decode_packet(sa_ctx);
     }
     */

     SDL_PauseAudio(0);
     double start_time = get_clock();
     //double last_pts;

     SDL_Event event;
     double w_clock;
     SAVideoPacket *vp = NULL;
     // int  i = 0; // DEBUG
     while(!(sa_ctx->video_eof && sa_ctx->audio_eof))
     {
          // DEBUG
          // printf("%d: %d %d\n", i++, sa_ctx->video_eof, sa_ctx->audio_eof);
          
          if(sa_ctx->video_eof)
               goto SKIP_VIDEO;

          while(vp == NULL || vp->pts <= get_clock() - start_time)
          {
               if(vp != NULL)
               {
                    av_free(vp->frame_ptr);
                    free(vp);
               }
               
               vp = SA_get_vp(sa_ctx);
               if(vp == NULL)
               {
                    sa_ctx->video_eof = 1;
                    goto SKIP_VIDEO; // FIXME: EOF encountered?
               }
          }
          
          w_clock = vp->pts - (get_clock() - start_time);
          while (w_clock >= 0.0)
          {
               usleep(w_clock * 1000 + 1);
               w_clock = vp->pts - (get_clock() - start_time);
          }

          // last_pts = vp->pts;
          show_frame(vp->frame_ptr, overlay);
          // SAVE_FRAME(vp);

     SKIP_VIDEO:

          while(SDL_PollEvent(&event))
               if(event.type == SDL_QUIT)
               {
                    sa_ctx->video_eof = sa_ctx->audio_eof = 1;
                    SDL_PauseAudio(1);
                    
                    // DEBUG
                    // printf("Quiting...\n");
                    
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

                    // DEBUG
                    SA_seek(sa_ctx, get_clock() - start_time + delta, delta);
                    // SA_seek(sa_ctx, last_pts + delta, delta);
                    
                    // DEBUG
                    // printf("Seeking to: %f\n", last_pts + delta);

                    // FIXME: should call this only when EOF encountered?
                    SDL_PauseAudio(0);

                    if(vp != NULL)
                    {
                         av_free(vp->frame_ptr);
                         free(vp);
                         vp = NULL;
                    }

                    vp = SA_get_vp(sa_ctx);
                    if(vp == NULL)
                    {
                         printf("...what?\n");
                         
                         sa_ctx->video_eof = 1;
                         goto SKIP_VIDEO; // FIXME: eof?
                    }

                    // DEBUG
                    // printf("But in fact: %f\n", vp->pts);

                    //last_pts = vp->pts;
                    show_frame(vp->frame_ptr, overlay);
                    // DEBUG
                    /*
                    while(sa_ctx->vq_ctx->nb < 3 || sa_ctx->aq_ctx->nb < 3)
                    {
                         printf("vq:%d aq:%d\n", sa_ctx->vq_ctx->nb, sa_ctx->aq_ctx->nb);
                         _SA_decode_packet(sa_ctx);
                    }
                    */
                    // SAVE_FRAME(vp);
                    start_time = get_clock() - vp->pts;

                    // DEBUG
                    //printf("video clock: %f\n\n", get_clock() - start_time);
                    
               IGNORE_KEY:;
               }
     }

     if(vp != NULL)
     {
          av_free(vp->frame_ptr);
          free(vp);
     }

     // DEBUG
     // printf("what?\n");

PROGRAM_QUIT:

     SA_close(sa_ctx);
     // DEBUG
     // printf("what?\n");
     SDL_CloseAudio();
     SDL_Quit();
     
     return 0;
}
