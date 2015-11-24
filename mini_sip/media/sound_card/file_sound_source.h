#ifndef FILE_SOUND_SOURCE_H
#define FILE_SOUND_SOURCE_H

#include "sound_source.h"

class File_Sound_Source : public Sound_Source
{
public:
    File_Sound_Source(std::string callId, std::string filename, uint32_t id,
        uint32_t inputFreq,
        uint32_t inputNChannels,
        uint32_t outputFreq,
        uint32_t outputDurationMs,
        uint32_t outputNChannels,
        bool repeat=false);

    File_Sound_Source(std::string callId, short *raw_audio, int samples, bool repeat=false);

    ~File_Sound_Source();

    void enable();
    void disable();

    virtual void push_sound(short *samples, int32_t nSamples, int32_t index, int sampleRate, bool isStereo=false);
    virtual void get_sound(short *dest, bool dequeue=true);
private:
    short *audio;
    uint32_t nsamples;
    bool enabled;
    bool repeat;
    int index;
    uint32_t nchannels;
    uint32_t noutput_frames;
    uint32_t noutput_frames_resampled;

    SRef<Resampler *> resampler;
};

#endif // FILE_SOUND_SOURCE_H
