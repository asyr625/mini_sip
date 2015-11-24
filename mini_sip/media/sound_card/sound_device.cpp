#include "sound_device.h"
#include "sound_driver_registry.h"
#include "my_time.h"
#include "thread.h"

SRef<Sound_Device *> Sound_Device::create( std::string deviceId )
{
    if( deviceId == "" )
        return NULL;

    SRef<Sound_Device*> device = Sound_Driver_Registry::get_instance()->create_device( deviceId );
    if( device )
    {
        return device;
    }

#ifdef WAVE_SOUND
#define SOUND_DEVICE_IMPLEMENTED
    if( deviceId.substr( 0, 5 ) == "wave:" )
    {
        return new Wave_Sound_Device( deviceId.substr( 5, string::npos ) );
    }

#endif
    return NULL;
}

Sound_Device::Sound_Device( std::string fileName )
    : opened_record(false), opened_playback(false)
{
    dev = fileName;
    set_sleep_time(20);
}

Sound_Device::~Sound_Device()
{
}

void Sound_Device::set_format( int format_ )
{
    switch( format_ ) {
    case SOUND_S16LE:
    case SOUND_S16BE:
    case SOUND_U16LE:
    case SOUND_U16BE:
        sample_size = 2;
        this->format = format_;
        break;
    case SOUND_S8LE:
    case SOUND_U8LE:
        sample_size = 1;
        this->format = format_;
        break;
    case SOUND_S32LE:
    case SOUND_U32LE:
        sample_size = 4;
        this->format = format_;
        break;
    default:
        cerr << "Sound_Device::set_format - format not understood!" << endl;
        break;
    }
}

void Sound_Device::lock_read()
{
    mlock_read.lock();
}

void Sound_Device::unlock_read()
{
    mlock_read.unlock();
}

void Sound_Device::lock_write()
{
    mlock_write.lock();
}

void Sound_Device::unlock_write()
{
    mlock_write.unlock();
}

int Sound_Device::read( byte_t * buffer, uint32_t nSamples )
{
    int nSamplesRead = 0;
    int totalSamplesRead = 0;

    if( !opened_record )
        return -1;

    byte_t * byteBuffer = buffer;

    while( (uint32_t)totalSamplesRead < nSamples )
    {
        nSamplesRead = read_from_device( byteBuffer, nSamples - totalSamplesRead );
        if( nSamplesRead >= 0 ) // does it actively loop while waiting for samples? :-/
        {
            // 			fprintf( stderr, "nSamplesRead %d\n", nSamplesWritten );
            byteBuffer += nSamplesRead * get_sample_size() * get_nchannels_play();
            totalSamplesRead += nSamplesRead;
        }
        else
        {
            nSamplesRead = read_error( nSamplesRead, byteBuffer, nSamples - totalSamplesRead );
            if( nSamplesRead < 0 )
            {
                return -1;
            }
            else
                continue;
        }
    }
    return totalSamplesRead;
}

int Sound_Device::write( byte_t * buffer, uint32_t nSamples )
{
    byte_t * byteBuffer = buffer;

    if( !opened_playback )
        return -1;

    int nSamplesWritten = 0;
    int totalSamplesWritten = 0;

    //timed access ..
    uint64_t currentTime;
    static uint64_t lastTimeWrite = 0;

    if( sleep_time > 0 )
    {
        currentTime = my_time();
        if( lastTimeWrite == 0 )
        {
            lastTimeWrite = currentTime - sleep_time; //init last time we wrote ...

#ifdef DEBUG_OUTPUT
            printf( "nsamples = %d\n\n", nSamples );
#endif
        }
        else if( (currentTime - lastTimeWrite) > sleep_time*10 )
        {

#ifdef DEBUG_OUTPUT
            printf( "SoundDevice: resetting lastTimeWrite! +++++++++++++++++++++++++++++ \n\n");
#endif
            lastTimeWrite = currentTime - sleep_time;
        }

        int64_t sleep = sleep_time - (currentTime-lastTimeWrite);
        // 	printf( "\n\nsleep = %d\n", sleep );
        while ( sleep > 0 )
        {
            Thread::msleep( (int32_t)sleep );
            currentTime = my_time();
            sleep = sleep_time - (currentTime-lastTimeWrite);
        }
        lastTimeWrite += sleep_time;
    }

    //cerr <<"EEEE: write: nSamples="<<nSamples<<endl;
    while( (uint32_t)totalSamplesWritten < nSamples )
    {
        //cerr <<"EEEE: write..."<<endl;
        nSamplesWritten = write_to_device( byteBuffer, nSamples - totalSamplesWritten );
        //cerr <<"EEEE: nSamplesWritten="<<nSamplesWritten<<endl;

        if( nSamplesWritten >= 0 )
        {
            //fprintf( stderr, "nSamplesWritten %d\n", nSamplesWritten );
            byteBuffer += nSamplesWritten * get_sample_size() * get_nchannels_play();
            totalSamplesWritten += nSamplesWritten;
        }
        else
        {
            //cerr <<"EEEE: warning - writeError..."<<endl;
            nSamplesWritten = write_error( nSamplesWritten, byteBuffer, nSamples - totalSamplesWritten );
            if( nSamplesWritten < 0 )
                return -1;
            else
                continue;
        }
    }
    return totalSamplesWritten;
}
