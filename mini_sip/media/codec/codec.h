#ifndef CODEC_H
#define CODEC_H

#include "splugin.h"
#include "ssingleton.h"
#include "my_types.h"

class Rtp_Packet;
class Processing_Data_Rtp;
class Processing_Data;
class Decoder_Instance;
class Encoder_Instance;
class Realtime_Media_Stream_Receiver;

class Codec_Description : public SPlugin
{
public:
    virtual SRef<Decoder_Instance *> new_decoder_instance(const SRef<Realtime_Media_Stream_Receiver*>&) = 0;
    virtual SRef<Encoder_Instance *> new_encoder_instance() = 0;

    virtual std::string get_codec_name() = 0;

    virtual std::string get_codec_description() = 0;

    virtual uint8_t get_sdp_media_type() = 0;

    virtual std::string get_sdp_media_attributes() = 0;

    virtual std::string get_mem_object_type() const {return "Codec_Description";}

    virtual std::string get_name()const
    {
        return (const_cast<Codec_Description*>(this))->get_codec_name();
    }

    virtual std::string get_description()const
    {
        return (const_cast<Codec_Description*>(this))->get_codec_description();
    }

protected:
    Codec_Description( SRef<Library *> lib ): SPlugin( lib ) {}
    Codec_Description(): SPlugin() {}
};

class Decoder_Instance : public SObject
{
public:
    Decoder_Instance(const SRef<Realtime_Media_Stream_Receiver*>& rtmsr);
    virtual SRef<Processing_Data*> decode(const SRef<Rtp_Packet*>& rtp) = 0;

    virtual std::string get_mem_object_type() const {return "DecoderInstance";}

    uint8_t get_sdp_media_type(){ return codec->get_sdp_media_type(); }

    void set_codec( SRef<Codec_Description *> c ){ codec = c; }
    SRef<Codec_Description*> get_codec(){return codec;}

protected:
    SRef<Codec_Description *> codec;
    const SRef<Realtime_Media_Stream_Receiver*>& realtimeStream;
};


class Encoder_Instance : public SObject
{
public:
    Encoder_Instance();
    virtual SRef<Processing_Data_Rtp*> encode(const SRef<Processing_Data*>& data, uint16_t * seqNo, uint32_t ssrc) = 0;

    virtual std::string get_mem_object_type() const {return "EncoderInstance";}

    uint8_t get_sdp_media_type(){ return _codec->get_sdp_media_type(); }

    void set_codec( SRef<Codec_Description *> c ) { _codec = c; }
    SRef<Codec_Description*> get_codec() { return _codec;}

    void set_rtp_payload_type_no(uint8_t pt);
    uint8_t get_rtp_payload_type_no();
    virtual void request_codec_intracoded(){}

protected:
    uint8_t _rtp_payload_type_no;
    SRef<Codec_Description *> _codec;
};


class Audio_Codec_Description : public Codec_Description
{
public:
    virtual int32_t get_input_nr_samples() = 0;
    virtual int32_t get_sampling_freq() = 0;
    virtual int32_t get_sampling_size_ms() = 0;

    virtual std::string get_plugin_type()const { return "AudioCodec"; }

protected:
    Audio_Codec_Description( SRef<Library *> lib ): Codec_Description( lib ) {}
    Audio_Codec_Description(): Codec_Description() {}
};

class Audio_Codec_Registry : public SPlugin_Registry, public SSingleton<Audio_Codec_Registry>
{
public:
    virtual std::string get_plugin_type(){ return "AudioCodec"; }

    SRef<Decoder_Instance*> create_decoder_instance(const SRef<Realtime_Media_Stream_Receiver*>&, uint8_t payloadType );

    SRef<Encoder_Instance*> create_encoder_instance( uint8_t payloadType );
    SRef<Audio_Codec_Description *> create( const std::string& );

protected:
    Audio_Codec_Registry();
private:
    friend class SSingleton<Audio_Codec_Registry>;
};

#endif // CODEC_H
