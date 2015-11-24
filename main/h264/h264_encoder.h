#ifndef H264_ENCODER_H
#define H264_ENCODER_H

#define HAVE_MMX

#include <string>
#include "video_codec.h"

extern "C"{
    #include <avcodec.h>
    #include "../../../../x264-speex/src/x264/x264VideoCoder.h"
}
#define AVCODEC_MAX_VIDEO_FRAME_SIZE (3*1024*1024)

typedef uint8_t byte_t;

SRef<Video_Encoder_Instance*> h264_factory();

class Video_Encoder_Callback;
class SwsContext;

struct encoder_rtp_data;

class H264_Encoder : public virtual Video_Encoder_Instance
{
public:
    H264_Encoder();
    virtual void init_encoder(const unsigned int &_width, const unsigned int &_height, const unsigned int &_framerate, const unsigned int &_bitrate, volatile int *_globalBitratePtr);
    virtual ~H264_Encoder();

    virtual SRef<Processing_Data_Rtp*> encode(const SRef<Processing_Data*>&, uint16_t* seqNo, uint32_t ssrc);

    virtual bool handles_chroma( uint32_t chroma );

    virtual std::string get_mem_object_type() const {return "AVEncoder";}

    uint32_t mbCounter;

    void close();
    void set_profile(int profile);
    void set_width(uint32_t width);
    void set_height(uint32_t height);

    uint32_t get_width() { return width; }
    uint32_t get_height() { return height; }
    uint32_t get_framerate() { return framerate; }
    uint32_t get_bitrate() { return bitrate; }

    virtual int set_video_size(int w, int h);
    virtual float get_video_ratio();

    virtual void request_codec_intracoded();

private:
    virtual void init(const uint32_t &grabbingWidth, const uint32_t &grabbingHeight);
    Mutex lock;
    bool initialized;
    bool reinit;
    /*VideoCodec*/ void *video_codec;
    /*Video*/ void *video;

    void make_h264_header(unsigned char *buf, int packetization_mode, unsigned char nal_unit_octet, int frag_start, int frag_end);
    void hdviper_h264_packetize_nal_unit(Video *v, unsigned char *h264_data, int size, bool last_nal_unit_of_frame, std::list<struct encoder_rtp_data*> &rtpdataout);
    void hdviper_h264_packetize(Video *v, std::list<struct encoder_rtp_data*> &rtpdataout);

    Video_Encoder_Callback * callback;

    SRef<Video_Display*> local_display;

    SwsContext *swsctx;
    AVFrame converted_frame;
    int N;
    int profile;
    uint32_t width, height, framerate, bitrate, grabbing_width, grabbing_height;
    float grabbingRatio;
    double bw_bitsPerBixel;
    int64_t intraFrameTime_us, ptsOfLastEncodedFrame_us;
    volatile int *globalBitratePtr;
};

#endif // H264_ENCODER_H
