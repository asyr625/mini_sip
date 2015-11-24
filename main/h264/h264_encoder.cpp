#include "h264_encoder.h"

#include "my_time.h"
#include "timestamp.h"
#include "video_exception.h"

#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>

#ifdef LOGGING_SUPPORT
#include "logging_manager.h"
#endif

//extern int albertohack_width;
//extern int albertohack_height;

extern "C"
{
#include<libswscale/swscale.h>
}

#include <math.h>
#include <sys/time.h>

#define H264_SINGLE_NAL_UNIT_PACKET 0
#define H264_AGGREGATION_PACKET 1
#define H264_FRAGMENTATION_UNIT 2

//RTP header length set to zero since it should not be added here
//(SrtpPacket handles it).
#define RTP_HEADER_LEN 0
#define HDVIPER_RTP_VER 2

#define HDVIPER_RTP_PT_PCMU 0
#define HDVIPER_RTP_PT_PCMA 8
#define HDVIPER_RTP_PT_SPEEX 107
#define HDVIPER_RTP_PT_H264 119
#define HDVIPER_RTP_PT_H263 120
#define HDVIPER_RTP_PT_JPEG 26

#define RTP_PAYLOAD_LEN 1400

#define BASELINE_PROFILE 0

using namespace std;

SRef<Video_Encoder_Instance*> h264_factory()
{
    return new H264_Encoder();
}

H264_Encoder::H264_Encoder()
    : initialized(false), width(0), height(0), framerate(0), bitrate(0), globalBitratePtr(0),
      grabbing_width(0), grabbing_height(0), intraFrameTime_us(0), ptsOfLastEncodedFrame_us(-1)
{
    reinit = false;
    video = NULL;
    video_codec = NULL;
    swsctx = NULL;
    N = 0;
    /*We initialize ratio 16/9 it can be overwritted when init
 * codec */
    grabbingRatio = 16 / 9;
    profile = BASELINE_PROFILE;
}

H264_Encoder::~H264_Encoder()
{
    if(swsctx)
        sws_freeContext(swsctx);
    avpicture_free((AVPicture *) &converted_frame );
}


int H264_Encoder::set_video_size(int w, int h)
{
    lock.lock();

    if( w / h != grabbingRatio)
    {
        lock.unlock();
        return -1;
    }
    if( w % 2 != 0 ) w++;
    if( h % 2 != 0 ) h++;
    if(w < 320 || h < 180)
    {
        w = 320;
        h = 180;
    }

    width = w;
    height = h;
    bitrate = w * h * framerate * bw_bitsPerBixel / 1024;
    if (bitrate < 256)
        bitrate = 256;
    reinit = true;
    lock.unlock();
    return 0;
}

float H264_Encoder::get_video_ratio()
{
    return	grabbingRatio;
}

void H264_Encoder::init_encoder(const unsigned int &_width, const unsigned int &_height, const unsigned int &_framerate,
                                const unsigned int &_bitrate, volatile int *_globalBitratePtr)
{
    lock.lock();
    avpicture_alloc((AVPicture *) &converted_frame, PIX_FMT_YUV420P, _width, _height);
    width = _width;
    height = _height;
    framerate = _framerate;
    bitrate = _bitrate;
    globalBitratePtr = _globalBitratePtr;
    double pixelspersecond = width * height * framerate;
    double bitspersecond = bitrate * 1024;
    bw_bitsPerBixel = bitspersecond / pixelspersecond;
    lock.unlock();
}


void H264_Encoder::init(const uint32_t &grabbingWidth, const uint32_t &grabbingHeight)
{

}

void H264_Encoder::close()
{
    VideoCodec *videoCodec = (VideoCodec*)this->video_codec;
    if (videoCodec)
        hdviper_destroy_video_encoder(videoCodec);
#ifdef LOGGING_SUPPORT
    Logger::get_instance()->info(string("Encoder closed"), "info.encoder");
#endif
}

void H264_Encoder::set_profile(int profile)
{
    this->profile = profile;
}

void H264_Encoder::set_width(uint32_t width)
{
    this->width = width;
}

void H264_Encoder::set_height(uint32_t height)
{
    this->height = height;
}


struct encoder_rtp_data{
    byte_t * data;
    int length;
    int64_t pts_us;
    bool marker;
};

#define REPORT_N 10

SRef<Processing_Data_Rtp*> H264_Encoder::encode(const SRef<Processing_Data*>&, uint16_t* seqNo, uint32_t ssrc)
{

}


int h264_start_code(unsigned char* p)
{
    if (p[0]==0 && p[1]==0 && p[2]==1)
    {
        return p[3]&0x1F;
    }
    return 0;
}


void H264_Encoder::make_h264_header(unsigned char *buf, int packetization_mode, unsigned char nal_unit_octet, int frag_start, int frag_end)
{

}

void H264_Encoder::hdviper_h264_packetize_nal_unit(Video *v, unsigned char *h264_data, int size, bool last_nal_unit_of_frame, std::list<struct encoder_rtp_data*> &rtpdataout)
{

}

void H264_Encoder::hdviper_h264_packetize(Video *v, std::list<struct encoder_rtp_data*> &rtpdataout)
{

}

bool H264_Encoder::handles_chroma( uint32_t chroma )
{
    return (chroma == M_CHROMA_RV24) || (chroma == M_CHROMA_I420);
}

void H264_Encoder::request_codec_intracoded()
{
    hipermed_force_keyframe((VideoCodec*)videoCodec);
}
