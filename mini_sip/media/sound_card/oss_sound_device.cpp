#include "oss_sound_device.h"

#ifndef DISABLE_OSS
#include<config.h>

#include "my_error.h"
#include "dbg.h"

#include <string.h>
#include <unistd.h>
#include <errno.h>

#define PLAYOUT_FRAGMENT_SETTINGS 0x0002000C
#define RECORD_FRAGMENT_SETTINGS  0x0014000C

#define OPEN_OSS_IN_NON_BLOCKING_MODE true

using namespace std;
Oss_Sound_Device::Oss_Sound_Device( std::string device )
    : Sound_Device( device )
{
    fd_playback = -1;
    fd_record = -1;
}

int Oss_Sound_Device::read_from_device( byte_t * buffer, uint32_t nSamples )
{
    int nReadBytes = 0;
    int nBytesToRead = nSamples * get_sample_size() * get_nchannels_record();
    int totalSamplesRead = 0;

    if( fd_record == -1 )
        return -1;

    nReadBytes = ::read( fd_record, buffer, nBytesToRead );

    if( nReadBytes >= 0 )
    {
        totalSamplesRead = nReadBytes / ( get_sample_size() * get_nchannels_record() );
        if(nReadBytes % ( get_sample_size() * get_nchannels_record() ))
            cerr << "OssSoundDevice::readFromDevice - partial sample read! Data alignment lost as a result!" << std::endl;
    } else {
        totalSamplesRead = -errno;
    }
    return totalSamplesRead;
}

int Oss_Sound_Device::write_to_device( byte_t * buffer, uint32_t nSamples )
{
    int nWrittenBytes = 0;
    int nBytesToWrite = nSamples * get_sample_size() * get_nchannels_play();
    int totalSamplesWritten = 0;

    if( fd_playback == -1 )
        return -1;

    nWrittenBytes = ::write( fd_playback, buffer, nBytesToWrite );

    if( nWrittenBytes >= 0 )
    {
        //convert back to samples ...
        totalSamplesWritten = nWrittenBytes / ( get_sample_size() * get_nchannels_play() );
        if(nWrittenBytes % ( get_sample_size() * get_nchannels_record() ))
            cerr << "OssSoundDevice::writeToDevice - partial sample written! Data alignment lost as a result!" << std::endl;
    } else {
        totalSamplesWritten = -errno;
    }
    return totalSamplesWritten;
}

int Oss_Sound_Device::read_error( int errcode, byte_t * buffer, uint32_t nSamples )
{
    bool mustReturn = true;
    switch( errcode )
    {
    case -EAGAIN:
    case -EINTR:
        mustReturn = false;
        break;
    default:
        mustReturn = true;
        break;
    }
    if( mustReturn ) { return -1; }
    else { return 0; }
}

int Oss_Sound_Device::write_error( int errcode, byte_t * buffer, uint32_t nSamples )
{
    bool mustReturn = true;
    switch( errcode )
    {
    case -EAGAIN:
    case -EINTR:
        mustReturn = false;
        break;
    default:
        mustReturn = true;
        break;
    }
    if( mustReturn ) { return -1; }
    else { return 0; }
}

int Oss_Sound_Device::open_playback( int32_t samplingRate, int nChannels, int format )
{
    this->nchannels_play = nChannels;

    if( is_opened_playback() )
        return 0;

    int mode = O_WRONLY;
    /* FIXME */
    this->fragment_setting = PLAYOUT_FRAGMENT_SETTINGS;

    fd_playback = ::open( dev.c_str(), mode | O_NONBLOCK );

    if( fd_playback == -1 )
    {
        my_err << "Could not open the sound device " << dev <<
                  " for playback: "
               << strerror( errno ) << endl;
        return -1;
    }

    bool openNonBlocking = OPEN_OSS_IN_NON_BLOCKING_MODE;

    if( openNonBlocking )
    {
        sleep_time = 20; //min time between calls ... simulated
#ifdef DEBUG_OUTPUT
        cerr << "OSS: opening playback in non-blocking mode" << endl;
#endif
    } else {
        int flags = fcntl( fd_playback, F_GETFL );
        sleepTime = 0;
        // Remove O_NONBLOCK
        flags &= ~O_NONBLOCK;
        fcntl( fd_playback, F_SETFL, flags );
#ifdef DEBUG_OUTPUT
        cerr << "OSS: opening playback in blocking mode" << endl;
#endif
    }

    if( ioctl( fd_playback, SNDCTL_DSP_SETFRAGMENT, &fragment_setting ) == -1 )
    {
#ifdef DEBUG_OUTPUT
        my_error( "ioctl, SNDCTL_DSP_SETFRAGMENT (set buffer size)" );
#endif
    }

    /*
    if( channels != this->nChannelsPlay - 1 ){
        cerr << "ERROR: could not set to stereo- running mono"<< endl;
        if( this->nChannelsPlay == 2 ) channels = 0;
        else channels = 1;
        if( ioctl( fdPlayback, SNDCTL_DSP_STEREO, &channels ) == -1 ){
            my_error("ioctl, SNDCTL_DSP_STEREO (tried to fallback)");
        }

    }
*/
    set_format( format );
    int ossFormat = format;

    switch( format )
    {
    case SOUND_S16LE:
        ossFormat = AFMT_S16_LE;
        break;
    case SOUND_S16BE:
        ossFormat = AFMT_S16_BE;
        break;
    case SOUND_U16LE:
        ossFormat = AFMT_U16_LE;
        break;
    case SOUND_U16BE:
        ossFormat = AFMT_U16_BE;
    }

    if( ioctl( fd_playback, SNDCTL_DSP_SETFMT, &ossFormat ) == -1 ){
#ifdef DEBUG_OUTPUT
        my_error( "ioctl, SNDCTL_DSP_SETFMT (failed to set format to AFMT_S16_LE)" );
#endif
    }

#ifdef DEBUG_OUTPUT
    cerr << "OSSSoundDevice format set to" << ossFormat << endl;
#endif

    int channels = nChannels;
    this->nchannels_play = nChannels;

    if( ioctl( fd_playback, SNDCTL_DSP_CHANNELS, &channels ) == -1 )
    {
#ifdef DEBUG_OUTPUT
        my_error("ioctl, SNDCTL_DSP_CHANNELS (tried to set channels number)");
#endif
    }

#ifdef DEBUG_OUTPUT
    cerr << "OssSoundDevice: number of channels set to "<< channels << endl;
#endif

    this->nchannels_play = channels;

    int setSpeed;

    /* remove because of the use of spatial audio

#ifdef IPAQ
    // The iPAQ h5550 is known not to support 8kHz, we use 16kHz and
    // resample
    if( samplingRate == 8000 ){
        setSpeed = 16000;
        cerr << "Enabling iPAQ frequency workaround" << endl;
    }
    else
#endif
    */
    setSpeed = samplingRate;

    if( ioctl( fd_playback, SNDCTL_DSP_SPEED, &setSpeed ) == -1 )
    {
#ifdef DEBUG_OUTPUT
        my_error( "ioctl, SNDCTL_DSP_SPEED (tried to set sample rate to 8000)" );
#endif
    }

    this->sampling_rate = setSpeed;

#ifdef DEBUG_OUTPUT
    cerr << "OSSDevice: DSP speed set to "<< this->sampling_rate << endl;
#endif

    opened_playback = true;
    return 0;
}

int Oss_Sound_Device::open_record( int32_t samplingRate, int nChannels, int format )
{
    this->nchannels_record = nChannels;
    if( is_opened_record() )
        return 0;

    int mode = O_RDONLY; /*duplex ? O_RDWR : O_WRONLY;*/
    /* FIXME */
    this->fragment_setting = RECORD_FRAGMENT_SETTINGS;

    fd_record = ::open( dev.c_str(), mode | O_NONBLOCK );

    if( fd_record == -1 )
    {
#ifdef DEBUG_OUTPUT
        my_err << "Could not open the sound device " << dev <<
                " for recording: "
             << strerror( errno ) << endl;
#endif
        return -1;
    }

    // Remove O_NONBLOCK
    int flags = fcntl( fd_record, F_GETFL );
    flags &= ~O_NONBLOCK;
    fcntl( fd_record, F_SETFL, flags );

    if( ioctl( fd_record, SNDCTL_DSP_SETFRAGMENT, &fragment_setting ) == -1 )
    {
#ifdef DEBUG_OUTPUT
        my_error( "ioctl, SNDCTL_DSP_SETFRAGMENT (set buffer size)" );
#endif
    }

    //int channels = 1;
    int channels = nChannels;
    //this->nChannels = nChannels;

    if( ioctl( fd_record, SNDCTL_DSP_CHANNELS, &channels ) == -1 )
    {
#ifdef DEBUG_OUTPUT
        my_error("ioctl, SNDCTL_DSP_CHANNELS");
#endif
    }

#ifdef DEBUG_OUTPUT
    cerr << "OssSoundDevice: number of channels set to "<< channels << endl;
#endif

    this->nchannels_record = channels;

    set_format( format );
    int ossFormat = format;

    switch( format )
    {
    case SOUND_S16LE:
        ossFormat = AFMT_S16_LE;
        break;
    case SOUND_S16BE:
        ossFormat = AFMT_S16_BE;
        break;
    case SOUND_U16LE:
        ossFormat = AFMT_U16_LE;
        break;
    case SOUND_U16BE:
        ossFormat = AFMT_U16_BE;
    }

    if( ioctl( fd_record, SNDCTL_DSP_SETFMT, &ossFormat ) == -1 )
    {
        my_error( "ioctl, SNDCTL_DSP_SETFMT (failed to set format to AFMT_S16_LE)" );
    }

    int setSpeed;

#ifdef IPAQ
    // The iPAQ h5550 is known not to support 8kHz, we use 16kHz and
    // resample
    if( samplingRate == 8000 )
    {
        setSpeed = 16000;
        cerr << "Enabling iPAQ frequency workaround" << endl;
    }
    else
#endif
        setSpeed = samplingRate;

    if( ioctl( fd_record, SNDCTL_DSP_SPEED, &setSpeed ) == -1 )
    {
#ifdef DEBUG_OUTPUT
        my_error( "ioctl, SNDCTL_DSP_SPEED (tried to set sample rate to 8000)" );
#endif
    }

    this->sampling_rate = setSpeed;

#ifdef DEBUG_OUTPUT
    cerr << "DSP speed set to "<< this->sampling_rate << endl;
#endif

    opened_record = true;
    return 0;
}

int Oss_Sound_Device::close_playback()
{
    if( !opened_playback || fd_playback == -1 )
    {
#ifdef DEBUG_OUTPUT
        cerr << "WARNING: doing close on already "
                "closed sound card"<< endl;
#endif
        return -1;
    }

    ::close( fd_playback );
    fd_playback = -1;
    openedPlayback = false;
    return 0;
}

int Oss_Sound_Device::close_record()
{
#ifdef DEBUG_OUTPUT
    cerr << "OSS: Closing sound card for recording" << endl;
#endif
    if( !opened_record || fd_record == -1 )
    {
#ifdef DEBUG_OUTPUT
        cerr << "WARNING: doing close on already "
                "closed sound card"<< endl;
#endif
        return -1;
    }

    ::close( fd_record );
    fd_record = -1;
    opened_record = false;
    return 0;
}

void Oss_Sound_Device::sync()
{
    bool interrupted = false;
    do{
        interrupted = false;
        if( ioctl( fd_playback, SNDCTL_DSP_SYNC ) == -1 )
        {
#ifdef DEBUG_OUTPUT
            my_error( "ioctl sync error on soundcard" );
#endif
            interrupted = true;
        }
    }while( interrupted && errno==EINTR );
}
