#include "call_recorder.h"

#include "audio_media.h"
#include "file_sound_device.h"
#include "string_utils.h"
#include "mutex.h"
#include "my_time.h"
#include "circular_buffer.h"

#include "audio_defines.h"

#include<string.h>
using namespace std;

#define AUDIO_FRAME_DURATION_MS 20


Call_Recorder::Call_Recorder( SRef<Audio_Media *> aMedia, SRef<Session*> s, SRef<Rtp_Receiver *> rtpReceiver,
                              SRef<Ip_Provider *> /*ipProvider*/,
                              IRequest_Video_Keyframe *keyframeRequestCallback)
    : Realtime_Media_Stream_Receiver( "callrecorder", (Realtime_Media *)*aMedia, s, rtpReceiver, NULL, keyframeRequestCallback ),
      enabled_mic(false),
      enabled_ntwk(false),
      file_dev( NULL ),
      audio_media( aMedia)
{
    static int count = 0;
    count ++;

    flush_last_time = 0;

    set_allow_start( false );
    set_enabled_mic( false );
    set_enabled_network( false );

    //Init the circular buffers ... max delay of 5 "groups" of samples = 100ms
    mic_data = ntwk_data = NULL;
    mic_data = new Circular_Buffer( 160 * 5 );
    ntwk_data = new Circular_Buffer( 160 * 5 );

    set_filename( itoa(count), 0 );

    file_dev = new File_Sound_Device( "", get_filename(), FILESOUND_TYPE_RAW );
    file_dev->set_sleep_time( 0 );
    file_dev->set_loop_record(false );

    start(); //register to rtp receiver and start getting packets

    audio_media->get_sound_io()->register_recorder_receiver( this, SOUND_CARD_FREQ * AUDIO_FRAME_DURATION_MS / 1000, false );

#ifdef DEBUG_OUTPUT
    cerr << "Call_Recorder::created: " << get_debug_string() << endl;
#endif
}

Call_Recorder::~Call_Recorder()
{
#ifdef DEBUG_OUTPUT
    cerr << "CallRecorder Destroyed - " << get_filename() << endl;
#endif
    if( file_dev->is_opened_playback() )
    {
        if( file_dev->close_playback() == -1 )
        {
            cerr << "CallRecorder::destroy - ERROR closing file opened to record = " << filename << endl;
        } else {
            cerr << "CallRecorder::destroy - closing file opened to record = " << filename << endl;
        }
    }
    if (audio_media)
        audio_media->get_sound_io()->unregister_recorder_receiver( this );
    if( mic_data != NULL ) delete mic_data;
    if( ntwk_data != NULL ) delete ntwk_data;
}

void Call_Recorder::free()
{
    if (audio_media)
        audio_media->get_sound_io()->unregister_recorder_receiver( this );
    audio_media = NULL;

    //free inherited references
    rtp_receiver = NULL;
    rtp6_receiver = NULL;
}

void Call_Recorder::srcb_handle_sound(void *samplearr, int /*length*/)
{
    if( !allow_start ) { return; }
    if( !enabled_mic ) { return; }

    if( !audio_media || !audio_media->get_resampler() ) { return; }
    audio_media->get_resampler()->resample( (short *)samplearr, resampled_data );

    add_mic_data( (void *)resampled_data, 160 );
    flush();
}

#ifdef AEC_SUPPORT
void Call_Recorder::srcb_handle_sound(void *samplearr, void *samplearrR)
{
    printf( "Call_Recorder:: srcb_handle_sound Function not implemented!!!\n" );
    if( ! enabled_mic ) { return; }
}
#endif


void Call_Recorder::handle_rtp_packet( SRef<SRtp_Packet *> packet, SRef<IPAddress *> from )
{

}

void Call_Recorder::set_filename( std::string name, int ssrc )
{
    filename = "minisip.callrecord." + name + "." + itoa( ssrc ) + ".8khz.16bit.signed.raw.sw";
#ifdef DEBUG_OUTPUT
    cerr << "CallRecorder::setFilename - " << filename << endl;;
#endif
}

void Call_Recorder::add_mic_data( void * data, int nSamples )
{
    bool ret;

    mic_mutex.lock();

    if( mic_data->get_free() < nSamples )
    {
        mic_mutex.unlock();
        return;
    }
    ret = mic_data->write( (short *)data, nSamples );
    if( !ret ) {
#ifdef DEBUG_OUTPUT
        printf("CR: Error in addMicData\n");
#endif
    }

    //printf("M3 - byteLength=%d - storedSamples=%d\n", length, micWrite-micData);
    mic_mutex.unlock();
}

void Call_Recorder::add_ntwk_data( void * data, int nSamples )
{
    bool ret;

    ntwk_mutex.lock();

    if( ntwk_data->get_free() < nSamples )
    {
        ntwk_mutex.unlock();
        return;
    }
    ret = ntwk_data->write( (short *)data, nSamples );
    if( !ret ) {
#ifdef DEBUG_OUTPUT
        printf("CR: Error in addMicData\n");
#endif
    }

    //printf("N3 - byteLength=%d - storedSamples=%d\n", length, ntwkWrite-ntwkData);
    ntwk_mutex.unlock();
}

void Call_Recorder::flush( )
{
    bool ret;

    file_dev_mutex.lock();
    mic_mutex.lock();
    ntwk_mutex.lock();

    if (flush_last_time == 0)
        flush_last_time = my_time();
    uint64_t currentTime = my_time();

    if(currentTime - flush_last_time < AUDIO_FRAME_DURATION_MS )
    {
        ntwk_mutex.unlock();
        mic_mutex.unlock();
        file_dev_mutex.unlock();
        return;
    }

    //make sure there is no left overs from previous executions
    memset( mixed_data, 0, 160 * 2 * sizeof(int) );
    //now we only use half this buffer ... we reading a mono stream
    memset( temp_flush, 0, 160 * 2 * sizeof(short) );

    ret = mic_data->read( temp_flush, 160 );
    if( ret )
    {
        for( int i=0; i < 160; i++ )
            mixed_data[i * 2] = temp_flush[i];

    } else {
// 		printf( "FLUSHMIC_UF\n" );
    }

    //now we only use half this buffer ... we reading a mono stream
    memset( temp_flush, 0, 160 * 2 * sizeof(short) );
    ret = ntwk_data->read( temp_flush, 160 );
    if( ret )
    {
        for( int i=0; i < 160; i++ )
            mixed_data[i * 2 + 1] = temp_flush[i];
    } else {
// 		printf( "FLUSHNTWK_UF\n" );
    }

    //normalize ... not really needed ... we ain't mixing
    //Here we are using all the tempFlush ... it is a stereo stream
    //	after mixing
    memset( temp_flush, 0, 160 * 2 * sizeof(short) );
    for( int j = 0; j < 160 * file_dev->get_nchannels_play(); j++ ) {
        if( mixed_data[j] > 32737 ) {
            temp_flush[j] = 32737;
        } else if( mixed_data[j] < -32737 ) {
            temp_flush[j] = -32737;
        } else {
            temp_flush[j] = (short)mixed_data[j];
        }
    }

    write( (void *)temp_flush, 160 );

    flush_last_time = my_time();

    ntwk_mutex.unlock();
    mic_mutex.unlock();
    file_dev_mutex.unlock();
}

int Call_Recorder::write( void * data, int nSamples )
{
    if( !file_dev )
        return 0;

    //if enabled, check if open ... if not, do it
    if( !file_dev->is_opened_playback() )
    {
        //don't care about the params ... the function does not use them
#ifdef DEBUG_OUTPUT
        cerr << "Call_Recorder: Start recording to file <"
             << get_filename() << ">" << endl;
#endif
        file_dev->open_playback( 8000,  //sampling rate ...
                               2 /* number of channels */
                               /*, SOUND_S16LE*/);
    }

    int writtenBytes;
    //the filedevice returns the number of samples written
    writtenBytes = file_dev->write( (byte_t *)data, nSamples );
    if( writtenBytes < nSamples  ) {
#ifdef DEBUG_OUTPUT
        cerr << "CallRecorder::write - not all written ("
             << itoa(writtenBytes) << "/"
             << itoa(nSamples) << ")" << endl;
#endif
    }
    // 	printf("write:: %d/%d\n", writtenBytes, nSamples);
    return 0;
}

std::string Call_Recorder::get_debug_string()
{
    std::string ret;
#ifdef DEBUG_OUTPUT
    ret=  "Call_Recorder: filename=" + get_filename();
    ret+= (enabled_mic ? "; MIC enabled":"; MIC disabled");
    ret+= (enabled_ntwk ? "; NTWK enabled":"; NTWK disabled");
    ret+= (allow_start ? "; START allowed":"; START not allowed");
#endif
    return ret;
}
