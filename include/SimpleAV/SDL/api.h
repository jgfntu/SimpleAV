#ifndef __SIMPLEAV_SDL_API_H__
#define __SIMPLEAV_SDL_API_H__

#include <libswscale/swscale.h>
#include "SimpleAV/core/core.h"

enum SASDLVideoStatus {
     SASDL_is_playing, SASDL_is_paused, SASDL_is_stopped
};

typedef struct {
     enum SASDLVideoStatus status;
     double video_start_at, start_time;
     double last_pts;
     struct SwsContext *swsctx;
     SAVideoPacket *vp;
} SASDLContext;

/*
 * init, open/close
 */
int SASDL_init(void);

SAContext *SASDL_open(char *);

int SASDL_close(SAContext *);

/*
 * video control: play/pause/stop, seek
 */
void SASDL_play(SAContext *);

void SASDL_pause(SAContext *);

int SASDL_stop(SAContext *);

int SASDL_seek(SAContext *, double);

/*
 * output: draw & delay & audio callback
 */
int SASDL_draw(SAContext *, SDL_Surface *);

int SASDL_delay(SAContext *);

void SASDL_audio_callback(void *, uint8_t *, int);

/*
 * others
 */
int SASDL_get_width(SAContext *);

int SASDL_get_height(SAContext *);

double SASDL_get_video_clock(SAContext *);

#endif
