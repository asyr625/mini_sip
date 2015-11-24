#include "sound_source.h"

#include<stdio.h>
#include<string.h>
#include<iostream>
using namespace std;

#include "sound_source.h"
#include "my_time.h"

#include "string_utils.h"

#include  "circular_buffer.h"

//jitter buffer size in units of 20ms
#define CIRCULAR_BUFFER_SIZE 10

Sound_Source::Sound_Source(int id, std::string cId)
    : source_id(id),callid(cId)
{
    leftch = NULL;
    rightch = NULL;
    set_silenced( false );
    position = 0;
    of_error_timestamp = uf_error_timestamp = 0;
    of_error_count = uf_error_count = 0;
}

Sound_Source::~Sound_Source()
{
}

int Sound_Source::get_id()
{
    return source_id;
}

int32_t Sound_Source::get_pos()
{
    return position;
}

void Sound_Source::set_pos(int32_t p)
{
    position =  p;
}

int32_t Sound_Source::get_pointer()
{
    return pointer;
}

void Sound_Source::set_pointer(int32_t wpointer)
{
    pointer = wpointer;
}



Basic_Sound_Source::Basic_Sound_Source(int32_t id, std::string callId, Sound_IO_PLC_Interface *plc,
                                       int32_t position, uint32_t oFreq, uint32_t oDurationMs, uint32_t oNChannels)
    : Sound_Source(id, callId),
      plc_provider(plc)
{
    this->onchannels = oNChannels;

    memset(plc_cache, 0, 2048*sizeof(short));
    this->ofreq = oFreq;

    this->position=position;

    oframes = ( oDurationMs * oFreq ) / 1000;
    //	iFrames = ( oDurationMs * 8000 ) / 1000;
    iframes = oframes;

    resampler = Resampler_Registry::get_instance()->create( 8000, oFreq, oDurationMs, oNChannels );

    temp = new short[iframes * oNChannels];
    memset(temp, 0, iframes * oNChannels * sizeof(short));

    //FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME
    //With this implementation, we keep losing audio ...
    //  For every 1000 round of getSound(), pushSound() only does 999,
    //  causing a delay for the audio on the headphones from which we never
    //  recover.
    //HACK Set a small buffer size ... kind of limiting the max delay.
    //	This limits the delay, but we loose audio frames all along ...
    //         For now, we do 20ms * 5 = 100ms
    //	We can set this even smaller ... but then we may have problems
    //	if rtp packets come in burst  ...
    cbuff = new Circular_Buffer( iframes * oNChannels * CIRCULAR_BUFFER_SIZE );

    /* spatial audio initialization */
    leftch = new short[1028];
    rightch = new short[1028];
    pointer = 0;
    j=0;
    k=0;
    //	lookupleft = new short[65536];
    //	lookupright = new short[65536];

#ifdef DEBUG_OUTPUT
    cerr << "Basic_Sound_Source::  - new with id(ssrc) = " << itoa(id) << endl;
    /*	printf( "Basic_Sound_Source:: buffer size = %d, bufferSizeMonoSamples = %d, oFrames = %d, iFrames = %d, oDurationMs = %d, oNChannels = %d\n",
        bufferSizeInMonoSamples*oNChannels, bufferSizeInMonoSamples, oframes, iframes, oDurationMs, this->onchannels );*/
#endif
}

Basic_Sound_Source::~Basic_Sound_Source()
{
    delete [] temp;
    temp = NULL;

    delete cbuff;
    cbuff = NULL;
    if (leftch)
        delete []leftch;
    leftch = NULL;
    if (rightch)
        delete []rightch;
    rightch = NULL;
}

#ifdef DEBUG_OUTPUT
bool nprint = false;
int npush = 1;
int nget = 1;
#endif

void Basic_Sound_Source::push_sound(short *samples, int32_t nMonoSamples, int32_t index, int samplerate, bool isStereo )
{
#ifdef DEBUG_OUTPUT
    npush++;
    if (npush%1000 == 0)
    {
        static uint64_t lastTimePush = 0;
        uint64_t currentPush;
        if( lastTimePush == 0 )
            lastTimePush = my_time();
        currentPush = my_time();
        //	printf( "pushSound: nget=%d; npush=%d; diff=%d, cbuff_size=%d, time_diff=%d\n",
        //			nget, npush, nget - npush,
        //			cbuff->get_size(), (int) (currentPush - lastTimePush) ) ;
        lastTimePush = currentPush;
        // 		printf( "CircBuff_push: maxSize = %d; size=%d; free=%d\n", cbuff->get_max_size(), cbuff->get_size(), cbuff->get_free() );
        //cerr << "Calling pushSound for source " << get_id() << endl;
    }
#endif

    buffer_lock.lock();
    //Check for OverFlow ... this happens if we receive big burst of packets ...
    //or we are not emptying the buffer quick enough ...
    if( cbuff->get_free() < nMonoSamples * (int)onchannels )
    {
#ifdef DEBUG_OUTPUT
        of_error_count++;
        uint64_t now = my_time();
        if (now - of_error_timestamp >= 60000)  // at maximum one report per source. This error might not be because
        {
            // of a problem. It is generated if the remote mutes its sending
            // or for some other reason stops sending (common when using i2conf)
            std::cerr <<"Sound buffer overflow for id="<<get_id()<<" repeated "<<of_error_count<<" times."<< std::endl;
            of_error_timestamp = now;
            of_error_count = 0;
        }
#endif
        //bufferLock.unlock();
        //return;
    }

    //If the incoming samples are already stereo, as is our circular buffer,
    //just copy them to the buffer.
    //Otherwise, transform them from mono to stereo (copy them twice ... ).
    int writeRet = false;
    if( isStereo )
    {

        //TODO: FIXME: handle case where sampleRate is > 8kHz
        resampler->resample( samples, temp);
        writeRet = cbuff->write( temp, nMonoSamples * 2, true );
    }
    else
    {
        int tempVal;

        for( int32_t nSamples = nMonoSamples; nSamples > 0; )
        {
            int32_t cur = nSamples;

            if( cur > (int32_t)iframes )
                cur = iframes;
            memset( temp, 0, iframes * onchannels );
            for( int32_t i = 0; i<cur; i++ )
            {
                tempVal = i*onchannels;
                temp[ tempVal ] = samples[i];
                tempVal ++;
                temp[ tempVal ] = samples[i];
            }

            if (samplerate == ofreq)
            {
                writeRet = cbuff->write( temp, cur * 2, true );
                samples += cur;
                nSamples -= cur;
            }
            else
            {
                short temp2[2048];
                resampler->resample( temp, temp2);
                writeRet = cbuff->write( temp2, cur * 2*2, true );
                samples += cur;
                nSamples -= cur;
            }
        }
    }

    buffer_lock.unlock();
#ifdef DEBUG_OUTPUT
    if( writeRet == false )
    {
        cerr << "Basic_Sound_Source::pushSound - Buffer write error"<<endl;
    }
#endif
}

void Basic_Sound_Source::get_sound(short *dest, bool dequeue )
{
#ifdef DEBUG_OUTPUT
    nget++;
    if (nget%1000==0)
    {
        static uint64_t lastTimeGet = 0;
        uint64_t currentGet;
        if( lastTimeGet == 0 )
            lastTimeGet = my_time();
        currentGet = my_time();
        printf( "get_sound: nget=%d; npush=%d; diff=%d, cbuff_size=%d, time_diff=%d\n",
                nget, npush, nget - npush,
                cbuff->get_size(), (int) (currentGet - lastTimeGet) ) ;
        lastTimeGet = currentGet;
        //printf( "CircBuff_get: maxSize = %d; size=%d; free=%d\n", cbuff->get_max_size(), cbuff->get_size(), cbuff->get_free() );
        //cerr << "nget="<< nget<< endl;
        //cerr << "Calling get_sound for source " << get_id() << endl;
    }
#endif

    buffer_lock.lock();

    //Check for underflow ...
    //	if it is so, use the PLC to fill in the missing audio, or produce silence
    //NOTE Underflow is not so bad ... for example, if using a peer like minisip, it will
    //	only send 1 packet/second when we are not the main call ... as long as we keep
    //	receiving 1 pack/set instead of 1pack/20ms ... we get underflow.
    if( cbuff->get_size() < (int) (iframes * onchannels) )
    {
#ifdef DEBUG_OUTPUT
        uf_error_count++;
        uint64_t now = my_time();
        if (now - uf_error_timestamp >= 1000)
        {
            //	cerr <<"Sound buffer underflow for id="<<get_id()<<" repeated "<<uf_error_count<<" times"<<endl;
            uf_error_timestamp = now;
            uf_error_count = 0;
        }
#endif
        if (plc_provider)
        {
#ifdef DEBUG_OUTPUT
            std::cerr << "PLC!"<< std::endl;
#endif
            short *b = plc_provider->get_plc_sound(oframes);
            memcpy(dest, b, oframes);
        }
        else
        {
            //	for (uint32_t i=0; i < oFrames * oNChannels; i++){
            //		dest[i]=0;
            //	}
            for (uint32_t i=0; i < oframes * onchannels; i++)
            {
                dest[i] = plc_cache[i];
                plc_cache[i] /= 2;
            }

        }
        buffer_lock.unlock();
        return;
    }

    //If there is no underflow, take the data from the circular buffer and
    //put it in the temp buffer, where it will be resampled
    memset( temp, 0, iframes * onchannels * sizeof( short ) );
    bool retRead;
    if( ! is_silenced() )
    {
        retRead = cbuff->read( temp, iframes*onchannels );
    } else {
        retRead = cbuff->remove( iframes*onchannels );
    }
#ifdef DEBUG_OUTPUT
    if( !retRead )
    {
        std::cerr << "Basic_Sound_Source::pushSound - Buffer read error" << std::endl;
    }
#endif
    //resampler->resample( temp, dest );
    memcpy(dest, temp, iframes * onchannels * sizeof( short ) );

    memcpy((void*)&plc_cache[0], dest, oframes*onchannels );

    //memset( dest, 0, oFrames * oNChannels * sizeof( short ) );
    buffer_lock.unlock();
}
