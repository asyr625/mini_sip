#ifndef AUDIO_MIXER_SIMPLE_H
#define AUDIO_MIXER_SIMPLE_H

#include"audio_mixer.h"

class Sound_Source;

class Audio_Mixer_Simple : public Audio_Mixer
{
public:
    Audio_Mixer_Simple();
    virtual ~Audio_Mixer_Simple();

    virtual std::string get_mem_object_type() const {return "AudioMixerSimple";}

    virtual short * mix(std::list<SRef<Sound_Source *> > sources);
    virtual bool init( uint32_t numChannels_ );
    virtual bool set_sources_position( std::list<SRef<Sound_Source *> > &sources, bool addingSource = true);
protected:
    virtual inline bool normalize( int32_t length);
    virtual inline bool normalize_mono( int32_t length);
    virtual inline bool normalize_stereo( int32_t length);
    virtual inline bool normalize_multi( int32_t length);
private:
    int32_t normalize_factor;
};

#endif // AUDIO_MIXER_SIMPLE_H
