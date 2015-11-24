#ifndef ALSA_SOUND_DEVICE_H
#define ALSA_SOUND_DEVICE_H

#include "sound_device.h"

#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API
#include <alsa/asoundlib.h>
#define MIN_HW_PO_BUFFER (20 * 1000)

class Alsa_Sound_Device : public Sound_Device
{
public:
    Alsa_Sound_Device( std::string device );
    virtual int read_from_device( byte_t * buffer, uint32_t nSamples );
    virtual int read_error( int errcode, byte_t * buffer, uint32_t nSamples );

    virtual int write_to_device( byte_t * buffer, uint32_t nSamples );
    virtual int write_error( int errcode, byte_t * buffer, uint32_t nSamples );

    virtual int open_record( int samplingRate, int nChannels, int format );
    virtual int close_record();

    virtual int open_playback( int samplingRate, int nChannels, int format );
    virtual int close_playback();

    virtual void sync();

    virtual std::string get_mem_object_type() const { return "AlsaSoundDevice";}

private:
    int calculate_alsa_params(const bool &periodSizePresent,
                              unsigned long &periodSizeMin,
                              unsigned long &periodSizeMax,
                              const bool &periodCountPresent,
                              uint32_t &periodsMin,
                              uint32_t &periodsMax,
                              unsigned long &maxBufferSize);

    unsigned long period_size;
    uint32_t num_periods;
    unsigned long buffer_size;

    snd_pcm_t * read_handle;
    snd_pcm_t * write_handle;

    Mutex lock_open;
};

#endif // ALSA_SOUND_DEVICE_H
