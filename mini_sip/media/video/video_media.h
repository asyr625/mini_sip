#ifndef VIDEO_MEDIA_H
#define VIDEO_MEDIA_H

#include "avdecoder.h"
#include "video_codec.h"

#include "video_display.h"
#include "grabber.h"
#include "codec.h"
#include "streams_player.h"
#include "media_stream.h"
#include "realtime_media.h"

class Video_Encoder_Callback;
class Video_Codec_Description;
class Video_Display;
class Grabber;
class Video_Media_Source;
class AVDecoderInstance;
class Rtp_Packet;

class Video_Stream_Player;
class IRequest_Video_Keyframe;

class Video_Media :public Realtime_Media, public Video_Stream_Player_Factory
{
public:
    Video_Media( Mini_Sip*, SRef<Sip_Configuration *> config, SRef<Codec_Description *> codec, bool isSecondMediaHack);

    virtual ~Video_Media();
    virtual void handle( const SRef<SImage *>& );

    virtual SRef<Encoder_Instance*> create_encoder_instance( uint8_t payloadType ); //overload default implementation to set default resolution, bandwidth etc.

    virtual std::string get_mem_object_type() const { return "VideoMedia"; }

    virtual std::string get_sdp_media_type();

    virtual void play_data( const SRef<Rtp_Packet *> & rtpPacket );

    virtual void register_realtime_media_sender( SRef<Realtime_Media_Stream_Sender *> sender );
    virtual void unregister_realtime_media_sender( SRef<Realtime_Media_Stream_Sender *> sender );

    virtual void register_media_source( const SRef<Session*>& session, uint32_t ssrc,
                                        std::string callId, SRef<Realtime_Media_Stream_Receiver*> rtMSR );

    virtual void unregister_media_source( uint32_t ssrc );
    virtual void handle_mheader( SRef<Sdp_HeaderM *> m );



    uint8_t  get_codec_get_sdp_media_type();

    SRef<Video_Media_Source *> get_source( uint32_t ssrc );

    virtual std::list<Receiving_MSS_Reporter *> get_receiving_media_sources();
    void keyframe_request_arrived();

    virtual Stream_Player* create_video_stream_player(IStream_To_Streams_Player *_owner, Image_Handler *_sink,
                                                  const int &_rtpTimestampSamplingRate,
                                                  const unsigned int &bufferOverflowUnderflowModifier_us,
                                                  const unsigned int &synchronizationToleration_us);

    void set_keyframe_request_callback(IRequest_Video_Keyframe *_keyframeRequestCallback);

private:
    SRef<Sip_Configuration *> config;
    SRef<Grabber *> grabber;

    SRef<Video_Codec_Description *> codec;
    std::list<SRef<Video_Media_Source *> > sources;
    Mutex sources_lock;

    IRequest_Video_Keyframe *keyframe_request_callback;
};


class Video_Media_Source : public Receiving_MSS_Reporter, public Media_Pipeline_Output_Handler, public SObject
{
public:
    Video_Media_Source( const SRef<Session*>& session, const std::string &_callID,
                        const uint32_t &ssrc, SRef<Realtime_Media_Stream_Receiver*> rtMSR );
    virtual ~Video_Media_Source();

    virtual void play_data( const SRef<Rtp_Packet *> & packet );

    virtual void frame_converter_output(const SRef<SImage*>& image);

    SRef<AVDecoder *> get_decoder();

    uint32_t ssrc;

    virtual std::string get_mem_object_type() const { return "VideoMediaSource"; }

    friend class Video_Media;
    virtual std::string get_decoder_description();

    virtual uint64_t get_number_of_received_packets();
    virtual uint64_t get_number_of_missing_packets();
    virtual float get_received_video_framerate_fps();

    void set_keyframe_request_callback(IRequest_Video_Keyframe *keyframeRequestCallback);

    void handle_data(const SRef<Processing_Data*>& data );

private:
    void add_packet_to_frame(const SRef<Rtp_Packet *> & packet, bool flush);

    SRef<Realtime_Media_Pipeline*> processing_video;
    SRef<Threaded_Frame_Converter *> frame_converter;


    SRef<AVDecoder *> decoder;
    SRef<Video_Display *> display;

    uint32_t nbytes_received;
    bool packet_loss;
    std::string call_id;
    int number_of_received_packets;
    int number_of_missing_packets;
};

#endif // VIDEO_MEDIA_H
