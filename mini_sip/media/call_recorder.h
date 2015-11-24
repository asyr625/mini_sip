#ifndef CALL_RECORDER_H
#define CALL_RECORDER_H

#include "media_stream.h"
#include "file_sound_device.h"
#include "sound_recorder_callback.h"

class Audio_Media;
class Ip_Provider;
class Circular_Buffer;

class Call_Recorder : public Realtime_Media_Stream_Receiver, public Sound_Recorder_Callback
{
public:
    Call_Recorder(  SRef<Audio_Media *> aMedia,
                    SRef<Session*> s,
                    SRef<Rtp_Receiver *> rtpReceiver,
                    SRef<Ip_Provider *> ipProvider,
                    IRequest_Video_Keyframe *keyframeRequestCallback);
    virtual ~Call_Recorder();

    void free();

    virtual std::string get_mem_object_type() const {return "CallRecorder";}
    virtual void srcb_handle_sound(void *samplearr, int length);
#ifdef AEC_SUPPORT
    virtual void srcb_handle_sound(void *samplearr, void *samplearrR);
#endif

    virtual void handle_rtp_packet( SRef<SRtp_Packet *> packet, SRef<IPAddress *> from );

    std::string get_filename() { return filename; }
    void set_filename( std::string name, int ssrc );

    bool is_enabled() { return enabled_mic && enabled_ntwk; }

    void set_enabled_mic( bool en )
    {
        enabled_mic = en;
#ifdef DEBUG_OUTPUT
        std::cerr << get_debug_string() << "[1]"  << std::endl;
#endif
    }

    void set_enabled_network( bool en )
    {
        enabled_ntwk = en;
#ifdef DEBUG_OUTPUT
        std::cerr << get_debug_string() << "[2]"  << std::endl;
#endif
    }

    void set_allow_start( bool allow )
    {
        allow_start = allow;
#ifdef DEBUG_OUTPUT
        std::cerr << get_debug_string() << "[3]" << std::endl;
#endif
        if( get_filename() != "" )
        {
            std::cerr << "CallRecorder: Stopped recording to file <"
                      << get_filename() << ">" << std::endl;
        }
    }

    void add_mic_data( void * data, int nSamples );
    void add_ntwk_data( void * data, int nSamples );

    void flush( );

    int write( void * data, int length );

    std::string get_debug_string();

protected:
    std::string filename;
    bool enabled_mic;
    bool enabled_ntwk;

    bool allow_start;
    SRef<File_Sound_Device *> file_dev;

    Mutex file_dev_mutex;
    SRef<Audio_Media *> audio_media;

    short codec_output[320];

    short resampled_data[160];

    Circular_Buffer * mic_data;
    Mutex mic_mutex;

    Circular_Buffer * ntwk_data;
    Mutex ntwk_mutex;

    short temp_flush[160*2];

    int mixed_data[160*2];

    uint64_t flush_last_time;
};

#endif // CALL_RECORDER_H
