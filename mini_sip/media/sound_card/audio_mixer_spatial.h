#ifndef AUDIO_MIXER_SPATIAL_H
#define AUDIO_MIXER_SPATIAL_H

#include "audio_mixer.h"

class Sound_Source;
class Sp_Audio;

class Audio_Mixer_Spatial : public Audio_Mixer
{
public:
    Audio_Mixer_Spatial(SRef<Sp_Audio *> spatial);
    virtual ~Audio_Mixer_Spatial();

    virtual std::string get_mem_object_type() const { return "AudioMixerSpatial"; }

    virtual short * mix(std::list<SRef<Sound_Source *> > sources);

    virtual bool set_sources_position( std::list<SRef<Sound_Source *> > &sources, bool addingSource = true);
protected:
    bool sort_sound_source_list(std::list<SRef<Sound_Source *> > &srcList );
private:
    SRef< Sp_Audio *> sp_audio;

    Audio_Mixer_Spatial();
};

#endif // AUDIO_MIXER_SPATIAL_H
