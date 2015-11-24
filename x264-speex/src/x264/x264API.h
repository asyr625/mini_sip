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

#ifndef X264_API_H
#define X264_API_H

#include <stdarg.h>
#include <stdint.h>
#include <x264.h>
#include "../video.h"

/****************************************************************************
 * Picture structures and functions.
 ****************************************************************************/

typedef struct _TIDX264Enc TIDX264Enc;
 
struct _TIDX264Enc
{
  x264_t *h264enc;
  x264_param_t *h264param;

  unsigned int threads;
  int slice_count;       /* Number of slices per frame: forces rectangular slices. */
  unsigned int deblock;
  unsigned int maxvbr;
  unsigned int pass;
  int byte_stream;
  unsigned int bitrate;
  unsigned int framerate;
  int me;
  unsigned int subme;
  unsigned int analyse;
  int dct8x8;
  unsigned int ref;
  unsigned int bframes;
  int b_pyramid;
  int b_adaptive;
  int weightb;
  unsigned int sps_id;
  int trellis;
  unsigned int vbv_buf_capacity;
  unsigned int keyint_max;
  int cabac;

  unsigned int width, height;
  unsigned int stride, luma_plane_size;
  int framerate_num, framerate_den;

  char keyframeRequested;

//  unsigned char *buffer;
//  unsigned long buffer_size;
//  int *i_nal;
  
};

int tidx264_enc_init_encoder (TIDX264Enc *encoder, int profile, unsigned char complexity /* the bigger, the better the quality */);
void tidx264_enc_init_frame (x264_picture_t * pic_in, unsigned char *yuv[3], int width, int height);
int tidx264_enc_encode_frame (TIDX264Enc *encoder, x264_picture_t *pic_in, x264_nal_t **nals, int *nalsCount);
void tidx264_enc_force_keyframe (TIDX264Enc *encoder);
void tidx264_enc_close_encoder (TIDX264Enc *encoder);

#endif
