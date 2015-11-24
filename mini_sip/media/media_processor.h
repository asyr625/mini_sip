#ifndef MEDIA_PROCESSOR_H
#define MEDIA_PROCESSOR_H
#include <list>

#include "my_types.h"
#include "sobject.h"

#include "mutex.h"
#include "ssingleton.h"
#include "splugin.h"

#define PROCESSOR_INSERT_POINT_RTP 1
#define PROCESSOR_INSERT_POINT_STREAM 2
#define PROCESSOR_INSERT_POINT_DEVICE 4
std::string getProcessorInsertPointString(int ip);

#define PROCESSOR_DIRECTION_DEV2NET 1
#define PROCESSOR_DIRECTION_NET2DEV 2
std::string getProcessorDirectionString(int dir);

class Mini_Sip;
class Rtcp_Packet;
class SRtp_Packet;
class SImage;

class Processing_Data : public SObject
{
public:
    uint32_t _rtp_timestamp;
    std::list<SRef<Rtcp_Packet*> > _rtcp_packets_out; //RTCP packets that should be sent after the data has been processed
};


class Processing_Data_Rtp : public Processing_Data
{
public:
    Processing_Data_Rtp();
    Processing_Data_Rtp(const SRef<SRtp_Packet*>& p);

    std::list<SRef<SRtp_Packet*> > _rtp_packets;
    std::list<SRef<Rtcp_Packet*> > _rtcp_packets_in;  //RTCP packets received from the remote side
    uint32_t flags; //currently unused.
};


class Processing_Data_Audio : public Processing_Data
{
public:
    Processing_Data_Audio(bool deleteBuf);
    virtual ~Processing_Data_Audio();
    short *samples;
    int nsamples;
    int sfreq;
    int nchannels;
    int rtp_seq_no;
    int rtp_marker;
    int ssrc;
    bool delete_buf;
    //SRef<AudioFrame*> audioFrame; TODO:unimplemented
};

class AVFrame;

class Processing_Data_Video : public Processing_Data
{
public:
    SRef<SImage*> image;
    //	AVFrame* avimage;
    //SRef<VideoData*> videoFrame; TODO:unimplemented
};

class Media_Pipeline_Output_Handler
{
public:
    virtual void handle_data(const SRef<Processing_Data*>& data ) = 0;
};

class Media_Pipeline;
class Realtime_Media_Stream;
class Session;

class Realtime_Media_Processor : public virtual SObject
{
public:
    Realtime_Media_Processor( int priority, const SRef<Realtime_Media_Stream*>& strm );

    uint16_t get_priority() { return _priority; }

    virtual ~Realtime_Media_Processor();

    void set_next_processor(SRef<Realtime_Media_Processor*> p)
    {
        my_assert(p);
        _next = p;
    }

    virtual void initialize() = 0;
    virtual void uninitialize() = 0;

    virtual void start_processor() = 0;
    virtual void stop_processor() = 0;

    virtual void handle_data(const SRef<Processing_Data*>& data ) = 0;

    virtual std::string get_processor_debug_string();

    SRef<Session*> get_session() { return _session; }
protected:
    Mini_Sip* _minisip;
    SRef<Session*> _session;
    SRef<Realtime_Media_Stream*> _realtime_media_stream;
    SRef<Realtime_Media_Processor*> _next;

    int _dir;
    int _insert_point;
    std::string _media_type;

private:
    int _priority;
    void internal_uninitialize()
    {
        _next = NULL;
    }

    friend class Realtime_Media_Pipeline;
};

class Realtime_Media_Pipeline : public Realtime_Media_Processor
{
public:
    Realtime_Media_Pipeline( Mini_Sip* minisip, const SRef<Session*>& session, const SRef<Realtime_Media_Stream*>& stream,
                             Media_Pipeline_Output_Handler *handler, std::string mediaType, int direction, int insert_point );
    ~Realtime_Media_Pipeline();

    void start();
    void stop();

    void free();

    std::string get_processor_debug_string();
    std::string get_debug_string();

    void add_processor(SRef<Realtime_Media_Processor*> p);

    void process(const SRef<Processing_Data*>& data );

    virtual void handle_data(const SRef<Processing_Data*>& data ); //called by the last "real" processor has processed some data

    virtual void initialize(){}
    virtual void uninitialize(){}
    virtual void start_processor(){}
    virtual void stop_processor(){}

    int size() { int ret; _lock.lock(); ret = _processors.size(); _lock.unlock(); return ret; }
    void set_output_handler(Media_Pipeline_Output_Handler *h) { _handler = h; }

private:
    void initialize_start_all();

    Mini_Sip* _minisip;
    SRef<Session*> _session;
    std::string _media_type;
    int _dir;
    int _insert_point;

    Mutex _lock;
    std::list<SRef<Realtime_Media_Processor*> > _processors;
    Media_Pipeline_Output_Handler* _handler;
    bool _started;
    friend class Processor_Registry;
};


class Realtime_Media_Processor_Plugin : public SPlugin
{
public:
    virtual SRef<Realtime_Media_Processor*> new_instance( SRef<Realtime_Media_Stream*> strm ) = 0;
    virtual std::string get_sdp_media_type() = 0; //typically "rtp" (any kind of media stream),"audio" or "video"
    virtual int direction() = 0;
    virtual int get_insert_point() = 0;
protected:
    Realtime_Media_Processor_Plugin(SRef<Library*> lib);
};

class Processor_Registry : public SPlugin_Registry, public SSingleton<Processor_Registry>
{
public:
    virtual std::string get_plugin_type(){ return "Processor"; }

    void add_plugins(const SRef<Realtime_Media_Pipeline* >& pipeline, const SRef<Realtime_Media_Stream*>& strm);
protected:
    Processor_Registry();
private:
    friend class SSingleton<Processor_Registry>;
};

#endif // MEDIA_PROCESSOR_H
