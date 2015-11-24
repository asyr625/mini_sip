#ifndef AVDECODER_H
#define AVDECODER_H

#include <string>

#include "sobject.h"
#include "video_stream_player.h"
#include "threaded_frame_converter.h"
#include "media_stream.h"
#include "codec.h"

//1920x1080x3 - uncompressed full-hd RGBA 24
#define MAX_ENCODED_FRAME_SIZE 6220800

class SImage;
class IRequest_Video_Keyframe;

struct AVCodec;
struct AVCodecContext;
struct AVFrame;

class AVDecoder : public Decoder_Instance
{
public:
    AVDecoder(const SRef<Realtime_Media_Stream_Receiver*>& rtmsr);
    ~AVDecoder();

    SRef<Processing_Data*> decode(const SRef<Rtp_Packet*>& rtp);

    virtual std::string get_mem_object_type() const { return "AVDecoder";}

    void close();

    void set_ssrc( uint32_t ssrc );
    std::string get_description();
    float get_current_framerate();
    void set_keyframe_request_callback(IRequest_Video_Keyframe *_keyframeRequestCallback);

private:
    SRef<Processing_Data*> add_packet_to_frame(const SRef<Rtp_Packet*>& rtp, bool flush);
    SRef<Processing_Data*> decode_frame( uint8_t * data, uint32_t length, unsigned int timestamp );

    void try_requesting_keyframe();

    byte_t frame[MAX_ENCODED_FRAME_SIZE];
    uint32_t index;

    SRef<Realtime_Media_Stream_Receiver*> realtime_stream;
    AVCodec * codec;
    AVCodecContext * context;
    AVFrame *decoded_frames[3];
    int decoded_frame_i;

    uint32_t ssrc;

    bool needs_convert;

    void* swsctx;
    int N;

    unsigned int frame_count, last_reported_frame_count;
    int64_t timestamp_of_last_framerate_get_ms;
    IRequest_Video_Keyframe *keyframe_request_callback;
};

#endif // AVDECODER_H
