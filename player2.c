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
     while(SASDL_draw(sa_ctx, screen) == 0) // FIXME: add precise EOF detect.
     {
          SDL_Flip(screen);
          if(SASDL_delay(sa_ctx) < 0)
               break;
     }
     
     SDL_Quit();
     return 0;
}
