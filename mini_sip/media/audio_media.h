#ifndef AUDIO_MEDIA_H
#define AUDIO_MEDIA_H

#include "realtime_media.h"
#include "streams_player.h"
#include "media_processor.h"
#include "sound_io.h"

#define AUDIOMEDIA_CODEC_MAXLEN 16384

class Audio_Media_Source;
class Silence_Sensor;
class Resampler;

class Audio_Media : public Realtime_Media, public Sound_Recorder_Callback,
        public Audio_Stream_Player_Factory, public Media_Pipeline_Output_Handler
{
public:
    Audio_Media(Mini_Sip*, SRef<Sound_IO *> soundIo, const std::list<SRef<Codec_Description *> > & codecList, Streams_Player *_streamsPlayer );

    virtual std::string get_mem_object_type() const { return "AudioMedia"; }

    virtual std::string get_sdp_media_type();

    uint8_t  get_codec_get_sdp_media_type(){return 255;} //xx

    virtual void play_data(const SRef<Rtp_Packet *> & packet );

    virtual void register_realtime_media_sender( SRef<Realtime_Media_Stream_Sender *> sender );

    virtual void unregister_realtime_media_sender( SRef<Realtime_Media_Stream_Sender *> sender );

    virtual void register_media_source( const SRef<Session*>& session, uint32_t ssrc,
                                      std::string callId, SRef<Realtime_Media_Stream_Receiver*> rtMSR );

    virtual void unregister_media_source( uint32_t ssrc );

    virtual void srcb_handle_sound(void *data, int nsamples, int samplerate );
    #ifdef AEC_SUPPORT
    virtual void srcb_handle_sound( void *samplearr, int length, void *samplearrR);	//hanning
    #endif

    virtual void handle_data(const SRef<Processing_Data*>& a);

    void start_ringing( std::string ringtoneFile );
    void stop_ringing();

    virtual std::string get_debug_string();

    SRef<Audio_Media_Source *> get_source( uint32_t ssrc );

    SRef<Resampler *> get_resampler() { return _resampler; }

    SRef<Sound_IO *> get_sound_io() { return _sound_io; }

    virtual std::list<Receiving_MSS_Reporter *> get_receiving_media_sources();

    virtual Stream_Player* create_audio_stream_player(IStream_To_Streams_Player *_owner, Audio_Media_Source *_sink,
                                                  const int &_rtpTimestampSamplingRate,
                                                  const unsigned int &bufferOverflowUnderflowModifier_us,
                                                  const unsigned int &synchronizationToleration_us,
                                                  const unsigned int &_intraWakePeriod_ms);
protected:
    SRef<Realtime_Media_Pipeline*> _processing_audio_device_sender;

    SRef<Resampler *> _resampler;
    Silence_Sensor * _silence_sensor;
    SRef<Sound_IO *> _sound_io;
    uint32_t _seq_no;
    byte_t _encoded[1600];
    short _resampled_data[1600];
    #ifdef AEC_SUPPORT
    short resampled_dataR[160];		//hanning
    static AEC aec;				//hanning
    #endif
    std::list< SRef<Audio_Codec_Description *> > _codecs;
    std::list< SRef<Audio_Media_Source *> > _sources;
    Streams_Player *_streams_player;
};

class Audio_Stream_Player;

class Audio_Media_Source : public Basic_Sound_Source, public Receiving_MSS_Reporter,
    public Media_Pipeline_Output_Handler
{
public:
    Audio_Media_Source( const SRef<Session*>& session, uint32_t ssrc, std::string callId, SRef<Media *> media,
                        Streams_Player *_streamsPlayer, SRef<Realtime_Media_Stream_Receiver*> rtMSR );
    virtual ~Audio_Media_Source();

    virtual void handle_data(const SRef<Processing_Data*>& data );

    void play_data( const SRef<Rtp_Packet *> & rtpPacket );
    uint32_t get_ssrc();

    SRef<Media *> get_media() { return _media; }

    SRef<Decoder_Instance*> find_decoder( uint8_t payloadType );

    virtual std::string get_decoder_description();
    virtual uint64_t get_number_of_received_packets();
    virtual uint64_t get_number_of_missing_packets();
    virtual float get_received_video_framerate_fps();

protected:
    SRef<Realtime_Media_Pipeline*> _processing_audio;
    std::list< SRef<Decoder_Instance*> > _codecs;
    SRef<Media *> _media;
    uint32_t _ssrc;
    Streams_Player *_streams_player;
    Audio_Stream_Player *_audio_stream_player;
    uint16_t _last_packet_seq_no;
    SRef<Realtime_Media_Stream_Receiver*> _realtime_stream;
};

#endif // AUDIO_MEDIA_H
