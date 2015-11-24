#ifndef OSS_SOUND_DEVICE_H
#define OSS_SOUND_DEVICE_H

#include "sound_device.h"

class Oss_Sound_Device : public Sound_Device
{
public:
    Oss_Sound_Device( std::string device );

    virtual int read_from_device( byte_t * buffer, uint32_t nSamples );
    virtual int write_to_device( byte_t * buffer, uint32_t nSamples );

    virtual int read_error( int errcode, byte_t * buffer, uint32_t nSamples );
    virtual int write_error( int errcode, byte_t * buffer, uint32_t nSamples );

    virtual int open_playback( int32_t samplingRate, int nChannels, int format );
    virtual int open_record( int32_t samplingRate, int nChannels, int format );

    virtual int close_playback();
    virtual int close_record();

    virtual void sync();

    virtual std::string get_mem_object_type() const { return "OssSoundDevice";}

private:
    int fd_playback;
    int fd_record;

    int fragment_setting;
};

#endif // OSS_SOUND_DEVICE_H
