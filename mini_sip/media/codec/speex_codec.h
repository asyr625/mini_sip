#ifndef SPEEX_CODEC_H
#define SPEEX_CODEC_H

#include "mutex.h"
#include "codec.h"
#include "resampler.h"

#include <speex/speex.h>

class Speex_Decoder_Instance: public Decoder_Instance
{
public:
    Speex_Decoder_Instance(const SRef<Realtime_Media_Stream_Receiver*>& rtmsr);
    virtual ~Speex_Decoder_Instance();

    virtual SRef<Processing_Data*> decode(const SRef<Rtp_Packet*>& rtp);

private:
    void init_dec();
    void free_dec();

    Mutex decLock;

    bool dec_initialized;
    void         *dec_state;
    char dec_buffer[2048];
    SpeexBits    dec_bits;
};

class Speex_Encoder_Instance: public Encoder_Instance
{
public:
    Speex_Encoder_Instance();
    virtual ~Speex_Encoder_Instance();

    virtual SRef<Processing_Data_Rtp*> encode(const SRef<Processing_Data*>& data, uint16_t* seqNo, uint32_t ssrc);
private:
    void init_enc();
    void free_enc();

    SRef<Resampler *> resampler;

    Mutex encLock;

    bool enc_initialized;
    void         *enc_state;
    char enc_buffer[2048];
    SpeexBits    enc_bits;
};

class Speex_Codec_Description : public Audio_Codec_Description
{
public:
    Speex_Codec_Description( SRef<Library *> lib );

    virtual SRef<Decoder_Instance*> new_decoder_instance(const SRef<Realtime_Media_Stream_Receiver*>&);
    virtual SRef<Encoder_Instance*> new_encoder_instance();

    virtual int32_t get_sampling_freq();
    virtual int32_t get_sampling_size_ms();
    virtual int32_t get_input_nr_samples();

    virtual std::string get_codec_name();

    virtual std::string get_codec_description();

    virtual uint8_t get_sdp_media_type();

    virtual std::string get_sdp_media_attributes();

    virtual std::string get_mem_object_type() const {return "SpeexCodecDescription";}

    virtual uint32_t get_version() const;
};

#endif // SPEEX_CODEC_H
