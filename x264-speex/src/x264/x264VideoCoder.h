/*
 Copyright (C) 2004-2011 the Minisip Team

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

/* Copyright (C) 2011
 *
 * Authors: Telefonica,
 *          Piotr Szymaniak
*/

#ifndef X264_VIDEO_CODER_H
#define X264_VIDEO_CODER_H

#include <stdint.h>
#include <stdarg.h>
#include "x264API.h"

#define VIDEO_CODEC_H264 1

typedef struct {
	int codec_type;
	void *object;          /* Encoder instance */
	unsigned int bitrate;  /* In kbps */
} VideoCodec;

int hdviper_setup_video_encoder(VideoCodec *, int, Video *);
int hdviper_video_encode(VideoCodec *, Video *);
void hipermed_force_keyframe(VideoCodec *);
void hdviper_destroy_video_encoder(VideoCodec *);

int hdviper_setup_video_decoder(VideoCodec *, int);
int hdviper_video_decode(VideoCodec *, Video *);
void hdviper_destroy_video_decoder(VideoCodec *);

#endif
