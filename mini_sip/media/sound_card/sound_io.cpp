#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>

#include<iostream>
using namespace std;

#include "sound_io.h"

#include "my_assert.h"
#include "thread.h"
#include "my_time.h"

#include "audio_defines.h"
#include "sound_source.h"
#include "sound_device.h"

#include "audio_mixer_spatial.h"
#include "audio_mixer_simple.h"
#include "sp_audio.h"

#ifdef AEC_SUPPORT
#	include  "aec.h"
#endif

#ifdef _MSC_VER

#else
#	include<sys/time.h>
#	include<unistd.h>
#endif

#define BS 160

Recorder_Receiver::Recorder_Receiver(Sound_Recorder_Callback *cb, bool stereo)
    : callback(cb),
      stereo(stereo)
{
}

bool Recorder_Receiver::get_stereo()
{
    return stereo;
}

Sound_Recorder_Callback *Recorder_Receiver::get_callback()
{
    return callback;
}

Sound_IO::Sound_IO(SRef<Sound_Device *> inputDevice, SRef<Sound_Device *> outputDevice, std::string mixerType,
                   int nChannels, int32_t samplingRate, int format)
    : nchannels(nChannels),
      sampling_rate(samplingRate),
      format(format),
      recording(false),
      source_list_cond(new Cond_Var),
      recorder_cond(new Cond_Var)
{
    sound_dev_in = inputDevice;
    sound_dev_out = outputDevice;

    set_mixer( mixerType );

    start_sound_player();
    start_recorder();
}

Sound_IO::~Sound_IO()
{
    while( !recorder_callbacks.empty() )
    {
        delete recorder_callbacks.back();
        recorder_callbacks.pop_back();
    }
    delete source_list_cond;
    delete recorder_cond;
}

void Sound_IO::close_playback()
{
    if( sound_dev_out )
    {
        sound_dev_out->lock_write();
        sound_dev_out->close_playback();
        sound_dev_out->unlock_write();
    }
}

void Sound_IO::close_record()
{
    if( sound_dev_in )
    {
        sound_dev_in->lock_read();
        sound_dev_in->close_record();
        sound_dev_in->unlock_read();
    }
}

void Sound_IO::sync()
{
    if( sound_dev_out )
        sound_dev_out->sync();

    if( sound_dev_in )
        sound_dev_in->sync();
}

void Sound_IO::play_testtone(int secs)
{
    int nSamples = secs * sound_dev_out->get_sampling_rate();
    short *data = (short*)malloc( nSamples * sound_dev_out->get_sample_size() * sound_dev_out->get_nchannels_play() );
    for (int32_t i=0; i< nSamples; i++)
    {
        if (i%4==0)data[i]=0;
        if (i%4==1)data[i]=10000;
        if (i%4==2)data[i]=0;
        if (i%4==3)data[i]=-10000;
    }
    send_to_card( data, nSamples );
}

void Sound_IO::open_playback()
{
    if( sound_dev_out )
    {
        sound_dev_out->lock_write();
        if( !sound_dev_out->is_opened_playback() )
        {
            sound_dev_out->open_playback( sampling_rate, nchannels, format );
        }
        sound_dev_out->unlock_write();
    }
}

void Sound_IO::open_record()
{
    if( sound_dev_in )
    {
        sound_dev_in->lock_read();
        if( !sound_dev_in->is_opened_record() )
        {
            sound_dev_in->open_record( sampling_rate, nchannels, format );
        }
        sound_dev_in->unlock_read();
    }
}

void Sound_IO::start_record()
{
    recording = true;
    recorder_cond->broadcast();
}

void Sound_IO::stop_record()
{
    recording = false;
}

void Sound_IO::register_recorder_receiver(Sound_Recorder_Callback *callback, int32_t nrsamples, bool stereo)
{
    recorder_callbacks.push_back(new Recorder_Receiver(callback, stereo));
    recorder_buffer_size = nrsamples;
}

void Sound_IO::unregister_recorder_receiver( Sound_Recorder_Callback *callback )
{
    std::list<Recorder_Receiver *>::iterator iter;
    for( iter = recorder_callbacks.begin(); iter != recorder_callbacks.end(); iter++ )
    {
        if( (*iter)->get_callback() == callback )
        {
            recorder_callbacks.erase( iter );
            return;
        }
    }
}

void Sound_IO::set_recorder_buffer_size(int32_t bs)
{
    recorder_buffer_size=bs;
}

void Sound_IO::file_io_timeout(int)
{
}

void *Sound_IO::recorder_loop(void *sc_arg)
{
#ifdef DEBUG_OUTPUT
    set_thread_name("Sound_IO::recorderLoop");
#endif
    Sound_IO *soundcard = (Sound_IO *)sc_arg;
    int32_t i;
    short *buffers[2];	// Two sound buffers for "double buffering"
    my_assert(soundcard!=NULL);
    int32_t nread=0; /* IN SAMPLES */
    //FIXME
    soundcard->recorder_buffer_size = SOUND_CARD_FREQ*20/1000;
    // 	printf( "Sound_IO::recLoop - recorder_buff_size = %d\n", soundcard->recorder_buffer_size );

    for (i=0; i<2; i++)
    {
        //buffers[i] = (short *)malloc(soundcard->recorder_buffer_size*sizeof(short)*2);
        buffers[i] = (short *)malloc(4096);
        memset(buffers[i],0,4096);
    }

    short * tempBuffer=NULL;
#ifdef AEC_SUPPORT
    short * tempBufferR=NULL;		//hanning
#endif
    bool tempBufferAllocated = false;

    while( true )
    {
        if( ! soundcard->recording )
        {
            if( soundcard->sound_dev_in->is_opened_record() )
            {
                soundcard->close_record();
                if( tempBufferAllocated )
                {
                    delete [] tempBuffer;
#ifdef AEC_SUPPORT
                    delete [] tempBufferR;		//hanning
#endif
                }
                tempBuffer = NULL;
            }
            bool condVarDeleted;

            /* sleep until a recorder call back is added */
            Cond_Var::wait(*soundcard->recorder_cond, condVarDeleted);
            if(condVarDeleted)
                return NULL;

            if( ! soundcard->sound_dev_in->is_opened_record() )
            {
                soundcard->open_record();
                //printf( "Sound_IO::recLoop: openrecord channels = %d\n", soundcard->sound_dev_in->get_nchannels_record() );
            }
        }

        soundcard->sound_dev_in->lock_read();
        if( soundcard->sound_dev_in->is_opened_record() )
        {
            //soundcard->recorder_buffer_size is, for now, fixed to 960
            //		(SNDCARD_FREQ * 20 / 1000 )
            nread = soundcard->sound_dev_in->read( (byte_t *)buffers[i%2], soundcard->recorder_buffer_size);
        }

        soundcard->sound_dev_in->unlock_read();

        if( nread < 0 )
            continue;

        if( soundcard->sound_dev_in->get_nchannels_record() > 1 )
        {
            if( !tempBuffer )
            {
                tempBuffer = new short[soundcard->recorder_buffer_size];
#ifdef AEC_SUPPORT
                tempBufferR = new short[soundcard->recorder_buffer_size];	//hanning
#endif
                tempBufferAllocated = true;
            }

            for( int j = 0; j < soundcard->recorder_buffer_size; j++ )
            {
                tempBuffer[j] = buffers[i%2][j * soundcard->sound_dev_in->get_nchannels_record() ];
#ifdef AEC_SUPPORT
                tempBufferR[j] = buffers[i%2][j * soundcard->sound_dev_in->get_nchannels_record() + 1];	//hanning
#endif
            }
        }
        else
            tempBuffer = buffers[i%2];

        if (nread /*!=*/ < soundcard->recorder_buffer_size)
        {
#ifdef DEBUG_OUTPUT
            if (nread>0)
                cerr << "WARNING: dropping "  << nread <<" samples in partial buffer"<< std::endl;
#endif
        }
        else
        {
            //AudioMedia iimplements the callback ...
            for (list<Recorder_Receiver *>::iterator cb=soundcard->recorder_callbacks.begin();
                 cb!= soundcard->recorder_callbacks.end(); cb++)
            {
                if ((*cb)!=NULL && (*cb)->get_callback()!=NULL){
#ifdef AEC_SUPPORT
                    (*cb)->get_callback()->srcb_handle_sound( tempBuffer, soundcard->recorder_buffer_size, tempBufferR); //hanning
#else
                    (*cb)->get_callback()->srcb_handle_sound( tempBuffer, soundcard->recorder_buffer_size, soundcard->sampling_rate);
#endif
                }else{
                    cerr << "Ignoring null callback"<< endl;
                }
            }
            i++;
        }
    }
    for (i=0; i<2; i++)
    {
        free(buffers[i]);
    }
    return NULL;
}

void Sound_IO::start_recorder()
{
    Thread::create_thread(recorder_loop, (void*)this, Thread::Above_Normal_Priority);
}

void Sound_IO::register_source(SRef<Sound_Source *> source)
{
#ifdef DEBUG_OUTPUT
    cerr << "Sound_IO::register_source - Calling register source on created source " << source->get_id() << endl;
#endif
    //int32_t j=1;
    //int32_t nextSize=sources.size()+1;
    queue_lock.lock();
    for (list<SRef<Sound_Source *> >::iterator i=sources.begin(); i!= sources.end(); i++)
    {
        if (source->get_id()==(*i)->get_id())
        {
            source_list_cond->broadcast();
            queue_lock.unlock();
            return;
        }
        //(*i)->setPos(spAudio.assignPos(j,nextSize));
        //(*i)->initLookup(nextSize);
        //j++;
    }
    //source->setPos( spAudio.assignPos(j,nextSize) );
    //sources.push_front(source);
    sources.push_back(source);
    mixer->set_sources_position( sources, true ); //added sources

    source_list_cond->broadcast();
    queue_lock.unlock();
}

void Sound_IO::unregister_source(int sourceId)
{
#ifdef DEBUG_OUTPUT
    cerr << "Sound_IO::unregisterSource - Calling unregister source on source " << sourceId << endl;
#endif
    queue_lock.lock();
    std::list<SRef<Sound_Source *> >::iterator i;
    for (i = sources.begin(); i != sources.end(); i++)
    {
        if ((*i)->get_id() == sourceId)
        {
            sources.erase(i);
            break;
        }
    }
    mixer->set_sources_position( sources, false );//removed sources
    queue_lock.unlock();
}

void Sound_IO::send_to_card(short *buf, int32_t n_samples)
{
    byte_t *ptr = (byte_t *)buf;
    int32_t nWritten;

    sound_dev_out->lock_write();
    if( sound_dev_out->is_opened_playback() )
    {
        nWritten = sound_dev_out->write( ptr, n_samples );
    }
    sound_dev_out->unlock_write();
}

SRef<Sound_Source *> Sound_IO::get_sound_source(int32_t id)
{
    for (std::list<SRef<Sound_Source *> >::iterator i = sources.begin(); i != sources.end(); i++)
    {
        if ((*i)->get_id() == id)
            return *i;
    }
    return NULL;
}

void *Sound_IO::player_loop(void *arg)
{
#ifdef DEBUG_OUTPUT
    set_thread_name("Sound_IO::playerLoop");
#endif
    Sound_IO *soundcard = (Sound_IO *)arg;

    short *outbuf = NULL;
    uint32_t nChannels = 0;

    if( soundcard->get_mixer().is_null() )
    {
        cerr << "Error: Sound I/O ... mixer is null ... stopping the thread!!!" << endl;
        return NULL;
    }

#ifdef DEBUG_OUTPUT
    //uint32_t counter = 0;
#endif
    while( true )
    {
        soundcard->queue_lock.lock();
        if( soundcard->sources.size() == 0 )
        {
            if( soundcard->sound_dev_out->is_opened_playback() )
            {
                soundcard->close_playback();
            }
            Mutex *queueLock = &soundcard->queue_lock;
            bool condVarDeleted;

            /* Wait for someone to add a source */
            Cond_Var::wait(*soundcard->source_list_cond, *queueLock, condVarDeleted);
            if(condVarDeleted)
            {
                queueLock->unlock();
                return NULL;
            }

            soundcard->open_playback();
            nChannels = soundcard->sound_dev_out->get_nchannels_play();
            soundcard->mixer->init(nChannels);
        }
        outbuf = soundcard->mixer->mix( soundcard->sources );

        soundcard->queue_lock.unlock();

        if( soundcard->sound_dev_out->is_opened_playback() )
        {
            soundcard->send_to_card(outbuf, soundcard->mixer->get_frame_size());
        }

    }
    return NULL;
}

void Sound_IO::start_sound_player()
{
    Thread::create_thread(player_loop, (void*)this, Thread::Above_Normal_Priority);
}

SRef<Audio_Mixer *> Sound_IO::get_mixer()
{
    return mixer;
}

bool Sound_IO::set_mixer(  std::string type )
{
    bool ret = true;
    if( type == "simple" )
    {
#ifdef DEBUG_OUTPUT
        cout << "Sound I/O: using Simple Mixer" << endl;
#endif
        mixer = new Audio_Mixer_Simple();
    }
    else if( type == "spatial" )
    {
#ifdef DEBUG_OUTPUT
        cout << "Sound I/O: using Spatial Audio Mixer" << endl;
#endif
        SRef<Sp_Audio *> spatial = new Sp_Audio( 5 );
        spatial->init();
        mixer = new Audio_Mixer_Spatial(spatial);
    }
    else
    {
        cerr << "ERROR: Sound_IO could not create requested mixer! (type _" << type << "_ not understood)" << endl;
        cerr << "ERROR:      Creating Spatial Audio mixer instead." << endl;
        set_mixer( "spatial" );
        ret = false;
    }
    return ret;
}

