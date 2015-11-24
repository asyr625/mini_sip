#ifndef GSM_CODEC_H
#define GSM_CODEC_H
#include "codec.h"
#include "resampler.h"

typedef struct gsm_state *      gsm;

class Gsm_Decoder_Instance: public Decoder_Instance
{
public:
    Gsm_Decoder_Instance(const SRef<Realtime_Media_Stream_Receiver*>& rtmsr);
    ~Gsm_Decoder_Instance();

    virtual SRef<Processing_Data*> decode(const SRef<Rtp_Packet*>& rtp);

    std::string get_mem_object_type() const {return "GsmDecoderInstance";}
private:
    gsm gsmState;
};

class Gsm_Encoder_Instance: public Encoder_Instance
{
public:
    Gsm_Encoder_Instance();
    ~Gsm_Encoder_Instance();

    virtual SRef<Processing_Data_Rtp*> encode(const SRef<Processing_Data*>& data, uint16_t* seqNo, uint32_t ssrc);

    std::string get_mem_object_type() const {return "GsmEncoderInstance";}

private:
    gsm gsmState;
    SRef<Resampler *> resampler;
};

class Gsm_Codec_Description: public Audio_Codec_Description
{
public:
    Gsm_Codec_Description( SRef<Library *> lib );

    SRef<Decoder_Instance*> new_decoder_instance(const SRef<Realtime_Media_Stream_Receiver*>&);
    SRef<Encoder_Instance*> new_encoder_instance();

    virtual int32_t get_sampling_freq();
    virtual int32_t get_sampling_size_ms();
    virtual int32_t get_input_nr_samples();

    virtual std::string get_codec_name();

    virtual std::string get_codec_description();

    virtual uint8_t get_sdp_media_type();

    virtual std::string get_sdp_media_attributes();

    virtual std::string get_mem_object_type() const {return "GsmCodecDescription";}

    virtual uint32_t get_version() const;
};

#endif // GSM_CODEC_H
