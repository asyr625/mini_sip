#ifndef SOUND_DEVICE_H
#define SOUND_DEVICE_H

#include "my_types.h"
#include "sobject.h"
#include "mutex.h"
#include <iostream>
using namespace std;
/**
Define sound types, specially useful for the soundcard access
SOUND_XNNYE
 - X  is S or U, meaning signed or unsigned samples
 - NN is the size of the samples, in bits (8, 16, 32, .. )
 - Y is the endiannes (L little, B big).

The defined values have hardware meaning when opening a real soundcard.
(for now, only the 16 bit ones ... the rest are random values.
*/
#define SOUND_S16LE 0xF0
#define SOUND_S16BE 0xF1
#define SOUND_U16LE	0xF2
#define SOUND_U16BE	0xF3
#define SOUND_S8LE	0x01
#define SOUND_U8LE	0x02
#define SOUND_S32LE	0x03
#define SOUND_U32LE	0x04

class Sound_Device : public SObject
{
public:
    static SRef<Sound_Device *> create( std::string deviceId );

    virtual ~Sound_Device();

    virtual int open_record( int32_t samplingRate, int nChannels, int format ) = 0;

    virtual int open_playback( int32_t samplingRate, int nChannels, int format ) = 0;

    virtual int close_record() = 0;

    virtual int close_playback() = 0;

    bool is_opened_playback() { return opened_playback; }
    bool is_opened_record() { return opened_record; }

    virtual int read( byte_t * buffer, uint32_t nSamples );

    virtual int read_from_device( byte_t * buffer, uint32_t nSamples ) = 0;

    virtual int read_error( int errcode, byte_t * buffer, uint32_t nSamples ) = 0;

    virtual int write( byte_t * buffer, uint32_t nSamples );

    virtual int write_to_device( byte_t * buffer, uint32_t nSamples ) = 0;

    virtual int write_error( int errcode, byte_t * buffer, uint32_t nSamples ) = 0;

    virtual void sync() = 0;

    void lock_read();
    void unlock_read();
    void lock_write();
    void unlock_write();

    std::string dev;

    int get_sampling_rate(){ return sampling_rate; }
    int get_nchannels_play(){ return nchannels_play; }
    int get_nchannels_record(){ return nchannels_record; }

    virtual void set_format( int format_ );

    int get_sample_size() { return sample_size; }
    void set_sleep_time( int sleep ) { sleep_time = sleep; }
    int get_sleep_time() { return sleep_time; }

protected:
    Sound_Device( std::string fileName );
    int sampling_rate;
    int nchannels_play;
    int nchannels_record;
    int format;
    int sample_size;
    uint32_t sleep_time;

    bool opened_record;
    bool opened_playback;

private:
    Mutex mlock_read;
    Mutex mlock_write;
};

#endif // SOUND_DEVICE_H
