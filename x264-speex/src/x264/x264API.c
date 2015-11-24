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

#include "x264API.h"
#include <stdint.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

/*
#ifndef _MSC_VER
#include "config.h"
#include "common/common.h"
#endif
*/

#define ARG_THREADS_DEFAULT 4
#define ARG_SLICE_COUNT_DEFAULT 1
#define ARG_DEBLOCKING_DEFAULT 0
#define ARG_MAXVBR_DEFAULT 1000
#define ARG_PASS_DEFAULT 0
#define ARG_BYTE_STREAM_DEFAULT 1
#define ARG_BITRATE_DEFAULT (2 * 1024)
#define ARG_VBV_BUF_CAPACITY_DEFAULT 1.5 // of an average frame size
#define ARG_ME_DEFAULT X264_ME_HEX
#define ARG_SUBME_DEFAULT 1
#define ARG_ANALYSE_DEFAULT 0
#define ARG_DCT8x8_DEFAULT 0
#define ARG_REF_DEFAULT 1
#define ARG_BFRAMES_DEFAULT 0
#define ARG_B_PYRAMID_DEFAULT X264_B_PYRAMID_STRICT
#define ARG_B_ADAPTIVE_DEFAULT 0
#define ARG_WEIGHTB_DEFAULT 0
#define ARG_SPS_ID_DEFAULT 0
#define ARG_TRELLIS_DEFAULT 1
#define ARG_KEYINT_MAX_DEFAULT 0
#define ARG_CABAC_DEFAULT 1

/* initialize the new element
 * instantiate pads and add them to element
 * set functions
 * initialize structure
 */
void
tidx264_enc_init(TIDX264Enc * encoder, unsigned char complexity)
{
  /* initialize internals */
  encoder->h264enc = NULL;

  encoder->framerate_num = encoder->framerate;
  encoder->framerate_den = 1;
  encoder->b_pyramid = ARG_B_PYRAMID_DEFAULT;
  encoder->keyint_max = ARG_KEYINT_MAX_DEFAULT;
  encoder->vbv_buf_capacity = ARG_VBV_BUF_CAPACITY_DEFAULT;
  encoder->keyframeRequested = 0;
/*encoder->threads = ARG_THREADS_DEFAULT;*/

/*
  encoder->slice_count = ARG_SLICE_COUNT_DEFAULT;
  encoder->maxvbr = ARG_MAXVBR_DEFAULT;
  encoder->pass = ARG_PASS_DEFAULT;
  encoder->byte_stream = ARG_BYTE_STREAM_DEFAULT;
//  encoder->bitrate = ARG_BITRATE_DEFAULT;
  encoder->me = ARG_ME_DEFAULT;
  encoder->subme = ARG_SUBME_DEFAULT;
  encoder->analyse = ARG_ANALYSE_DEFAULT;
  encoder->dct8x8 = ARG_DCT8x8_DEFAULT;
  encoder->ref = ARG_REF_DEFAULT;
  encoder->bframes = ARG_BFRAMES_DEFAULT;
  encoder->b_adaptive = ARG_B_ADAPTIVE_DEFAULT;
  encoder->weightb = ARG_WEIGHTB_DEFAULT;
  encoder->deblock = ARG_DEBLOCKING_DEFAULT;
  encoder->sps_id = ARG_SPS_ID_DEFAULT;
  encoder->trellis = ARG_TRELLIS_DEFAULT;
  encoder->cabac = ARG_CABAC_DEFAULT;

  encoder->buffer_size = 1040000;
  encoder->buffer = (unsigned char *)malloc ((sizeof(unsigned char))*encoder->buffer_size);
*/

  encoder->h264param = malloc(sizeof(x264_param_t));
  unsigned char x264_preset_names_size = 0xff;
  while(x264_preset_names[++x264_preset_names_size]);
  const char *preset = x264_preset_names[complexity < x264_preset_names_size ? complexity : x264_preset_names_size-1];
  if(x264_param_default_preset(encoder->h264param, preset, "zerolatency") < 0)
    printf("tidx264_enc_init function in tidx264.c failed when calling x264_param_default_preset\n");
  if(x264_param_apply_profile(encoder->h264param, "baseline") < 0)
    printf("tidx264_enc_init function in tidx264.c failed when calling x264_param_apply_profile\n");
}

/*
 * tidx264_enc_init_encoder
 * @encoder:  Encoder which should be initialized.
 *
 * Initialize tidh264 encoder.
 *
 */
int
tidx264_enc_init_encoder (TIDX264Enc * encoder, int profile, unsigned char complexity)
{
	tidx264_enc_init (encoder, complexity);

	encoder->h264param->i_fps_num = encoder->framerate_num;
	encoder->h264param->i_fps_den = encoder->framerate_den;
	encoder->h264param->i_width = encoder->width;
	encoder->h264param->i_height = encoder->height;
	encoder->h264param->vui.i_sar_width = 1;
	encoder->h264param->vui.i_sar_height = 1;
	encoder->h264param->vui.i_overscan = 1;
	/* macroblock refresh interval is defaulted to 30 secs. since there is now a possibility to request a keyframe when an error occured */
	encoder->h264param->i_keyint_max = encoder->keyint_max ? encoder->keyint_max : 30 * encoder->framerate_num / encoder->framerate_den;
	encoder->h264param->i_bframe_pyramid = encoder->b_pyramid;
/*encoder->h264param->rc.i_bitrate = encoder->bitrate;*/
	encoder->h264param->rc.i_vbv_max_bitrate = encoder->bitrate;
	encoder->h264param->rc.i_vbv_buffer_size = encoder->h264param->rc.i_vbv_max_bitrate * encoder->vbv_buf_capacity * (encoder->framerate_num / encoder->framerate_den);
	encoder->h264param->i_slice_max_size = 1400;
	encoder->h264param->b_intra_refresh = 1;
/*encoder->h264param->i_threads = encoder->threads;*/
/*printf("tidx264: fps_num %u, fps_den %u, i_width %d, i_height %d\n", encoder->h264param->i_fps_num, encoder->h264param->i_fps_den, encoder->h264param->i_width, encoder->h264param->i_height);*/

	encoder->h264enc = x264_encoder_open(encoder->h264param);
	if (!encoder->h264enc) {
		printf ("Cannot initialize x264 encoder!\n");
		return 0;
	}

	return 1;

	/* The code below is skipped entirely */


  encoder->cabac = profile;
  encoder->h264param->i_slice_count = encoder->slice_count;
  encoder->h264param->i_threads = encoder->threads;

  encoder->h264param->b_cabac = encoder->cabac;
//  encoder->h264param->b_aud = 1;
  encoder->h264param->i_sps_id = encoder->sps_id;
  if ((((encoder->height == 576) && ((encoder->width == 720)
                  || (encoder->width == 704) || (encoder->width == 352)))
          || ((encoder->height == 288) && (encoder->width == 352)))
      && (encoder->framerate_den == 1) && (encoder->framerate_num == 25)) {
    encoder->h264param->vui.i_vidformat = 1;     /* PAL */
  } else if ((((encoder->height == 480) && ((encoder->width == 720)
                  || (encoder->width == 704) || (encoder->width == 352)))
          || ((encoder->height == 240) && (encoder->width == 352)))
      && (encoder->framerate_den == 1001) && ((encoder->framerate_num == 30000)
          || (encoder->framerate_num == 24000))) {
    encoder->h264param->vui.i_vidformat = 2;     /* NTSC */
  } else
    encoder->h264param->vui.i_vidformat = 5;     /* unspecified */
//  encoder->h264param->analyse.i_trellis = encoder->trellis ? 1 : 0;
  encoder->h264param->analyse.b_psnr = 0;
  encoder->h264param->analyse.b_ssim = 0;
  encoder->h264param->analyse.i_me_method = encoder->me;
  encoder->h264param->analyse.i_subpel_refine = encoder->subme;
  encoder->h264param->analyse.inter = encoder->analyse;
  encoder->h264param->analyse.b_transform_8x8 = encoder->dct8x8;
  encoder->h264param->analyse.b_weighted_bipred = encoder->weightb;
  /*encoder->h264param->analyse.i_noise_reduction = 600; */
  encoder->h264param->i_frame_reference = encoder->ref;
  encoder->h264param->i_bframe = encoder->bframes;
  encoder->h264param->i_bframe_adaptive = encoder->b_adaptive;
  encoder->h264param->b_deblocking_filter = encoder->deblock;
  encoder->h264param->i_deblocking_filter_alphac0 = 0;
  encoder->h264param->i_deblocking_filter_beta = 0;
#ifdef X264_RC_ABR
  encoder->h264param->rc.i_rc_method = X264_RC_ABR;
#endif

  switch (encoder->pass) {
    case 0:
      encoder->h264param->rc.b_stat_read = 0;
      encoder->h264param->rc.b_stat_write = 0;
      break;
    case 1:
      /* Turbo mode parameters. */
      encoder->h264param->i_frame_reference = (encoder->ref + 1) >> 1;
      encoder->h264param->analyse.i_subpel_refine =
          x264_clip3 (encoder->subme - 1, 1, 3);
      encoder->h264param->analyse.inter &= ~X264_ANALYSE_PSUB8x8;
      encoder->h264param->analyse.inter &= ~X264_ANALYSE_BSUB16x16;
      encoder->h264param->analyse.i_trellis = 0;

      encoder->h264param->rc.b_stat_read = 0;
      encoder->h264param->rc.b_stat_write = 1;
      break;
    case 2:
      encoder->h264param->rc.b_stat_read = 1;
      encoder->h264param->rc.b_stat_write = 1;
      break;
    case 3:
      encoder->h264param->rc.b_stat_read = 1;
      encoder->h264param->rc.b_stat_write = 0;
      break;
  }
  encoder->h264param->rc.psz_stat_in = NULL;
  encoder->h264param->rc.psz_stat_out = NULL;
}

/* tidx264_enc_close_encoder
 * @encoder:  Encoder which should close.
 *
 * Close tidh264 encoder.
 */
void
tidx264_enc_close_encoder (TIDX264Enc * encoder)
{
/*
  free (encoder->buffer);
  encoder->buffer = NULL;
*/
  if (encoder->h264enc != NULL) {
    x264_encoder_close(encoder->h264enc);
    encoder->h264enc = NULL;
  }
}

void
tidx264_enc_init_frame (x264_picture_t * pic_in, unsigned char *yuv[3], int width, int height)
{
  x264_picture_alloc(pic_in, X264_CSP_I420, width, height);
  memcpy(pic_in->img.plane[0], yuv[0], width * height);
  memcpy(pic_in->img.plane[1], yuv[1], width * height / 4);
  memcpy(pic_in->img.plane[2], yuv[2], width * height / 4);

    /* create tidx264_picture_t from the buffer */
  /* x264_picture_init(pic_in); */

/*
  pic_in->img.i_csp = X264_CSP_I420;
  pic_in->img.i_plane = 3;
*/
  /* FIXME: again, this looks wrong for odd widths/heights (tpm) */
/*
  pic_in->img.plane[0] = (uint8_t *) buf;
  pic_in->img.i_stride[0] = width;

  pic_in->img.plane[1] = pic_in->img.plane[0] + (width * height);
  pic_in->img.i_stride[1] = width / 2;

  pic_in->img.plane[2] = pic_in->img.plane[1] + ((width * height) / 4);
  pic_in->img.i_stride[2] = width / 2;

  pic_in->img.plane[3] = NULL;
  pic_in->img.i_stride[3] = 0;

  pic_in->i_type = X264_TYPE_AUTO;
*/
}

/* chain function
 * this function does the actual processing
 */
int
tidx264_enc_encode_frame (TIDX264Enc * encoder, x264_picture_t *pic_in, x264_nal_t **nals, int *nalsCount)
{
  /* printf("tidx264_enc_encode_frame entering\n"); */
  x264_picture_t pic_out;
  int encoder_return;
/*
  int i_size = 0;
  int i;
  unsigned char * buf_aux;
*/

	if(encoder->keyframeRequested) {
		pic_in->i_type = X264_TYPE_KEYFRAME;
		encoder->keyframeRequested = 0;
	}

  encoder_return = x264_encoder_encode (encoder->h264enc, nals, nalsCount, pic_in, &pic_out);

  if (encoder_return < 0) {
    printf ("Encode h264 frame failed. tidx264encoder_encode return code=%d\n", encoder_return);
    return 0;
  }
	return 1;
/*
  if (i_nal == 0) {
    *buffersize=0;
    return 1;
  }
*/

/*
	int i;
	outputBuffer->buffersCount = i_nal;
  for (i = 0; i < i_nal; i++) {
		if(i == outputBuffer->capacity) { // enlarge the buffer
			int newCapacity = outputBuffer->capacity * 2 + 1;
			Buffer *newBuffers = (Buffer *)calloc(newCapacity, sizeof(Buffer));
			int j;
			for(j=0; j<outputBuffer->capacity; ++j)
				newBuffers[j] = outputBuffer->buffers[j];
			free(outputBuffer->buffers);
			outputBuffer->buffers = newBuffers;
			outputBuffer->capacity = newCapacity;
		}
		if(outputBuffer->buffers[i].buffer)
			free(outputBuffer->buffers[i].buffer);
		outputBuffer->buffers[i].buffer = (unsigned int)malloc(nal[i].i_payload);
		memcpy(outputBuffer->buffers[i].buffer, nal[i].p_payload, nal[i].i_payload);
		outputBuffer->buffers[i].bufferLength = nal[i].i_payload;
		overallSize += nal[i].i_payload;
	}
	return overallSize;
*/

/*
  i_size = 0;
  for (i = 0; i < i_nal; i++) {
    int i_data = encoder->buffer_size - i_size;

    if (i_data < encoder->buffer_size / 2) {
       printf("Enlarging encoder->buffer, since there is only %i of %u bytes free in it... ", i_data, (unsigned int)encoder->buffer_size); 
      encoder->buffer_size *= 2;
      i_data = encoder->buffer_size - i_size;
      buf_aux = (unsigned char *)malloc(sizeof(unsigned char)*encoder->buffer_size);
      memcpy(buf_aux, encoder->buffer, i_data);
      free(encoder->buffer);
      encoder->buffer = buf_aux;
*/
      /* printf("done. Now encoder->buffer is %u bytes\n", (unsigned int)encoder->buffer_size); */
/*
    }
*/

    /* printf("NAL startcode is %s\n", nal[i].b_long_startcode ? "long" : "short"); */
/*
    memcpy(encoder->buffer + i_size, nal[i].p_payload, nal[i].i_payload);
    i_size += nal[i].i_payload;
*/
    /* printf("This single loop pass got %d bytes\n", nal[i].i_payload); */
/*
  }
*/

  /* printf("Overall got %d bytes\n", i_size); */
/*
  memcpy (buffer, encoder->buffer, i_size);
  *buffersize = i_size; 
*/
  /* printf("tidx264_enc_encode_frame returning\n"); */
/*
  return i_size;
*/
}

void tidx264_enc_force_keyframe (TIDX264Enc *encoder)
{
  encoder->keyframeRequested = 1;
}
