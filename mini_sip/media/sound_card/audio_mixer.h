#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H

#include <list>

#include "sobject.h"
#include "mutex.h"

#include "sound_source.h"

#define NORMALIZE_MAX_RANGE 32737

class Audio_Mixer : public SObject
{
public:
    Audio_Mixer();
    virtual ~Audio_Mixer();

    virtual std::string get_mem_object_type() const { return "AudioMixer"; }
    virtual short * mix(std::list<SRef<Sound_Source *> > sources) = 0;

    virtual bool init( uint32_t numChannels_ );

    virtual bool set_sources_position( std::list<SRef<Sound_Source *> > &sources, bool addingSource = true) = 0;

    uint32_t get_num_channels() {return num_channels;}

    uint32_t get_frame_size() {return frame_size;}

protected:
    uint32_t num_channels;
    uint32_t frame_size;
    short * output_buffer;
    short * input_buffer;
    int32_t * mix_buffer;
};

#endif // AUDIO_MIXER_H
