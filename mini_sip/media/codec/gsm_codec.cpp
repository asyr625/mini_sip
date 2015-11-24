#include "gsm_codec.h"

#include "media_processor.h"
#include "rtp_packet.h"
#include<gsm.h>

#define GSM_EXEPECTED_INPUT 160
#define GSM_FRAME_SIZE 33

static std::list<std::string> pluginList;
static int initialized;

extern "C"
std::list<std::string> *mgsm_LTX_listPlugins( SRef<Library *> lib )
{
    if( !initialized )
    {
        pluginList.push_back("getPlugin");
        initialized = true;
    }
    return &pluginList;
}

extern "C"
SPlugin * mgsm_LTX_getPlugin( SRef<Library *> lib )
{
    return new Gsm_Codec_Description( lib );
}

Gsm_Decoder_Instance::Gsm_Decoder_Instance(const SRef<Realtime_Media_Stream_Receiver*>& /*rtmsr*/)
{
    gsmState = gsm_create();
}

Gsm_Decoder_Instance::~Gsm_Decoder_Instance()
{
    gsm_destroy( gsmState );
}

SRef<Processing_Data*> Gsm_Decoder_Instance::decode(const SRef<Rtp_Packet*>& rtp)
{
    short*inBuf = rtp->get_content();

    SRef<Processing_Data_Audio*> data = new Processing_Data_Audio(true);
    data->samples = (short*)malloc(GSM_EXEPECTED_INPUT*sizeof(short));
    data->nsamples = GSM_EXEPECTED_INPUT;
    data->sfreq = codec->get_sampling_freq();
    data->nchannels = 1;
    data->rtp_seq_no = rtp->get_header().get_seq_no();
    data->_rtp_timestamp = rtp->get_header().get_timestamp();
    data->rtp_marker = rtp->get_header().get_marker();
    data->ssrc= rtp->get_header().get_ssrc();
    short *outBuf = data->samples;

    if( inSize != GSM_FRAME_SIZE )
    {
        return 0;
    }

    if( gsm_decode( gsmState, (gsm_byte *)inBuf, (gsm_signal *)outBuf ) < 0 )
    {
        return 0;
    }

    return data;
}


Gsm_Encoder_Instance::Gsm_Encoder_Instance()
{
    gsmState = gsm_create();
}

Gsm_Encoder_Instance::~Gsm_Encoder_Instance()
{
    gsm_destroy( gsmState );
}

SRef<Processing_Data_Rtp*> Gsm_Encoder_Instance::encode(const SRef<Processing_Data*>& data, uint16_t* seqNo, uint32_t ssrc)
{
    SRef<Processing_Data_Audio*> adata = dynamic_cast<Processing_Data_Audio*>(*data);
    my_assert(adata);

    short resampledData[1600];
    short* toEncode = adata->samples;

    if (adata->sfreq!=8000)
    {
        if (!resampler)
            resampler = Resampler_Registry::get_instance()->create( SOUND_CARD_FREQ, 8000, 20, 1 /*Nb channels */);
        resampler->resample( adata->samples, resampledData );
        toEncode = &resampledData[0];
    }
    int inSize = adata->nsamples*2;

    unsigned char outBuf[GSM_FRAME_SIZE];

    if( inSize != GSM_EXEPECTED_INPUT * sizeof( short ) )
    {
        return 0;
    }

    gsm_encode( gsmState, (gsm_signal *)toEncode, (gsm_byte *)outBuf );

    *seqNo = *seqNo + 1;

    SRef<SRtp_Packet*> rtp = new SRtp_Packet(&outBuf[0], GSM_FRAME_SIZE, *seqNo, timestamp, ssrc);
    rtp->get_header().set_payload_type(/*rtpPayloadTypeNo*/ 3);
    SRef<Processing_Data_Rtp*> rdata = new Processing_Data_Rtp(rtp);
    return rdata;
}


Gsm_Codec_Description::Gsm_Codec_Description( SRef<Library *> lib )
    : Audio_Codec_Description( lib )
{
}

SRef<Decoder_Instance*> Gsm_Codec_Description::new_decoder_instance(const SRef<Realtime_Media_Stream_Receiver*>&rtmsr)
{
    SRef<Decoder_Instance *> ret =  new Gsm_Decoder_Instance(rtmsr);
    ret->set_codec( this );
    return ret;
}

SRef<Encoder_Instance*> Gsm_Codec_Description::new_encoder_instance()
{
    SRef<Encoder_Instance*> ret =  new Gsm_Encoder_Instance();
    ret->set_codec( this );
    return ret;
}

int32_t Gsm_Codec_Description::get_sampling_freq()
{
    return 8000;
}

int32_t Gsm_Codec_Description::get_sampling_size_ms()
{
    return 20;
}

int32_t Gsm_Codec_Description::get_input_nr_samples()
{
    return GSM_EXEPECTED_INPUT;
}

std::string Gsm_Codec_Description::get_codec_name()
{
    return "GSM";
}

std::string Gsm_Codec_Description::get_codec_description()
{
    return "GSM CODEC (13.2kb/s)";
}

uint8_t Gsm_Codec_Description::get_sdp_media_type()
{
    return 3;
}

std::string Gsm_Codec_Description::get_sdp_media_attributes()
{
    return "GSM/8000";
}

uint32_t Gsm_Codec_Description::get_version() const
{
    return 0x00000001;
}
