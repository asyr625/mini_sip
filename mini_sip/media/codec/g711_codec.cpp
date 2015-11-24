#include "g711_codec.h"

#include"g711/codec_g711.h"

#include "rtp_packet.h"
#include "media_processor.h"
#include "my_assert.h"
#include <iostream>

using namespace std;

static std::list<std::string> pluginList;
static int initialized;

extern "C"
std::list<std::string> *mg711_LTX_listPlugins( SRef<Library *> lib )
{
    if( !initialized )
    {
        pluginList.push_back("getPlugin");
        pluginList.push_back("getPluginG711a");
        initialized = true;
    }
    return &pluginList;
}

extern "C"
SPlugin * mg711_LTX_getPlugin( SRef<Library *> lib )
{
    return new G711_Codec_Description( lib, G711U );
}

extern "C"
SPlugin * mg711_LTX_getPluginG711a( SRef<Library *> lib )
{
    return new G711_Codec_Description( lib, G711A );
}

G711_Codec_Description::G711_Codec_Description( SRef<Library *> lib, G711Version v )
    : Audio_Codec_Description( lib ), version( v )
{
}

G711_Codec_Description::~G711_Codec_Description()
{
}

SRef<Processing_Data_Rtp*> G711_Encoder_Instance::encode(const SRef<Processing_Data*>& data, uint16_t* seqNo, uint32_t ssrc)
{
    SRef<Processing_Data_Audio*> adata = dynamic_cast<Processing_Data_Audio*>(*data);

    short resampledData[1600];
    short* toEncode = adata->samples;

    if (adata->sfreq!=8000)
    {
        if (!resampler)
            resampler = Resampler_Registry::get_instance()->create( SOUND_CARD_FREQ, 8000, 20, 1 /*Nb channels */);
        resampler->resample( adata->samples, resampledData );
        toEncode = &resampledData[0];
    }

    unsigned char out_data[160];
    for (int32_t i=0; i< 160; i++)
    {
        if( version == G711A )
            out_data[i] = linear2alaw(toEncode[i]);
        else
            out_data[i] = linear2ulaw(toEncode[i]);
    }

    *seqNo = *seqNo +1;
    SRef<SRtp_Packet*> rtp = new SRtp_Packet(&out_data[0], 160, *seqNo, data->_rtp_timestamp, ssrc);
    rtp->get_header().set_payload_type(/*rtpPayloadTypeNo*/ 0);
    SRef<Processing_Data_Rtp*> rdata = new Processing_Data_Rtp(rtp);
    return rdata;
}

G711_Decoder_Instance::G711_Decoder_Instance( const SRef<Realtime_Media_Stream_Receiver*>& rtmsr, G711Version v )
    : Decoder_Instance(rtmsr), version( v )
{
}

SRef<Processing_Data*> G711_Decoder_Instance::decode(const SRef<Rtp_Packet*>& rtp)
{
    unsigned char *in_data = rtp->get_content();
    int in_buf_size = rtp->get_content_length();

    SRef<Processing_Data_Audio*> data = new Processing_Data_Audio(true);
    data->samples = (short*)malloc(160*sizeof(short));
    data->nsamples = 160;
    data->sfreq = 8000;
    data->nchannels = 1;
    data->rtp_seq_no = rtp->get_header().get_seq_no();
    data->_rtp_timestamp = rtp->get_header().get_timestamp();
    data->rtp_marker = rtp->get_header().get_marker();
    data->ssrc= rtp->get_header().get_ssrc();

    short *out_data = data->samples;

    for (int32_t i=0; i< in_buf_size; i++)
    {
        if( version == G711A )
            out_data[i] = alaw2linear(in_data[i]);
        else
            out_data[i] = ulaw2linear(in_data[i]);
    }
    return *data;
}


int32_t G711_Codec_Description::get_sampling_freq()
{
    return 8000;
}

int32_t G711_Codec_Description::get_sampling_size_ms()
{
    return 20;
}

int32_t G711_Codec_Description::get_input_nr_samples()
{
    return 160;
}

std::string G711_Codec_Description::get_codec_name()
{
    if( version == G711A )
        return "G.711a";
    else
        return "G.711";
}

std::string G711_Codec_Description::get_codec_description()
{
    if( version == G711A )
        return "G.711 8kHz, PCMa";
    else
        return "G.711 8kHz, PCMu";
}

uint8_t G711_Codec_Description::get_sdp_media_type()
{
    if( version == G711A )
        return 8;
    else
        return 0;
}

std::string G711_Codec_Description::get_sdp_media_attributes()
{
    if( version == G711A )
        return "PCMA/8000/1";
    else
        return "PCMU/8000/1";
}

uint32_t G711_Codec_Description::get_version() const
{
    return 0x00000001;
}

SRef<Decoder_Instance*> G711_Codec_Description::new_decoder_instance(const SRef<Realtime_Media_Stream_Receiver*>&rtmsr)
{
    SRef<Decoder_Instance*> ret = new G711_Decoder_Instance( rtmsr, version );
    ret->set_codec( this );
    return ret;
}

SRef<Encoder_Instance*> G711_Codec_Description::new_encoder_instance()
{
    SRef<Encoder_Instance*> ret = new G711_Encoder_Instance( version );
    ret->set_codec( this );
    return ret;
}
