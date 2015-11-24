#include "float_resampler.h"

#include<iostream>


static std::list<std::string> pluginList;
static bool initialized;

extern "C"
std::list<std::string> *mfloat_resampler_LTX_listPlugins( SRef<Library*> lib )
{
    if( !initialized )
    {
        pluginList.push_back("getPlugin");
        initialized = true;
    }
    return &pluginList;
}

extern "C"
SPlugin * mfloat_resampler_LTX_getPlugin( SRef<Library*> lib )
{
    return new Float_Resampler_Plugin( lib );
}


Float_Resampler::Float_Resampler( uint32_t inputFreq, uint32_t outputFreq,
                                  uint32_t duration, uint32_t nChannels )
{
    src_data = new SRC_DATA();
    src_data->input_frames  = inputFreq * duration / 1000;
    src_data->output_frames = outputFreq * duration / 1000;

    input_length  = src_data->input_frames  * nChannels;
    output_length = src_data->output_frames * nChannels;

    src_data->src_ratio = (float)outputFreq / inputFreq;

    src_data->data_in  = new float[input_length];
    src_data->data_out = new float[output_length];

    src_state = src_new( 2, nChannels, &error );
}

Float_Resampler::~Float_Resampler()
{
    if( src_data )
    {
        delete [] src_data->data_in;
        delete [] src_data->data_out;
        delete src_data;
    }

    if( src_state )
    {
        src_delete( src_state );
    }
}

void Float_Resampler::resample( short * input, short * output )
{
    if( src_data && src_state )
    {
        src_short_to_float_array( input, src_data->data_in, input_length );
        src_process(src_state,src_data);
        src_float_to_short_array( src_data->data_out, output, output_length );
    }
}
