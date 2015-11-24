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

#include "x264VideoCoder.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sched.h>

int hdviper_setup_video_encoder(VideoCodec *c, int type, Video *v) {
    int resul = 0;

    if ( type == VIDEO_CODEC_H264 )
    {
	c->object = (TIDX264Enc *)malloc(sizeof(TIDX264Enc));
    }

    c->codec_type = type;
    ((TIDX264Enc *)c->object)->width = v->width;
    ((TIDX264Enc *)c->object)->height = v->height;
    ((TIDX264Enc *)c->object)->bitrate = c->bitrate;
    ((TIDX264Enc *)c->object)->framerate = v->fps;
    ((TIDX264Enc *)c->object)->slice_count = 4;

    if ( type == VIDEO_CODEC_H264 )
    {
			resul = tidx264_enc_init_encoder ((TIDX264Enc *)c->object, v->profile, v->complexity);
    }
    return resul;
}

int hdviper_video_encode(VideoCodec *c, Video *v) {
    int resul = 0;

    if ( c->codec_type == VIDEO_CODEC_H264 )
    {
      x264_picture_t picture;
      tidx264_enc_init_frame(&picture, v->yuvs, v->width, v->height);
      /*printf("hdviper_video_encode copied frame %dms\n", v->pts_us / 1000);*/
      resul = tidx264_enc_encode_frame ((TIDX264Enc *)c->object, &picture, &v->nals, &v->nalsCount);
      x264_picture_clean(&picture);
    }
    return resul;
}

void hipermed_force_keyframe(VideoCodec *c)
{
	tidx264_enc_force_keyframe((TIDX264Enc *)c->object);
}

void hdviper_destroy_video_encoder(VideoCodec *c) {
    if ( c->codec_type == VIDEO_CODEC_H264 )
    {
	tidx264_enc_close_encoder ((TIDX264Enc *)c->object);
    }
}


int hdviper_setup_video_decoder(VideoCodec *c, int type) {
}

int hdviper_video_decode(VideoCodec *c, Video *v) {
}

void hdviper_destroy_video_decoder(VideoCodec *c) {
}
