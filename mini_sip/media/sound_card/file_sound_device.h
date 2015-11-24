#ifndef FILE_SOUND_DEVICE_H
#define FILE_SOUND_DEVICE_H

#include<stdio.h>
#include <fcntl.h>
#include<iostream>

#ifdef _MSC_VER
#	include<io.h>
#	undef open
#	undef close
#	undef read
#	undef write
#else
#	include<sys/time.h>
#	include<unistd.h>
#endif

#include "sound_device.h"


#define FILESOUND_TYPE_RAW 0
#define FILESOUND_TYPE_WAV 1
#define FILESOUND_TYPE_MP3 2

class File_Sound_Device : public Sound_Device
{
public:
    File_Sound_Device(std::string in_file="", std::string out_file="",
                    int32_t filetype=FILESOUND_TYPE_RAW );

    virtual int read( byte_t * buffer, uint32_t nSamples );
    virtual int read_from_device( byte_t * buffer, uint32_t nSamples );

    virtual int write( byte_t * buffer, uint32_t nSamples );
    virtual int write_to_device( byte_t * buffer, uint32_t nSamples );

    virtual int read_error( int errcode, byte_t * buffer, uint32_t nSamples );
    virtual int write_error( int errcode, byte_t * buffer, uint32_t nSamples );

    virtual int open_playback( int32_t samplingRate, int nChannels, int format=SOUND_S16LE );
    virtual int open_record( int32_t samplingRate, int nChannels, int format=SOUND_S16LE );

    virtual int close_playback();
    virtual int close_record();

    bool get_loop_record() { return loop_record; }
    void set_loop_record( bool loop ) { loop_record = loop; }

    virtual void sync();

    virtual std::string get_mem_object_type() const { return "FileSoundDevice";}

protected:
    void set_audio_params(int samplingRate_, int nChannels_ );

    int file_type;
    std::string in_filename;
    std::string out_filename;

    int in_fd;
    int out_fd;

    bool is_first_time_open_write;
    bool loop_record;

    void print_error( std::string func );

    uint32_t file_sound_block_sleep;

    void write_sleep();

    uint64_t last_time_write;
    void read_sleep();
    uint64_t last_time_read;

    int get_file_size( int fd );
    Mutex lock_open;
};

#endif // FILE_SOUND_DEVICE_H
