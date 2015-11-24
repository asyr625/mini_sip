#ifndef FLOAT_RESAMPLER_H
#define FLOAT_RESAMPLER_H

#include "resampler.h"

#include <samplerate.h>

class Float_Resampler : public Resampler
{
public:
    Float_Resampler( uint32_t inputFreq, uint32_t outputFreq,
                uint32_t duration, uint32_t nChannels );

    ~Float_Resampler();

    virtual void resample( short * input, short * output );

private:
    uint32_t input_length;
    uint32_t output_length;

    SRC_DATA * src_data;
    SRC_STATE * src_state;

    int error;
};

class Float_Resampler_Plugin: public Resampler_Plugin
{
public:
    Float_Resampler_Plugin( SRef<Library *> lib ): Resampler_Plugin( lib ){}

    virtual std::string get_name() const { return "float_resampler"; }

    virtual uint32_t get_version() const { return 0x00000001; }

    virtual std::string get_description() const { return "Float resampler"; }

    virtual SRef<Resampler *> create_resampler( uint32_t inputFreq, uint32_t outputFreq,
            uint32_t duration, uint32_t nChannels ) const
    {
        return new Float_Resampler( inputFreq, outputFreq, duration, nChannels );
    }

    virtual std::string get_mem_object_type() const { return "FloatResamplerPlugin"; }
};

#endif // FLOAT_RESAMPLER_H
