#include <stdio.h>

#include <SDL/SDL.h>
#include "SimpleAV/SDL/api.h"

int main(int argc, char *argv[])
{
     if(argc != 2)
     {
          fprintf(stderr, "Usage:\n%s <filename>\n", argv[0]);
          return 1;
     }
     
     SDL_Init(SDL_INIT_EVERYTHING);
     SASDL_init();
     SAContext *sa_ctx = SASDL_open(argv[1]);
     if(sa_ctx == NULL)
     {
          fprintf(stderr, "failed to open video file?\n");
          SDL_Quit();
          return 1;
     }
     
     SDL_Surface *screen = SDL_SetVideoMode(
          SASDL_get_width(sa_ctx), SASDL_get_height(sa_ctx),
          32, SDL_SWSURFACE);

     SASDL_play(sa_ctx);

     double delta = 0.0f;
     SDL_Event event;
     int (*get_event)(SDL_Event *) = SDL_PollEvent;
     while(SASDL_draw(sa_ctx, screen) == 0) // FIXME: add precise EOF detect.
     {
          SDL_Flip(screen);

          // while(SDL_PollEvent(&event))
          while(get_event(&event))
               if(event.type == SDL_QUIT)
               {
                    SASDL_close(sa_ctx);
                    goto PROGRAM_QUIT;
               } else if(event.type == SDL_KEYDOWN)
               {
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
                    case SDLK_SPACE:
                         // DEBUG
                         printf("pause!\n");
                         
                         if(SASDL_get_video_status(sa_ctx) == SASDL_is_playing)
                         {
                              SASDL_pause(sa_ctx);
                              get_event = SDL_WaitEvent;
                              continue;
                         } else
                         {
                              SASDL_play(sa_ctx);
                              get_event = SDL_PollEvent;
                              goto NEXT_LOOP;
                         }
                    case SDLK_s:
                         // DEBUG
                         printf("stop!\n");
                         
                         SASDL_stop(sa_ctx);
                         get_event = SDL_WaitEvent;
                         continue;
                    default:
                         /* ignore this event. get the next one. */
                         continue;
                    }

                    if(SASDL_seek(sa_ctx, SASDL_get_video_clock(sa_ctx) + delta) < 0)
                    {
                         SASDL_close(sa_ctx);
                         goto PROGRAM_QUIT;
                    }
               }
          
          if(SASDL_delay(sa_ctx) < 0)
               break;
     NEXT_LOOP:;
     }
                    
PROGRAM_QUIT:
     SDL_Quit();
     return 0;
}
