#ifndef VIDEO_CODEC_H
#define VIDEO_CODEC_H

#include "image_handler.h"
#include "codec.h"

class Realtime_Media_Stream_Sender;
class Grabber;
class Video_Display;
class Video_Encoder_Instance;
typedef uint8_t byte_t;

typedef SRef<Video_Encoder_Instance*>(*Video_Coder_Factory_Func_Ptr)();

class Video_Codec_Description : public Codec_Description
{
public:
    SRef<Decoder_Instance*> new_decoder_instance(const SRef<Realtime_Media_Stream_Receiver*>&rtmsr);
    SRef<Encoder_Instance*> new_encoder_instance();

    Video_Codec_Description();

    virtual std::string get_codec_name();
    virtual std::string get_codec_description();
    virtual uint8_t get_sdp_media_type();
    virtual std::string get_sdp_media_attributes();

    virtual std::string get_mem_object_type() const {return "VideoCodecDescription";}

    virtual std::string get_plugin_type()const{return "VideoCodec";}

    virtual uint32_t get_version() const;

    static void set_video_coder_factory(Video_Coder_Factory_Func_Ptr f) { coder_factory = f;}
private:
    static Video_Coder_Factory_Func_Ptr coder_factory;
};


class Video_Decoder_Instance: public Decoder_Instance
{
public:
    Video_Decoder_Instance(const SRef<Realtime_Media_Stream_Receiver*>& rtmsr);
};

class Video_Encoder_Instance: public Encoder_Instance
{
public:
    Video_Encoder_Instance();

    virtual void init_encoder(const unsigned int &width, const unsigned int &height, const unsigned int &framerate,
                              const unsigned int &bitrate, volatile int *globalBitratePtr) = 0;
    virtual bool handles_chroma( uint32_t chroma ) = 0;
    virtual int set_video_size(int width, int height) = 0;
    virtual float get_video_ratio() = 0;
};

class Video_Codec_Registry: public SPlugin_Registry, public SSingleton<Video_Codec_Registry>
{
public:
    virtual std::string get_plugin_type(){ return "VideoCodec"; }

    SRef<Video_Codec_Description *> create(const std::string& codecname);

protected:
    Video_Codec_Registry();
private:
    friend class SSingleton<Video_Codec_Registry>;
};

#endif // VIDEO_CODEC_H
