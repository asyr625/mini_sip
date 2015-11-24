#include "file_sound_device.h"

#include "string_utils.h"
#include "my_time.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

#ifdef WIN32
#include <winsock2.h>
#else
#include <time.h>
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
#	define USE_WIN32_API
#endif

int filesleep( unsigned long usec )
{
#ifdef WIN32
#include<winsock2.h>
    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = (long)usec;

    return select (0, NULL, NULL, NULL, &tv);
#else
    struct timespec request;
    request.tv_sec = 0;
    request.tv_nsec = (long) usec * 1000;

    return nanosleep( &request, NULL );
#endif
}

File_Sound_Device::File_Sound_Device(std::string in_file, std::string out_file, int32_t filetype )
    : Sound_Device("!notused_filesounddevice!"),
      file_type(filetype),
      in_filename(in_file),
      out_filename(out_file)
{
    this->in_fd = -1;
    this->out_fd= -1;

    format = -1;
    opened_playback = false;
    opened_record = false;

    nchannels_play = 0;
    nchannels_record = 0;
    sampling_rate = 0;
    sample_size = 0;

    file_sound_block_sleep = 20; //default value
    last_time_read = 0;
    last_time_write = 0;

    is_first_time_open_write = true;
    this->loop_record = true;
}

int File_Sound_Device::read( byte_t * buffer, uint32_t nSamples )
{
    if (last_time_read == 0)
    {
        last_time_read = my_time();
    }

    //loop if needed
    if( loop_record )
    {
        int currPos;
        int fileSize;
        //Check if we are at the end of the file ...
        currPos = lseek( in_fd, 0, SEEK_CUR );
        fileSize = get_file_size( in_fd );

        if( currPos == -1 )
        {
            print_error("read-loop");
            return -1;
        }
        //Check not for the exact end of file, but for when there are not enough
        //samples to read ... skip the few left and loop ...
        if( (fileSize - currPos ) < ((int)nSamples * get_sample_size() * get_nchannels_record() ) )
        {
            if( currPos == -1 )
            {
                print_error("read-loop2");
                return -1;
            }
            currPos = lseek( in_fd, 0, SEEK_SET );
            if( currPos == -1 )
            {
                print_error("read-loop3");
                return -1;
            }
        }
    }
    Sound_Device::read( buffer, nSamples );

    if( file_sound_block_sleep != 0 )
        read_sleep();

    return nSamples;
}

int File_Sound_Device::read_from_device( byte_t * buffer, uint32_t nSamples )
{
    int retValue = -1;

    //select the appropriate way to write to the file ...
    switch( file_type )
    {
    case FILESOUND_TYPE_RAW:
#ifdef _WIN32_WCE
        retValue = ::_read(in_fd, buf, nSamples * get_sample_size() * get_nchannels_record() );
#else
        retValue = ::read(in_fd, buf, nSamples * get_sample_size() * get_nchannels_record() );
#endif

        if( retValue == -1 )
        {
            retValue = -errno;
            print_error( "readFromDevice" );
        } else {
            retValue = retValue / ( get_sample_size() * get_nchannels_record() );
        }
        break;
    case FILESOUND_TYPE_WAV:
    case FILESOUND_TYPE_MP3:
        cerr << "File_Sound_Device::read_from_device - filetype not implemented" << endl;
        break;
    default:
        cerr << "File_Sound_Device::read_from_device - filetype unknown" << endl;
    }
    return retValue;
}

int File_Sound_Device::write( byte_t * buffer, uint32_t nSamples )
{
    if (last_time_write == 0)
    {
        last_time_write = my_time();
    }

    Sound_Device::write( buffer, nSamples );

    //if SoundDevice::sleepTime is >0, then the device has been opened for
    //playback in non-blocking mode, thus SoundDevice::write will do the sleeping between
    //calls (no need for FileSoundDevice::write to block for fileSoundBlockSleep miliseconds).
    if( file_sound_block_sleep > 0  && sleepTime == 0)
        write_sleep();

    return nSamples;
}

int File_Sound_Device::write_to_device( byte_t * buffer, uint32_t nSamples )
{
    int retValue = -1;

    //select the appropriate way to write to the file ...
    switch( file_type )
    {
    case FILESOUND_TYPE_RAW:
        //write n samples to the file ...
#ifdef _WIN32_WCE
        retValue = ::_write(out_fd, buf, nSamples * get_sample_size() * get_nchannels_play() );
#else
        retValue = ::write(out_fd, buf, nSamples * get_sample_size() * get_nchannels_play() );
#endif
        if( retValue == -1 )
        {
            retValue = -errno;
            print_error( "write" );
        } else {
            retValue = retValue / ( get_sample_size() * get_nchannels_play() );
        }
        break;
    case FILESOUND_TYPE_WAV:
    case FILESOUND_TYPE_MP3:
        cerr << "FileSoundDevice::write - filetype not implemented" << endl;
        break;
    default:
        cerr << "FileSoundDevice::write - filetype unknown" << endl;
    }
    return retValue;
}

int File_Sound_Device::read_error( int errcode, byte_t * buffer, uint32_t nSamples )
{
    bool mustReturn = true;
    switch( errcode )
    {
    case -EAGAIN:

#ifndef _WIN32_WCE
    case -EINTR:
#else
    case WSAEINTR:
#endif
        mustReturn = false;
        break;
    default:
        mustReturn = true;
        break;
    }
    if( mustReturn ) { return -1; }
    else { return 0; }
}

int File_Sound_Device::write_error( int errcode, byte_t * buffer, uint32_t nSamples )
{
    bool mustReturn = true;
    switch( errcode )
    {
    case -EAGAIN:
#ifndef _WIN32_WCE
    case -EINTR:
#else
    case WSAEINTR:
#endif
        mustReturn = false;
        break;
    default:
        mustReturn = true;
        break;
    }
    if( mustReturn ) { return -1; }
    else { return 0; }
}

int File_Sound_Device::open_playback( int32_t samplerate_, int nChannels_, int format )
{
    if( format!= -1 && format_!=format )
    {
#ifdef DEBUG_OUTPUT
        cerr << "FileSoundDevice::openRecord - trying to modify the format!" << endl;
#endif
        exit(-1);
    }
    else
    {
        set_format( format_ );
        set_audio_params( samplerate_, nChannels_ );
    }

#ifdef DEBUG_OUTPUT
    printf( "FSD:playback - samplerate = %d, nChannels = %d, sampleSize = %d\n", sampling_rate, nchannels_play, sample_size );
#endif

    int openFlags;
    //if it is the first time we open for writing, create and truncate to zero size
    //otherwise, append to the file
    if( is_first_time_open_write )
    {
#ifdef _MSC_VER
        openFlags = _O_WRONLY |  _O_CREAT | _O_TRUNC;
#else
        openFlags = O_WRONLY |  O_CREAT | O_TRUNC;
#endif
        is_first_time_open_write = false;
    } else {
#ifdef _MSC_VER
        openFlags = _O_WRONLY |  _O_APPEND;
#else
        openFlags = O_WRONLY |  O_APPEND;
#endif
    }


#ifdef _MSC_VER
    out_fd =::_open(out_filename.c_str(), openFlags, _S_IREAD | _S_IWRITE );
#else
    out_fd =::open( out_filename.c_str(), openFlags, S_IWUSR |  S_IRUSR);
#endif

    if (out_fd==-1)
    {
        print_error("openPlayback");
        exit(-1); //FIX: handle nicer - exception
    }
    opened_playback = true;
    return 0;
}

int File_Sound_Device::open_record( int32_t samplerate_, int nChannels_, int format )
{
    if( format!= -1 && format_!=format )
    {
#ifdef DEBUG_OUTPUT
        cerr << "FileSoundDevice::openRecord - trying to modify the format!" << endl;
#endif
        exit(-1);
    } else {
        set_format( format_ );
        set_audio_params( samplerate_, nChannels_ );
    }

#ifdef DEBUG_OUTPUT
    printf( "FileSoundDev:record - samplerate = %d, nChannels = %d, sampleSize = %d\n", sampling_rate, nchannels_record, sample_size );
#endif

#ifndef _WIN32_WCE
    in_fd=::open(in_filename.c_str(), O_RDONLY);
#else
    //win32 wcecompat only takes 3 params ... open the file with mode = permission for everybody ...
    in_fd=::_open(in_filename.c_str(), O_RDONLY, S_IREAD | S_IWRITE | S_IEXEC);
#endif
    if (in_fd==-1)
    {
        print_error("openRecord");
        exit(-1); //FIX: handle nicer - exception
    }
    opened_record = true;
    return 0;
}

int File_Sound_Device::close_playback()
{
    int ret;
    opened_playback = false;
#ifdef _WIN32_WCE
    ret = ::_close(out_fd);
#else
    ret = ::close(out_fd);
#endif
    if( ret == -1 )
    {
        print_error("openPlayback");
    }
// 	lastTimeWrite = 0;
    return ret;
}

int File_Sound_Device::close_record()
{
    int ret;
    opened_record = false;
#ifdef _WIN32_WCE
    ret = ::_close(in_fd);
#else
    ret = ::close(in_fd);
#endif
    if( ret == -1 ) {
        print_error("openRecord");
    }
    last_time_read = 0;
    return ret;
}

void File_Sound_Device::sync()
{
    cerr << "ERROR: sync unimplemented for file sound device"<< endl;
}

void File_Sound_Device::set_audio_params( int samplingRate_, int nChannels_ )
{
    if( file_type == FILESOUND_TYPE_RAW )
    {
        if( samplingRate_ > 0 ) sampling_rate = samplingRate_;
        if( nChannels_ > 0 )
        {
            this->nchannels_record = nChannels_;
            this->nchannels_play = nChannels_;
        }
    } else {
        cerr << "File_Sound_Device: filetype not understood" << endl;
    }
}

void File_Sound_Device::print_error( std::string func )
{
    string errStr;
    errStr = "FileSoundDevice::" + func + " - errno = ";
    //cut the function in wince ... most of these are not defined ... not worth the trouble
#ifndef _WIN32_WCE
    switch( errno )
    {
    case EACCES: errStr + "eaccess"; break;
    case EEXIST: errStr + "eexist"; break;
    case EFAULT: errStr + "efault"; break;
    case EISDIR: errStr + "eisdir"; break;
#ifndef USE_WIN32_API
    case ELOOP: errStr + "eloop"; break;
#endif
    case EMFILE: errStr + "emfile"; break;
    case ENAMETOOLONG: errStr + "toolong"; break;
    case ENFILE: errStr + "enfile"; break;
    case ENODEV: errStr + "enodev"; break;
    case ENOENT: errStr + "enoent"; break;
    case ENOMEM: errStr + "enomem"; break;
    case ENOSPC: errStr + "enospc"; break;
    case ENOTDIR: errStr + "enotdir"; break;
    case ENXIO: errStr + "enxio"; break;
#ifndef USE_WIN32_API
    case EOVERFLOW: errStr + "eoverflow"; break;
#endif
    case EROFS: errStr + "erofs"; break;
#ifndef USE_WIN32_API
    case ETXTBSY: errStr + "etxtbsy"; break;
#endif
    default: errStr + "unknown";
    }
#endif
    cerr << errStr << " (check man page for explanation)" << endl;
}

void File_Sound_Device::write_sleep()
{
    uint64_t currentTime;

    currentTime = my_time();

    //the sleep thingy is deactivated if sleeptime < 0
    // (the time in the computer should not go backward, right?!
    // 	printf("W: %d ", currentTime - lastTimeWrite );
    while (currentTime - last_time_write < file_sound_block_sleep )
    {
        int32_t ret;
        int32_t sleep = (int32_t) (file_sound_block_sleep - (currentTime-last_time_write));
        if( sleep < 0 ) sleep = 0;
        // 		printf(" [%d] ", sleep);
        ret = filesleep( sleep * 1000);
        currentTime = my_time();
        // 		printf(" %d ", currentTime - lastTimeWrite);
    }
    // 	printf("\n");

    //lastTimeWrite = currentTime;
    last_time_write += file_sound_block_sleep;
}

void File_Sound_Device::read_sleep()
{
    uint64_t currentTime;

    currentTime = my_time();

    //the sleep thingy is deactivated if sleeptime < 0
    // (the time in the computer should not go backward, right?!
    // 	printf("R: %d ", currentTime - lastTimeRead );
    while (currentTime - last_time_read < file_sound_block_sleep)
    {
        int32_t ret;
        int32_t sleep = (int32_t ) (file_sound_block_sleep - (currentTime-last_time_read));
        if( sleep < 0 ) sleep = 0;
        //printf(" [%d] ", sleep);
        ret = filesleep( sleep * 1000);
        currentTime = my_time();
        //printf(" %d ", currentTime - lastTimeRead);
    }
    // 	printf("\n");

    last_time_read += file_sound_block_sleep;
}

int File_Sound_Device::get_file_size( int fd )
{
    int ret;
    int filesize;
    int currentPos;

    currentPos = lseek( fd, 0, SEEK_CUR );
    if( currentPos == -1 )
    {
#ifdef DEBUG_OUTPUT
        //printError("get_file_size (1)");
#endif
        return -1;
    }

    filesize = lseek( fd, 0, SEEK_END );
    if( filesize == -1 )
    {
#ifdef DEBUG_OUTPUT
        //printError("get_file_size(2)");
#endif
        return -1;
    }

    ret = lseek( fd, currentPos, SEEK_SET);
    if( ret == (currentPos-1) )
    {
#ifdef DEBUG_OUTPUT
        //printError("get_file_size(3)");
#endif
        return -1;
    }
    return filesize;
}
