#include "speex_codec.h"

#include<iostream>
#include "media_processor.h"
#include "rtp_packet.h"
#include<speex/speex.h>

using namespace std;

static std::list<std::string> pluginList;
static int initialized;

extern "C"
std::list<std::string> *mspeex_LTX_listPlugins( SRef<Library *> lib )
{
    if( !initialized )
    {
        pluginList.push_back("getPlugin");
        initialized = true;
    }
    return &pluginList;
}

extern "C"
SPlugin * mspeex_LTX_getPlugin( SRef<Library *> lib )
{
    return new Speex_Codec_Description( lib );
}

Speex_Decoder_Instance::Speex_Decoder_Instance(const SRef<Realtime_Media_Stream_Receiver*>& rtmsr)
    : Decoder_Instance(rtmsr)
{
    dec_initialized = false;
    memset(dec_buffer,0,2048);
    init_dec();
}

Speex_Decoder_Instance::~Speex_Decoder_Instance()
{
    free_dec();
}

void Speex_Decoder_Instance::init_dec()
{
    if (dec_initialized)
        free_dec();

    decLock.lock();

    speex_bits_init_buffer(&dec_bits, dec_buffer, 2000);

    dec_state = speex_decoder_init(&speex_wb_mode);
    massert(dec_state);

    int sample_rate = 16000;
    int ok = speex_decoder_ctl(dec_state, SPEEX_SET_SAMPLING_RATE, &sample_rate);
    my_assert(ok>=0);

    int frame_size;
    ok = speex_decoder_ctl(dec_state,SPEEX_GET_FRAME_SIZE,&frame_size);
    my_assert(frame_size==320);
    my_assert(ok>=0);
    dec_initialized=true;

    decLock.unlock();
}

void Speex_Decoder_Instance::free_dec()
{
    decLock.lock();
    if (dec_initialized)
    {
        speex_bits_destroy(&dec_bits);
        speex_decoder_destroy(dec_state);
        dec_initialized = false;
    }
    decLock.unlock();
}


SRef<Processing_Data*> Speex_Decoder_Instance::decode(const SRef<Rtp_Packet*>& rtp)
{
    unsigned char *in_data = rtp->get_content();
    int in_buf_size = rtp->get_content_length();

    SRef<Processing_Data_Audio*> data = new Processing_Data_Audio(true);
    data->samples = (short*)malloc(320*sizeof(short));
    data->nsamples = 320;
    data->sfreq = 16000;
    data->nchannels = 1;
    data->rtp_seq_no = rtp->get_header().get_seq_no();
    data->_rtp_timestamp = rtp->get_header().get_timestamp();
    data->rtp_marker = rtp->get_header().get_marker();
    data->ssrc= rtp->get_header().get_ssrc();
    short *out_data = data->samples;

    decLock.lock();
    speex_bits_read_from(&dec_bits, (char*)in_data, in_buf_size);
    int ret = speex_decode_int(dec_state, &dec_bits, (short*)out_data);
    if (ret!=0)
    {
        decLock.unlock();
        init_dec();
        decLock.lock();
        //TODO: set value in out_buf? better would be the samples from last time - are they in there?
    }
    decLock.unlock();
    return *data;
}


Speex_Encoder_Instance::Speex_Encoder_Instance()
{
    enc_initialized = false;
    memset(enc_buffer,0,2048);
    init_enc();
}

Speex_Encoder_Instance::~Speex_Encoder_Instance()
{
    free_enc();
}

void Speex_Encoder_Instance::init_enc()
{
    if (enc_initialized)
        free_enc();

    encLock.lock();

    speex_bits_init_buffer(&enc_bits, enc_buffer, 2000);
    enc_state = speex_encoder_init( &speex_wb_mode );
    my_assert(enc_state);

    int ok;
    int sample_rate = 16000;
    ok = speex_encoder_ctl(enc_state, SPEEX_SET_SAMPLING_RATE, &sample_rate);
    my_assert(ok>=0);

    //	int bitrate;
    //	speex_encoder_ctl(enc_state, SPEEX_GET_BITRATE, &bitrate);

    enc_initialized = true;
    encLock.unlock();
}

void Speex_Encoder_Instance::free_enc()
{
    encLock.lock();
    if (enc_initialized)
    {
        speex_bits_destroy(&enc_bits);
        speex_encoder_destroy(enc_state);
        enc_initialized = false;
    }
    encLock.unlock();
}

SRef<Processing_Data_Rtp*> Speex_Encoder_Instance::encode(const SRef<Processing_Data*>& data, uint16_t* seqNo, uint32_t ssrc)
{
    SRef<Processing_Data_Audio*> adata = dynamic_cast<Processing_Data_Audio*>(*data);
    my_assert(adata);
    short resampledData[1600];
    short* toEncode = adata->samples;

    if (adata->sfreq!=16000)
    {
        if (!resampler)
            resampler= Resampler_Registry::get_instance()->create( SOUND_CARD_FREQ, 16000, 20, 1 /*Nb channels */);
        resampler->resample( adata->samples, resampledData );
        toEncode = &resampledData[0];
    }

    encLock.lock();
    speex_bits_reset(&enc_bits);
    speex_encode_int(enc_state, toEncode, &enc_bits);
    unsigned char out_buf[MAX_NB_BYTES];
    int nbBytes = speex_bits_write(&enc_bits, (char*)out_buf,MAX_NB_BYTES /*640*/);
    encLock.unlock();

    *seqNo=*seqNo+1;
    SRef<SRtp_Packet*> rtp = new SRtp_Packet(&out_buf[0], nbBytes, *seqNo, data->_rtp_timestamp, ssrc);
    rtp->get_header().set_payload_type(/*rtpPayloadTypeNo*/ 119);
    SRef<Processing_Data_Rtp*> rdata = new Processing_Data_Rtp(rtp);
    return rdata;
}


Speex_Codec_Description::Speex_Codec_Description( SRef<Library *> lib )
    : Audio_Codec_Description( lib )
{
}

SRef<Decoder_Instance*> Speex_Codec_Description::new_decoder_instance(const SRef<Realtime_Media_Stream_Receiver*>&rtmsr)
{
    SRef<Decoder_Instance*> ret =  new Speex_Decoder_Instance(rtmsr);
    ret->set_codec( this );
    return ret;
}

SRef<Encoder_Instance*> Speex_Codec_Description::new_encoder_instance()
{
    SRef<Encoder_Instance*> ret =  new Speex_Encoder_Instance();
    ret->set_codec( this );
    return ret;
}

int32_t Speex_Codec_Description::get_sampling_freq()
{
    return 16000;
}

int32_t Speex_Codec_Description::get_sampling_size_ms()
{
    return 20;
}

int32_t Speex_Codec_Description::get_input_nr_samples()
{
    return 320;
}

std::string Speex_Codec_Description::get_codec_name()
{
    return "speex";
}

std::string Speex_Codec_Description::get_codec_description()
{
    return "SPEEX 16kHz, Speex";
}

uint8_t Speex_Codec_Description::get_sdp_media_type()
{
    return 119;
}

std::string Speex_Codec_Description::get_sdp_media_attributes()
{
    return "speex/16000/1";
}

uint32_t Speex_Codec_Description::get_version() const
{
    return 0x00000001;
}
