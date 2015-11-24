#include "audio_mixer.h"
#include "audio_defines.h"
#include <string.h>
Audio_Mixer::Audio_Mixer()
    : num_channels(0), frame_size(0),
      output_buffer(NULL), input_buffer(NULL), mix_buffer(NULL)
{
}

Audio_Mixer::~Audio_Mixer()
{
    if( output_buffer )
        delete [] output_buffer;
    if( input_buffer )
        delete [] input_buffer;
    if( mix_buffer )
        delete [] mix_buffer;
}

bool Audio_Mixer::init( uint32_t numChannels_ )
{
    bool hasChanged = (this->num_channels != numChannels_ );

    this->num_channels = numChannels_;
    this->frame_size = (SOUND_CARD_FREQ * 20) / 1000;

    //we may need to re-new the buffers if
    //either of these change ...
    if( !output_buffer || hasChanged )
    {
        my_assert(this->num_channels * this->frame_size);
        output_buffer = new short[this->num_channels * this->frame_size];
        memset(output_buffer,0,this->num_channels * this->frame_size*sizeof(short));
    }
    if( !input_buffer || hasChanged )
    {
        input_buffer = new short[this->num_channels * this->frame_size];
        memset(input_buffer, 0, this->num_channels * this->frame_size*sizeof(short));
    }
    if( !mix_buffer || hasChanged )
    {
        mix_buffer = new int32_t[this->num_channels * this->frame_size];
        memset(mix_buffer,0,this->num_channels * this->frame_size*sizeof(int32_t));
    }
#ifdef DEBUG_OUTPUT
    cerr << "Audio_Mixer::init() ... initializing audio mixer" << endl;
    printf( "Audio_Mixer::init - frameSize = %d, numChannels = %d\n", frame_size, num_channels );
#endif
    return true;
}
