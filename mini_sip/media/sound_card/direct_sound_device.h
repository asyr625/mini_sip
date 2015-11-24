#ifndef DIRECT_SOUND_DEVICE_H
#define DIRECT_SOUND_DEVICE_H

#include "sound_device.h"

#include<dsound.h>

class Direct_Sound_Device : public Sound_Device
{
public:
    Direct_Sound_Device( std::string fileName );

    virtual ~Direct_Sound_Device();

    virtual int openRecord( int samplingRate, int nChannels, int format );
    virtual int openPlayback( int samplingRate, int nChannels, int format );

    virtual int close_record();
    virtual int close_playback();

    virtual int read_from_device( byte_t * buffer, uint32_t nSamples );
    virtual int write_to_device( byte_t * buffer, uint32_t nSamples );

    virtual int read_error( int errcode, byte_t * buffer, uint32_t nSamples );
    virtual int write_error( int errcode, byte_t * buffer, uint32_t nSamples );

    virtual void sync();

private:

    HRESULT setCaptureNotificationPoints(LPDIRECTSOUNDCAPTUREBUFFER8 pDSCB);

    LPDIRECTSOUNDFULLDUPLEX dsDuplexInterfaceHandle;

    LPDIRECTSOUNDBUFFER8        outputBufferHandle; //secondary directsound buffer
    LPDIRECTSOUNDCAPTUREBUFFER8        inputBufferHandle;

#define cEvents  2
    HANDLE     inputSoundEvent[cEvents];
};

#endif // DIRECT_SOUND_DEVICE_H
