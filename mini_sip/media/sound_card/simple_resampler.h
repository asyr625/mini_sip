#ifndef SIMPLE_RESAMPLER_H
#define SIMPLE_RESAMPLER_H

#include "resampler.h"

class Simple_Resampler
{
public:
    virtual void resample( short * input, short * output );
    Simple_Resampler( uint32_t inputFreq, uint32_t outputFreq,
                      uint32_t duration, uint32_t nChannels );
    ~Simple_Resampler();

private:
    void up_sample( short * input, short * output );
    void down_sample( short * input, short * output );

    uint32_t input_frames;
    uint32_t output_frames;

    uint32_t nchannels;
    uint32_t sample_ratio;

    short * previous_frame;
};


class Simple_Resampler_Plugin: public Resampler_Plugin
{
public:
    Simple_Resampler_Plugin( SRef<Library *> lib ): Resampler_Plugin( lib ){}

    virtual std::string get_name() const { return "simple_resampler"; }

    virtual uint32_t get_version() const { return 0x00000001; }

    virtual std::string get_description() const { return "Simple resampler"; }

    virtual SRef<Resampler *> create_resampler( uint32_t inputFreq, uint32_t outputFreq,
            uint32_t duration, uint32_t nChannels ) const
    {
        return (Resampler*)new Simple_Resampler( inputFreq, outputFreq, duration, nChannels );
    }

    virtual std::string get_mem_object_type() const { return "SimpleResamplerPlugin"; }
};


#endif // SIMPLE_RESAMPLER_H
