#include "media_processor.h"

#include "media_stream.h"
#include "string_utils.h"
#include "dbg.h"

using namespace std;


std::string getProcessorDirectionString(int dir)
{
    string ret;
    if (dir & PROCESSOR_DIRECTION_DEV2NET)
        ret += "|PROCESSOR_DIRECTION_DEV2NET";
    if (dir & PROCESSOR_DIRECTION_NET2DEV)
        ret += "|PROCESSOR_DIRECTION_NET2DEV";
    if (ret.size() == 0)
        ret = "UNKNOWN_BUG?";
    return ret;
}

std::string getProcessorInsertPointString(int ip)
{
    string ret;
    if (ip & PROCESSOR_INSERT_POINT_DEVICE)
        ret += "PROCESSOR_INSERT_POINT_DEVICE";

    if (ip & PROCESSOR_INSERT_POINT_STREAM)
        ret += "PROCESSOR_INSERT_POINT_STREAM";

    if (ip & PROCESSOR_INSERT_POINT_RTP)
        ret += "PROCESSOR_INSERT_POINT_RTP";
    if (ret.size() == 0)
        ret = "UNKNOWN_BUG?";
    return ret;
}

Processing_Data_Rtp::Processing_Data_Rtp()
{
}

Processing_Data_Rtp::Processing_Data_Rtp(const SRef<SRtp_Packet*>& p)
{
    _rtp_packets.push_back(p);
}

Processing_Data_Audio::Processing_Data_Audio(bool deleteBuf)
    : delete_buf(deleteBuf)
{
    samples = NULL;
    nsamples = 0;
}

Processing_Data_Audio::~Processing_Data_Audio()
{
    if (delete_buf && samples)
        free(samples);
}


Realtime_Media_Processor::Realtime_Media_Processor( int priority, const SRef<Realtime_Media_Stream*>& strm )
    : _realtime_media_stream(strm), _priority(priority),_dir(0), _insert_point(0)
{
}

Realtime_Media_Processor::~Realtime_Media_Processor()
{
}

std::string Realtime_Media_Processor::get_processor_debug_string()
{
}


bool compare_priority(SRef<Realtime_Media_Processor*> p1, SRef<Realtime_Media_Processor*> p2)
{
    return p1->get_priority() < p2->get_priority();
}


Realtime_Media_Pipeline::Realtime_Media_Pipeline( Mini_Sip* minisip, const SRef<Session*>& sess, const SRef<Realtime_Media_Stream*>& stream,
                         Media_Pipeline_Output_Handler *handler, std::string mediaType, int direction, int insert_point )
    : Realtime_Media_Processor(0xFFFF, stream),
      _minisip(minisip),
      _session(sess),
      _media_type(mediaType),
      _dir(direction),
      _insert_point(insert_point),
      _handler(handler),
      _started(false)
{
    add_processor( this );
}

Realtime_Media_Pipeline::~Realtime_Media_Pipeline()
{
}

void Realtime_Media_Pipeline::start()
{
    std::list<SRef<Realtime_Media_Processor*> >::iterator i;
    _lock.lock();
    for (i = _processors.begin(); i != _processors.end(); i++)
    {
        (*i)->initialize();
    }
    for (i = _processors.begin(); i != _processors.end(); i++)
    {
        (*i)->start_processor();
    }
    _started = true;
    _lock.unlock();
}

void Realtime_Media_Pipeline::stop()
{
    _started = false;
    std::list<SRef<Realtime_Media_Processor*> >::iterator i;
    _lock.lock();
    for (i = _processors.begin(); i != _processors.end(); i++)
    {
        (*i)->stop_processor();
    }
    _lock.unlock();
}

void Realtime_Media_Pipeline::free()
{
    std::list<SRef<Realtime_Media_Processor*> >::iterator i;
    _lock.lock();
    for (i = _processors.begin(); i != _processors.end(); i++)
    {
        (*i)->uninitialize();
        (*i)->internal_uninitialize();
    }
    _lock.unlock();
}

void Realtime_Media_Pipeline::add_processor(SRef<Realtime_Media_Processor*> p)
{
    my_assert(p);
    my_assert(_minisip);
    p->_minisip = _minisip;
    p->_session = _session;
    p->_media_type = _media_type;
    p->_dir = _dir;
    p->_insert_point = _insert_point;

    _lock.lock();

    _processors.push_back( p );
    _processors.sort( compare_priority );
    SRef<Realtime_Media_Processor*> last;
    std::list<SRef<Realtime_Media_Processor*> >::iterator i;

    for (i = _processors.begin(); i != _processors.end(); i++)
    {
        if (last)
            last->set_next_processor( *i );
        last = *i;
    }
    last->set_next_processor(this);
    if (_started)
        p->start_processor();

    _lock.unlock();
}

void Realtime_Media_Pipeline::process(const SRef<Processing_Data*>& data )
{
    my_assert( _processors.size() > 0 );
//	if (_processors.size()==0)
//		_handler->handle_data(data);
    _lock.lock();
    SRef<Realtime_Media_Processor*> p = _processors.front();
    _lock.unlock();
    my_assert(p);
    p->handle_data(data);
}

void Realtime_Media_Pipeline::handle_data(const SRef<Processing_Data*>& data )
{
    if (_handler)
    {
        _handler->handle_data(data);
    }else{
    //my_err <<"WARNING: Realtime_Media_Pipeline: no handler - data dropped. Bug?"<<endl;
    }
}


std::string Realtime_Media_Pipeline::get_processor_debug_string()
{
    return "end of pipeline";
}

std::string Realtime_Media_Pipeline::get_debug_string()
{
    string ret;
    if (_processors.size()>0)
    {
        std::list<SRef<Realtime_Media_Processor*> >::iterator i;
        _lock.lock();
        for (i = _processors.begin(); i != _processors.end(); i++)
        {
            ret += "\n\t\tPlugin prio=" + itoa( (*i)->get_priority() ) + " "+(*i)->get_processor_debug_string();
        }
        _lock.unlock();
    }
    else
    {
        ret = "\n\t\tNo processors";
    }
    return ret;
}

void Realtime_Media_Pipeline::initialize_start_all()
{
    std::list<SRef<Realtime_Media_Processor*> >::iterator i;
    for (i = _processors.begin(); i != _processors.end(); i++)
    {
        (*i)->initialize();
    }
    for (i = _processors.begin(); i != _processors.end(); i++)
    {
        (*i)->start_processor();
    }
}


Realtime_Media_Processor_Plugin::Realtime_Media_Processor_Plugin(SRef<Library*> lib)
    : SPlugin(lib)
{
}


Processor_Registry::Processor_Registry()
{
}

static bool isSet(int set, int bit)
{
    return (set & bit) == bit;
}

void Processor_Registry::add_plugins(const SRef<Realtime_Media_Pipeline* >& pipeline, const SRef<Realtime_Media_Stream *> &strm)
{
    std::list<SRef<SPlugin *> >::iterator i;
    for (i = plugins.begin();i != plugins.end(); i++)
    {
        Realtime_Media_Processor_Plugin* p = dynamic_cast<Realtime_Media_Processor_Plugin*>(**i);
        my_assert(p);
//		int dir = p->direction();
        string type = p->get_sdp_media_type();
        int location= p->get_insert_point();

        if ( isSet(p->direction(), pipeline->_dir) && (type == "any" || pipeline->_media_type=="any" || type==pipeline->_media_type) && (isSet(p->get_insert_point(),pipeline->_insert_point)) )
        {
            SRef<Realtime_Media_Processor*> proc = p->new_instance( strm );
            pipeline->add_processor( proc );
        }
    }
    pipeline->initialize_start_all();
}
