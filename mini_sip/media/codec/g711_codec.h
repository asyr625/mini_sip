#ifndef G711_CODEC_H
#define G711_CODEC_H

#include "codec.h"
#include "resampler.h"

enum G711Version
{
    G711U = 1,
    G711A = 2
};

class G711_Decoder_Instance : public Decoder_Instance
{
public:
    G711_Decoder_Instance(const SRef<Realtime_Media_Stream_Receiver*>& rtmsr, G711Version v );
    virtual SRef<Processing_Data*> decode(const SRef<Rtp_Packet*>& rtp);
private:
    G711Version version;
};


class G711_Encoder_Instance : public Encoder_Instance
{
public:
    virtual SRef<Processing_Data_Rtp*> encode(const SRef<Processing_Data*>& data, uint16_t* seqNo, uint32_t ssrc);
private:
    G711Version version;
    SRef<Resampler *> resampler;
};


class G711_Codec_Description : public Audio_Codec_Description
{
public:
    virtual SRef<Decoder_Instance*> new_decoder_instance(const SRef<Realtime_Media_Stream_Receiver*>&);
    virtual SRef<Encoder_Instance*> new_encoder_instance();

    G711_Codec_Description(SRef<Library *> lib, G711Version v );
    virtual ~G711_Codec_Description();

    virtual int32_t get_sampling_freq();
    virtual int32_t get_sampling_size_ms();
    virtual int32_t get_input_nr_samples();

    virtual std::string get_codec_name();

    virtual std::string get_codec_description();

    virtual uint8_t get_sdp_media_type();

    virtual std::string get_sdp_media_attributes();

    virtual std::string get_mem_object_type() const {return "G711CodecDescription";}

    virtual uint32_t get_version() const;
private:
    G711Version version;
};

#endif // G711_CODEC_H
