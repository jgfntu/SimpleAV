/*
 * api.h
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

#ifndef __SIMPLEAV_SDL_API_H__
#define __SIMPLEAV_SDL_API_H__

#include <libavcodec/avcodec.h>
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
     SAVideoPacket *vp_cur, *vp_next;
     
     SAAudioPacket *ap; // FIXME: should ap be locked?
     /* or, will SDL_PauseAudio(1) kill the audio callback function if it is executing? */
     SDL_mutex *ap_lock;
     
     unsigned int audio_buf_index;
} SASDLContext;

// FIXME: I need ... a better threshold.
#define SASDL_AUDIO_ADJUST_THRESHOLD   (1.00f / 24)

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

double SASDL_get_delay_time(SAContext *); // if you don't like SASDL_delay()... ;-P

void SASDL_audio_callback(void *, uint8_t *, int); // FIXME: this should be private

/*
 * others
 */
int SASDL_get_width(SAContext *);

int SASDL_get_height(SAContext *);

double SASDL_get_video_duration(SAContext *);

double SASDL_get_video_clock(SAContext *);

enum SASDLVideoStatus SASDL_get_video_status(SAContext *);

/*
 * "private" functions
 * FIXME: maybe I should not write it here?
 */ 
void _SASDL_avframe_to_surface(SAContext *, AVFrame *, SDL_Surface *);

#endif
