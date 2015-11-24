#include "simple_resampler.h"

#include <iostream>
#include <string.h>
using namespace std;

Simple_Resampler::Simple_Resampler( uint32_t inputFreq, uint32_t outputFreq,
                                    uint32_t duration, uint32_t nChannels_ )
{
    sample_ratio = 10000 * inputFreq / outputFreq;

    input_frames  = inputFreq * duration / 1000;
    output_frames = outputFreq * duration / 1000;

    this->nchannels = nChannels_;

    previous_frame = new short[ nchannels ];
    memset( previous_frame, '\0', nchannels * sizeof( short ) );
}

Simple_Resampler::~Simple_Resampler()
{
    delete [] previous_frame;
}

void Simple_Resampler::resample( short * input, short * output )
{
    if( sample_ratio / 10000 > 1 )
    {
        down_sample( input, output );
    }
    else if( sample_ratio / 10000 == 0 )
    {
        up_sample( input, output );
    }
    else
    {
        if( input != output && input_frames == output_frames )
        {
            memcpy( output, input, input_frames * nchannels * sizeof(short) );
        }
    }
}

/* Linear interpolation */
void Simple_Resampler::up_sample( short * input, short * output )
{
    int i, j, channel;
//	int epsilon = 0;
    int outputOffset = 0;
    //int sum;
    int sampleGroupSize;
    //int sample;
    int step;
    int lastInputSample;

    if( input_frames == 1 )
    {
        for( channel = 0 ; channel < (int)nchannels ; channel ++ )
        {
            for( j = 0; j < (int)output_frames; j++ )
            {
                output[j*nchannels + channel] = input[channel];
            }

        }
        return;
    }


    for( channel = 0; channel < (int)nchannels; channel ++ )
    {
        outputOffset = channel;
        sampleGroupSize = output_frames / input_frames;
        lastInputSample = previous_frame[ channel ];
        for (i = 0; i < (int)input_frames - 1; i++)
        {
            step = input[(i)*nchannels + channel] / sampleGroupSize  - lastInputSample / sampleGroupSize;

            output[outputOffset] = lastInputSample;

            for (j = 1; j < sampleGroupSize; j++)
            {
                output[outputOffset + j*nchannels] = output[outputOffset + (j - 1)*nchannels] + step;
            }

            outputOffset += sampleGroupSize*nchannels;

            lastInputSample = input[i*nchannels + channel];
        }

        /* For the last case, we take a slightly bigger sampleGroupSize
         * to cover the whole output buffer */

        sampleGroupSize = output_frames - outputOffset / nchannels;

        output[outputOffset] = lastInputSample;

        step = input[(i)*nchannels + channel] / sampleGroupSize
             - lastInputSample / sampleGroupSize;

        for (j = 1; j < sampleGroupSize; j++)
        {
            output[outputOffset + j*nchannels] = output[outputOffset + (j - 1)*nchannels] + step;
        }
    }

    /* Save the last input frame for smooth transition */
    memcpy( previous_frame, &input[ (input_frames - 1)*nchannels ], nchannels * sizeof(short) );
}

void Simple_Resampler::down_sample( short * input, short * output )
{
    int i, j, channel;
    int epsilon = 0;
    int inputOffset = 0;
    int sum;
    int sampleGroupSize;
    int sample;

    for( channel = 0; channel < (int)nchannels; channel ++ )
    {
        inputOffset = channel;
        for (i = 0; i < (int)output_frames; i++)
        {
            epsilon += sample_ratio;
            sampleGroupSize = epsilon / 10000;
            epsilon -= sampleGroupSize * 10000;

            sum = 0;
            for (j = 0; j < sampleGroupSize; j++)
            {
                sample = input[inputOffset + channel + j * nchannels ];
                sum += sample;
            }

            output[channel + nchannels * i] = (sum / sampleGroupSize);

            inputOffset += sampleGroupSize * nchannels;
        }
    }
}
