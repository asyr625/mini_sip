#include "audio_mixer_simple.h"
#include "sound_source.h"
#include "dbg.h"

#include<string.h>
#include<stdlib.h> //abs()

using namespace std;

Audio_Mixer_Simple::Audio_Mixer_Simple()
{
}

Audio_Mixer_Simple::~Audio_Mixer_Simple()
{
}

bool Audio_Mixer_Simple::init( uint32_t numChannels_ )
{
    Audio_Mixer::init( numChannels_ );

    normalize_factor = 32;

    return true;
}

short * Audio_Mixer_Simple::mix(std::list<SRef<Sound_Source *> > sources)
{
    uint32_t size = frame_size * num_channels;

    memset( mix_buffer, '\0', size * sizeof( int32_t ) );

    for (list<SRef<Sound_Source *> >::iterator i = sources.begin(); i != sources.end(); i++)
    {
        (*i)->get_sound( input_buffer );

        for (uint32_t j=0; j<size; j++)
        {
#ifdef IPAQ
            /* iPAQ hack, to reduce the volume of the
                * output */
            mix_buffer[j] += (input_buffer[j]/32);
#else
            mix_buffer[j] += input_buffer[j];
#endif
        }
    }
    //mix buffer is 32 bit to prevent saturation ...
    // normalize, if needed, to prevent it
    normalize( size );
    return output_buffer;
}

bool Audio_Mixer_Simple::set_sources_position( std::list<SRef<Sound_Source *> > &sources, bool addingSource = true)
{
    return true;
}

bool Audio_Mixer_Simple::normalize( int32_t length)
{
    bool ret = false;
    if( num_channels < 1 )
        ret = false;
    else if (num_channels == 1 ) {
        ret = normalize_mono( length );
    } else if( num_channels == 2 ) {
        ret = normalize_stereo( length );
    } else {
        ret = normalize_multi( length );
    }
    return ret;
}

bool Audio_Mixer_Simple::normalize_mono( int32_t length)
{
    short * outbuff = output_buffer;
    int32_t * inbuff = mix_buffer;

    //indicates the end ...
    short * end = outbuff + length;

    if( normalize_factor < 64 )
        normalize_factor++;

    while( outbuff != end )
    {
        int32_t sample = (*inbuff * normalize_factor) >> 6;
        if( abs(sample) > NORMALIZE_MAX_RANGE)
        {
            normalize_factor = abs( (NORMALIZE_MAX_RANGE<<6) / (*inbuff) );
            if( sample < 0 )
                sample = -NORMALIZE_MAX_RANGE;
            else
                sample = NORMALIZE_MAX_RANGE;
        }
        *(outbuff++) = short(sample);
        inbuff++;
    }
    return true;
}

bool Audio_Mixer_Simple::normalize_stereo( int32_t length)
{
    //we need running pointers ...
    short * outbuff = output_buffer;
    int32_t * inbuff = mix_buffer;
    int32_t * sample = new int32_t[2];


    //indicates the end ...
    short * end = outbuff + length;
    int32_t originalSample;

    if( normalize_factor < 64 )
        normalize_factor++;

    while( outbuff != end )
    {
        originalSample = *inbuff;
        sample[0] = (*inbuff * normalize_factor) >> 6;
        inbuff++;
        sample[1] = (*inbuff * normalize_factor) >> 6;

        if( abs(sample[0]) > NORMALIZE_MAX_RANGE)
        {
            normalize_factor = abs( (NORMALIZE_MAX_RANGE<<6) / originalSample );
            if( sample[0] < 0 )
                sample[0] = -NORMALIZE_MAX_RANGE;
            else
                sample[0] = NORMALIZE_MAX_RANGE;
            if( sample[1]< 0 )
                sample[1] = -NORMALIZE_MAX_RANGE;
            else
                sample[1] = NORMALIZE_MAX_RANGE;
#ifdef DEBUG_OUTPUT
            my_err << "n";
#endif
        }

        *(outbuff++) = short(sample[0]);
        *(outbuff++) = short(sample[1]);

        inbuff++;
    }
    if( sample )
        delete [] sample;
    return true;
}

bool Audio_Mixer_Simple::normalize_multi( int32_t length)
{
    //we need running pointers ...
    short * outbuff = output_buffer;
    int32_t * inbuff = mix_buffer;
    uint32_t i;
    int32_t * sample = new int32_t[numChannels];
    int32_t originalSample;

    //indicates the end ...
    short * end = outbuff + length;

    if( normalize_factor < 64 )
        normalize_factor++;

    while( outbuff != end )
    {
        originalSample = *inbuff; //keep it, we may need it to normalize

        for( i = 0; i<num_channels; i++ )
        {
            sample[i] = (*inbuff * normalize_factor) >> 6;
            inbuff++;
        }

        if( abs(sample[0]) > NORMALIZE_MAX_RANGE)
        {
            normalize_factor = abs( (NORMALIZE_MAX_RANGE<<6) / originalSample );
            //after updating the norm factor ...
            //update all the samples from the channels
            for( i = 0; i<num_channels; i++ )
            {
                if( sample[i]< 0 )
                    sample[i] = -NORMALIZE_MAX_RANGE;
                else
                    sample[i] = NORMALIZE_MAX_RANGE;
            }
        }

        for( i = 0; i<num_channels; i++ )
        {
            *outbuff = short(sample[i]);
            outbuff++;
        }
    }
    if( !sample )
        delete [] sample;
    return true;
}
