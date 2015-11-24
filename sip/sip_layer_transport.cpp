#include <string.h>
#include <iostream>

#include "sip_layer_transport.h"
#include "sip_command_dispatcher.h"
#include "sip_command_string.h"
#include "sip_transport.h"
#include "network_exception.h"
#include "string_utils.h"
#include "network_functions.h"
#include "sip_utils.h"
#include "my_types.h"
#include "my_time.h"

#include "sip_exception.h"
#include "dns_naptr_query.h"

#include "sip_header_via.h"
#include "sip_header_contact.h"
#include "sip_header_to.h"
#include "sip_header_from.h"

#define TIMEOUT 600000
#define NB_THREADS 5
#define BUFFER_UNIT 1024

#if 0 //!defined(_MSC_VER) && !defined(__MINGW32__)
# define ENABLE_TS
#endif

#ifndef SOCKET
# ifdef WIN32
#  define SOCKET uint32_t
# else
#  define SOCKET int32_t
# endif
#endif

#define UDP_MAX_SIZE 65536

#ifdef DEBUG_UDPPACKETDROPEMUL
Mutex _drop_string_lock;
std::string _drop_string_out;
std::string _drop_string_in;

void set_drop_filter_out(std::string s)
{
    _drop_string_lock.lock();
    _drop_string_out = s;
    _drop_string_lock.unlock();
}

void set_drop_filter_in(std::string s)
{
    _drop_string_lock.lock();
    _drop_string_in = s;
    _drop_string_lock.unlock();
}

static bool drop_out()
{
    char c = '1';
    _drop_string_lock.lock();
    if( _drop_string_out.size()>0 )
    {
        c = _drop_string_out[0];
        _drop_string_out = _drop_string_out.substr(1);
    }
    _drop_string_lock.unlock();
    if( c=='0' )
    {
        return true;
    }
    else
    {
        return false;
    }
}

static bool drop_in()
{
    char c = '1';
    _drop_string_lock.lock();
    if( _drop_string_in.size()>0 )
    {
        c = _drop_string_in[0];
        _drop_string_in = _drop_string_in.substr(1);
    }
    _drop_string_lock.unlock();
    if( c=='0' )
    {
        return true;
    }
    else
    {
        return false;
    }
}
#endif


class Sip_Message_Parser
{
public:
    Sip_Message_Parser();
    ~Sip_Message_Parser();

    SRef<Sip_Message *> feed( uint8_t data );
    void init();
private:
    void expand_buffer();
    uint32_t find_content_length();
    uint8_t * _buffer;
    uint32_t _length;
    uint32_t _index;
    uint8_t _state;
    uint32_t _content_index;
    uint32_t _content_length;
};

Sip_Message_Parser::Sip_Message_Parser()
{
    _buffer = (uint8_t *)malloc( BUFFER_UNIT * sizeof( uint8_t ) );
    for (unsigned int i=0; i< BUFFER_UNIT * sizeof( uint8_t ); i++)
        _buffer[i]=0;

    _length = BUFFER_UNIT;
    _index = 0;
    _content_index = 0;
    _state = 0;
}

Sip_Message_Parser::~Sip_Message_Parser()
{
    free( _buffer );
}


void Sip_Message_Parser::init()
{
    _buffer = (uint8_t *)realloc(_buffer, BUFFER_UNIT * sizeof( uint8_t ) );
    for (unsigned int i=0; i< BUFFER_UNIT * sizeof( uint8_t ); i++)
        _buffer[i]=0;
    _length = BUFFER_UNIT;
    _state = 0;
    _index = 0;
    _content_index = 0;
}


SRef<Sip_Message *> Sip_Message_Parser::feed( uint8_t udata )
{
    char data = (char)udata;
    if( _index >= _length )
    {
        expand_buffer();
    }

    _buffer[_index++] = udata;

    switch( _state )
    {
    case 0:
        if( data == '\n' )
            _state = 1;
        break;
    case 1:
        if( data == '\n' )
        {
            /* Reached the end of the Header */
            _state = 2;
            _content_length = find_content_length();
            if( _content_length == 0 )
            {
                std::string messageString( (char *)_buffer, _index );
#ifdef ENABLE_TS
                char tmp[12];
                tmp[11]=0;
                memcpy(&tmp[0], _buffer , 11);
                ts.save(tmp);
#endif
                init();
                SRef<Sip_Message*> msg = Sip_Message::create_message( messageString );
#ifdef ENABLE_TS
                ts.save("createMessage end");
#endif
                return msg;
            }
            _content_index = 0;
        }
        else if( data != '\r' )
            _state = 0;
        break;
    case 2:
        if( ++_content_index == _content_length )
        {
#ifdef ENABLE_TS
            char tmp[12];
            tmp[11]=0;
            memcpy(&tmp[0], _buffer , 11);
            ts.save(tmp);
#endif
            std::string messageString( (char*)_buffer, _index );
            SRef<Sip_Message*> msg = Sip_Message::create_message( messageString );
#ifdef ENABLE_TS
            ts.save("createMessage end");
#endif
            init();
            return msg;
        }
        break;
    default:
        /*never reached*/
        my_assert(0);
        break;
    }
    return NULL;
}


void Sip_Message_Parser::expand_buffer()
{
    _buffer = (uint8_t *)realloc( _buffer, BUFFER_UNIT * ( _length / BUFFER_UNIT + 1 ) );
    _length += BUFFER_UNIT;
}

static int32_t find_int_header_value(char*buf, uint32_t buflen, std::string hName)
{
    hName = "\n"+hName;
    uint32_t nlen = hName.length();

    //search whole buffer on each position
    for (int i=0; i+nlen < buflen; i++)
    {
        if( strNCaseCmp( hName.c_str(), (char *)(buf + i) , nlen  ) == 0 )
        {
            uint32_t hnend = i+nlen;
            while (hnend<buflen&& is_ws(buf[hnend] ) )
                hnend++;
            //Break if not content length header (for example
            //on header "Content-Length-Somethingelse:")
            if (buf[hnend]!=':')
                continue;

            hnend++;

            int tmpi=i;
            uint32_t hend = Sip_Utils::find_end_of_header((const char*)buf,buflen,tmpi);
            uint32_t valstart=hnend;
            while (valstart < hend && is_ws(buf[valstart]))
                valstart++;
            return atoi(buf+valstart);
        }
    }
    return -1;
}

uint32_t Sip_Message_Parser::find_content_length()
{
    int32_t clen = find_int_header_value((char*)_buffer, _index, "Content-Length");
    if( clen < 0 )
        clen = find_int_header_value((char*)_buffer, _index, "l");
    if( clen < 0 )
        clen = 0;
    return clen;
}

static void updateVia(SRef<Sip_Message*> pack, SRef<IPAddress *>from, uint16_t port)
{
    SRef<Sip_Header_Value_Via*> via = pack->get_first_via();
    std::string peerAddr = from->get_string();

    if( !via ){
        my_err << "No Via header in incoming message!" << std::endl;
        return;
    }

    if( via->has_parameter( "rport" ) )
    {
        char buf[20] = "";
        sprintf(buf, "%d", port);
        via->set_parameter( "rport", buf);
    }

    std::string addr = via->get_ip();
    if( addr != peerAddr ){
        via->set_parameter( "received", peerAddr );
    }
}

class Stream_Thread_Data : public Input_Ready_Handler
{
public:
    Stream_Thread_Data( SRef<Stream_Socket*>, SRef<Sip_Layer_Transport *> );
    void input_ready( SRef<Socket*> socket );

protected:
    void stream_socket_read( SRef<Stream_Socket *> socket );

private:
    Sip_Message_Parser parser;
    SRef<Sip_Layer_Transport  *> transport;
    SRef<Stream_Socket *> ssocket;
};


Stream_Thread_Data::Stream_Thread_Data( SRef<Stream_Socket *> theSocket, SRef<Sip_Layer_Transport *> t)
    :ssocket( theSocket )
{
    this->transport = t;
}


bool sipdebug_print_packets = false;

void set_debug_print_packets(bool f)
{
    sipdebug_print_packets=f;
}

bool get_debug_print_packets()
{
    return sipdebug_print_packets;
}

uint64_t start_time = 0;

void printMessage(const std::string& hdr, const std::string& packet)
{
    if( start_time == 0 )
        start_time = my_time();
    uint64_t t;
    t = my_time();
    int64_t sec = t / 1000 - start_time / 1000;
    int64_t msec = t - start_time;
    msec = msec%1000;

    std::string header = hdr;

    header = std::string("\n")+(sec<100?std::string("0"):std::string("")) +
            (sec<10?"0":"") +
            itoa((int)sec)+
            ":"+
            (msec<10?"0":"")+
            (msec<100?"0":"")+
            itoa((int)msec)+
            " " +
            header+": ";


    std::string tmp = std::string("\n") + packet;
    replace_all(tmp,"\n", header);
    my_out << tmp << std::endl;
}

void Stream_Thread_Data::input_ready( SRef<Socket*> socket )
{
    stream_socket_read( ssocket );
}

#define STREAM_MAX_PKT_SIZE 65536

void Stream_Thread_Data::stream_socket_read( SRef<Stream_Socket *> socket )
{
    char buffer[STREAM_MAX_PKT_SIZE+1];
    for (int i=0; i< STREAM_MAX_PKT_SIZE+1; i++)
    {
        buffer[i]=0;
    }
    SRef<Sip_Message*> pack;

    int32_t nread;
    nread = socket->read( buffer, STREAM_MAX_PKT_SIZE);

    if (nread == -1)
    {
        my_dbg("signaling/sip") << "Some error occured while reading from Stream_Socket" << std::endl;
        return;
    }

    if ( nread == 0)
    {
        // Connection was closed
        my_dbg("signaling/sip") << "Connection was closed" << std::endl;
        transport->remove_socket( socket );
        return;
    }
#ifdef ENABLE_TS
    //ts.save( PACKET_IN );
#endif

    try{
        uint32_t i;
        for( i = 0; i < (uint32_t)nread; i++ )
        {
            pack = parser.feed( buffer[i] );

            if( pack )
            {
                if (sipdebug_print_packets)
                {
                    printMessage("IN (STREAM)", buffer);
                }
                //cerr << "Packet string:\n"<< pack->get_string()<< "(end)"<<std::endl;

                SRef<IPAddress *> peer = socket->get_peer_address();
                pack->set_socket( *socket );
                updateVia( pack, peer, (uint16_t)socket->get_peer_port() );

                if (transport->validate_incoming( pack ) )
                { // drop here if it does not look ok
                    Sip_SMCommand cmd(pack, Sip_SMCommand::transport_layer, Sip_SMCommand::transaction_layer);
                    if (transport->_dispatcher)
                    {
                        transport->_dispatcher->enqueue_command( cmd, LOW_PRIO_QUEUE );
                    }else
                        my_dbg("signaling/sip") << "SipLayerTransport: ERROR: NO SIP MESSAGE RECEIVER - DROPPING MESSAGE"<<std::endl;
                }
                pack = NULL;
            }
        }
    }

    catch(Sip_Exception_Invalid_Message &e )
    {
        my_dbg("signaling/sip") << "INFO: SipLayerTransport::streamSocketRead: dropping malformed packet: "<<e.what()<<std::endl;

#if 0
        // Check that we received data
        // is not too big, in which case close
        // the socket, to avoid DoS
        if(socket->received.size() > 8192)
        {
            break;
        }
#endif
        /* Probably we don't have enough data
                 * so go back to reading */
        // 				continue;
    }

    catch(Sip_Exception_Invalid_Start & )
    {
        // This does not look like a SIP
        // packet, close the connection

        my_dbg("signaling/sip") << "This does not look like a SIP packet, close the connection" << std::endl;
        socket->close();
        transport->remove_socket( socket );
    }
}


Sip_Layer_Transport::Sip_Layer_Transport( SRef<Certificate_Chain *> cchain,
                                          SRef<Certificate_Set *> cert_db)
    : _cert_chain(cchain), _cert_db(cert_db), _tls_ctx(NULL)
{
    _contact_udp_port = 0;
    _contact_sip_port = 0;
    _contact_sips_port = 0;
    _manager = new Socket_Server;
    _manager->start();
}

Sip_Layer_Transport::~Sip_Layer_Transport()
{
}

void Sip_Layer_Transport::stop()
{
    _servers_lock.lock();

    _manager->stop();

    std::list<SRef<Sip_Socket_Server *> >::iterator iter;
    std::list<SRef<Sip_Socket_Server *> >::iterator iter_end = _servers.end();
    for( iter = _servers.begin(); iter != iter_end; ++iter )
    {
        SRef<Sip_Socket_Server *> server = *iter;
        server->stop();
    }

    _manager->join();
    _manager->close_sockets();
    _manager = NULL;

    for( iter = _servers.begin(); iter != iter_end; ++iter )
    {
        SRef<Sip_Socket_Server *> server = *iter;
        server->join();
        server->close_sockets();
        server->free();
        server->set_receiver(NULL);
    }

    _servers.clear();
    _servers_lock.unlock();
}

bool Sip_Layer_Transport::handle_command(const Sip_SMCommand& command)
{
    if( command.get_type() == Sip_SMCommand::COMMAND_PACKET )
    {
        SRef<Sip_Message*> pack = command.get_command_packet();

        std::string branch = pack->get_branch();
        bool addVia = pack->get_type() != Sip_Response::type;

        if( branch== "" )
        {
            branch = "z9hG4bK" + itoa(rand());		//magic cookie plus random number
        }
        send_message(pack, branch, addVia);
        return true;
    }
    return 0;
}

void Sip_Layer_Transport::add_server(SRef<Sip_Socket_Server *> server)
{
    _servers_lock.lock();
    server->start();
    _servers.push_back(server);
    _servers_lock.unlock();
}

SRef<Sip_Socket_Server *> Sip_Layer_Transport::find_server( int32_t type, bool ipv6)
{
    std::list<SRef<Sip_Socket_Server *> >::iterator iter;
    std::list<SRef<Sip_Socket_Server *> >::iterator iter_end = _servers.end();
    for( iter = _servers.begin(); iter != iter_end; ++iter )
    {
        SRef<Sip_Socket_Server *> server = *iter;
        if( server->is_ipv6() == ipv6 && server->get_type() == type )
            return server;
    }
#ifdef DEBUG_OUTPUT
    cerr << "SipLayerTransport::findServer not found type=" << type << " ipv6=" << ipv6 << std::endl;
#endif
    return NULL;
}


SRef<Socket *> Sip_Layer_Transport::find_server_socket( int32_t type, bool ipv6)
{
    SRef<Sip_Socket_Server *> server;
    _servers_lock.lock();
    server = find_server(type, ipv6);
    _servers_lock.unlock();

    if( !server)
        return NULL;

    SRef<Socket *> sock = server->get_socket();
    return sock;
}

SRef<Sip_Transport*> getSocketTransport( SRef<Socket*> socket )
{
    SRef<Sip_Transport*> transport =
            Sip_Transport_Registry::get_instance()->find_transport( socket->get_type() );

    if( !transport )
    {
        my_dbg("signaling/sip") << "SipLayerTransport: Unknown transport protocol " + socket->get_type() <<std::endl;
        // TODO more describing exception and message
        throw Network_Exception();
    }
    return transport;
}

void getIpPort( SRef<Sip_Socket_Server*> server, SRef<Socket*> socket,
                std::string &ip, uint16_t &port )
{
    if( server )
    {
        port = server->get_external_port();
        ip = server->get_external_ip();
    }
    else
    {
        port = socket->get_port();
        ip = socket->get_local_address()->get_string();
    }
}


void Sip_Layer_Transport::add_via_header( SRef<Sip_Message*> pack, SRef<Sip_Socket_Server*> server,
                                          SRef<Socket *> socket, std::string branch )
{
    SRef<Sip_Transport*> transport;
    uint16_t port;
    std::string ip;

    if( !socket )
        return;

    //The Via header for CANCEL requests
    //is added when the packet is created
    if( pack->get_type() == "CANCEL" )
        return;

    transport = getSocketTransport( socket );

    getIpPort( server, socket, ip, port );

    SRef<Sip_Header_Value*> hdr_val =
            new Sip_Header_Value_Via(transport->get_via_protocol(), ip, port);

    // Add rport parameter, defined in RFC 3581
    hdr_val->add_parameter(new Sip_Header_Parameter("rport", "", false));
    hdr_val->set_parameter("branch",branch);

    SRef<Sip_Header*> hdr = new Sip_Header( hdr_val );

    pack->add_before( hdr );
}


static bool lookupDestSrv(const std::string &domain, SRef<Sip_Transport*> transport,
                          std::string &dest_addr, int32_t &destPort)
{
    //Do a SRV lookup according to the transport ...
    std::string srv = transport->get_srv();
    uint16_t port = 0;

    std::string addr = Network_Functions::get_host_handling_service(srv, domain, port);
#ifdef DEBUG_OUTPUT
    cerr << "getDestIpPort : srv=" << srv << "; domain=" << domain << "; port=" << port << "; target=" << addr << std::endl;
#endif

    if( addr.size() > 0 )
    {
        dest_addr = addr;
        destPort = port;
        return true;
    }

    return false;
}


// RFC 3263 4.1 Determining transport using NAPTR
static bool lookupNaptrTransport(const Sip_Uri &uri,
                                 SRef<Sip_Transport*> &dest_transport,
                                 std::string &dest_addr)
{
    SRef<Sip_Transport_Registry*> registry =
            Sip_Transport_Registry::get_instance();
    bool secure = uri.get_protocol_id() == "sips";

    std::list<std::string> services = registry->get_naptr_services( secure );
    SRef<Dns_Naptr_Query*> query = Dns_Naptr_Query::create();

    query->set_accept( services );

    if( !query->resolve( uri.get_ip(), "" ))
    {
        my_dbg("signaling/sip") << "NAPTR lookup failed" << std::endl;
        return false;
    }
    else if( query->get_result_type() != Dns_Naptr_Query::SRV )
    {
        my_dbg("signaling/sip") << "SipLayerTransport: NAPTR failed, invalid result type " << std::endl;
        return false;
    }

    const std::string &service = query->get_service();

    dest_transport = registry->find_transport_by_naptr( service );
    dest_addr = query->get_result();
    return true;
}


// RFC 3263 4.2 Determining Port and IP Address
static bool lookupDestIpPort(const Sip_Uri &uri, SRef<Sip_Transport*> transport,
                             std::string &dest_addr, int32_t &destPort)
{
    bool res = false;

    if( !uri.is_valid() )
        return false;

    std::string addr = uri.get_ip();
    int32_t port = uri.get_port();

    if( addr.size()>0 )
    {
        if( IPAddress::is_numeric( addr ) )
        {
            res = true;
            if( !port )
            {
                port = transport->get_default_port();
            }
        }
        // Not numeric
        else if( port )
        {
            // Lookup A or AAAA
            res = true;
        }
        // Lookup SRV
        else if( lookupDestSrv( uri.get_ip(), transport,
                                addr, port ))
        {
            res = true;
        }
        else
        {
            // Lookup A or AAAA
            port = transport->get_default_port();
            res = true;
        }
    }

    if( res )
    {
        dest_addr = addr;
        destPort = port;
    }

    my_dbg("signaling/sip") << "lookupDestIpPort " << res << " " << dest_addr << ":" << destPort << ";transport=" << transport->get_name() << std::endl;
    return res;
}

bool Sip_Layer_Transport::get_destination(SRef<Sip_Message*> pack, std::string &dest_addr,
                                          int32_t &destPort, SRef<Sip_Transport*> &dest_transport)
{
    SRef<Sip_Transport_Registry *> registry =
            Sip_Transport_Registry::get_instance();

    if( pack->get_type() == Sip_Response::type )
    {
        // RFC 3263, 5 Server Usage
        // Send responses to sent by address in top via.

        SRef<Sip_Header_Value_Via*> via = pack->get_first_via();

        if ( via )
        {
            dest_addr = via->get_parameter( "received" );

            if( dest_addr.empty() )
            {
                dest_addr = via->get_ip();
            }

            if( dest_addr.size()>0 )
            {
                std::string rport = via->get_parameter( "rport" );
                if( rport != "" )
                {
                    destPort = atoi( rport.c_str() );
                }
                else
                {
                    destPort = via->get_port();
                }
                dest_transport =
                        registry->find_via_transport( via->get_protocol() );
                if( !destPort && dest_transport )
                {
                    destPort = dest_transport->get_default_port();
                }
                return true;
            }
        }
    }
    else
    {
        // RFC 3263, 4 Client Usage

        // Send requests to address in first route if the route set
        // is non-empty, or directly to the reqeuest uri if the
        // route set is empty.

        SRef<Sip_Header_Value*> routeHeader =
                pack->get_header_value_no(SIP_HEADER_TYPE_ROUTE, 0);

        Sip_Uri uri;

        if( routeHeader )
        {
            SRef<Sip_Header_Value_Route*> route =
                    (Sip_Header_Value_Route*)*routeHeader;
            std::string str = route->get_string();
            uri.set_uri( str );
        }
        else
        {
            SRef<Sip_Request*> req = (Sip_Request*)*pack;
            uri = req->get_uri();
        }

        my_dbg("signaling/sip") << "Destination URI: " << uri.get_request_uri_string() << std::endl;

        if( uri.is_valid() )
        {
            // RFC 3263, 4.1 Selecting a Transport Protocol

            //dest_addr = IPAddress::create( uri.get_ip() );
            dest_addr = uri.get_ip();

            if( /*dest_addr*/ dest_addr.size()>0 )
            {
                bool secure = uri.get_protocol_id() == "sips";
                std::string protocol = uri.get_transport();
                if( protocol.length() > 0 )
                {
                    dest_transport = registry->find_transport( protocol, secure );
                    // TODO using TLS in transport parameter is deprecated
                    if( !dest_transport )
                    {
                        // Second, try Via protocol
                        // For example TLS
                        dest_transport = registry->find_via_transport( protocol );
                    }
                }
                else if( !IPAddress::is_numeric( dest_addr ) )
                {
                    lookupNaptrTransport(uri, dest_transport, dest_addr);
                    // TODO fallback to SRV lookup of
                    // all supported protocols
                    // _sip._udp etc.
                }

                // Fallback
                if( !dest_transport )
                {
                    if( secure )
                        dest_transport = registry->find_transport( "tcp", true );
                    else
                    {
                        if (find_server_socket(SSOCKET_TYPE_UDP, false))
                        {
                            dest_transport = registry->find_transport( "udp" );
                        }
                        else if (find_server_socket(SSOCKET_TYPE_TCP, false))
                        {
                            dest_transport = registry->find_transport( "tcp" );
                        }
                        else if (find_server_socket(SSOCKET_TYPE_TLS, false))
                        {
                            dest_transport = registry->find_transport( "tcp", true );
                        }
                        else
                        {
                            // this should not happen
                            my_err << "SipMessateTransport: Warning: could not find any supported transport - trying UDP"<<std::endl;
                            dest_transport = registry->find_transport( "udp" );
                        }
                    }
                }

                if( !dest_transport )
                {
                    my_dbg("signaling/sip") << "SipLayerTransport: Unsupported transport " << protocol << " secure:" << secure << std::endl;
                    return false;
                }

                return lookupDestIpPort(uri, dest_transport, dest_addr, destPort);
            }
        }
        else
        {
            my_dbg("signaling/sip") << "SipLayerTransport: URI invalid " << std::endl;
        }
    }
    return false;
}

void Sip_Layer_Transport::send_message(SRef<Sip_Message*> pack, const std::string &branch, bool addVia)
{
    std::string dest_addr;
    int32_t destPort = 0;
    SRef<Sip_Transport*> dest_transport;

    if( !get_destination( pack, dest_addr, destPort, dest_transport) )
    {
#ifdef DEBUG_OUTPUT
        cerr << "SipLayerTransport: WARNING: Could not find destination. Packet dropped."<<std::endl;
#endif
        return;
    }

    send_message( pack, /* **dest_addr */ dest_addr, destPort, branch, dest_transport, addVia );
}


bool Sip_Layer_Transport::find_socket( SRef<Sip_Transport*> transport, IPAddress &dest_addr, uint16_t port,
                                       SRef<Sip_Socket_Server*> &server, SRef<Socket*> &socket)
{
    bool ipv6 = false;
    int32_t type = 0;

    ipv6 = (dest_addr.get_type() == IP_ADDRESS_TYPE_V6);
    type = transport->get_socket_type();

    _servers_lock.lock();
    server = find_server(type, ipv6);
    _servers_lock.unlock();

    if( type & SSOCKET_TYPE_STREAM )
    {
        SRef<Stream_Socket*> ssocket = find_stream_socket(dest_addr, port);
        if( ssocket.is_null() )
        {
            /* No existing Stream_Socket to that host,
             * create one */
            my_dbg << "SipLayerTransport: sendMessage: creating new socket" << std::endl;

            ssocket = transport->connect( dest_addr, port,
                                          _cert_db, get_certificate_chain() );
            add_socket( ssocket );
        }
        else my_dbg << "SipLayerTransport: sendMessage: reusing old socket" << std::endl;
        socket = *ssocket;
    }
    else
    {
        if( server )
        {
            socket = server->get_socket();
        }
    }

    if( !socket )
    {
        throw Network_Exception();
    }

    return !socket.is_null();
}


bool Sip_Layer_Transport::validate_incoming(SRef<Sip_Message *> msg)
{
    bool isRequest = (msg->get_type() != Sip_Response::type);
    bool isInvite = (msg->get_type() == "INVITE");
    // check that required headers are present


    /*	if (!msg->get_headerValueFrom()){
        //too severely damaged to answer (could try, but why bother?)
        return false;
    }
*/

    if (!msg->get_header_value_from() || !msg->get_header_value_to()
            || (isInvite && !msg->get_header_value_no(SIP_HEADER_TYPE_CONTACT,0)))
    {
        if (isRequest)
        {
            SRef<Sip_Message*> resp = new Sip_Response( 400, "Required header missing", (Sip_Request*)*msg );
            resp->set_socket( msg->get_socket() );
            send_message(resp, "TL", false);
        }
        return false;
    }
    return true;
}


void Sip_Layer_Transport::update_contact(SRef<Sip_Message*> pack, SRef<Sip_Socket_Server *> server, SRef<Socket *> socket)
{
    SRef<Sip_Header_Value_Contact*> contactp = pack->get_header_value_contact();
    uint16_t port;
    std::string ip;

    if( !contactp )
        return;

    Sip_Uri contactUri = contactp->get_uri();

    if( contactUri.has_parameter("minisip") ){

        bool ipv6 = socket->get_local_address()->get_type() == IP_ADDRESS_TYPE_V6;
        // Copy scheme/protocol-id from the request URI
        SRef<Sip_Request*> req;
        if( pack->get_type() == Sip_Response::type )
        {
            SRef<Sip_Response*> resp = dynamic_cast<Sip_Response*>(*pack);
            req = resp->get_request();
        }
        else
        {
            req = dynamic_cast<Sip_Request*>(*pack);
        }

        const Sip_Uri &uri = req->get_uri();
        bool secure = uri.get_protocol_id() == "sips";
        SRef<Sip_Transport*> transport = getSocketTransport( socket );

        // Can't use local server address in the contact unless
        // both the contact and the server use sip or sips. They can't
        // use different schemes.
        if( secure != transport->is_secure() )
        {
            server = NULL;
        }
        else if( !server )
        {
            server = find_server(socket->get_type(), ipv6);
        }

        getIpPort( server, socket, ip, port );

        contactUri.set_protocol_id( uri.get_protocol_id() );
        contactUri.set_ip( ip );
        contactUri.set_transport( transport->get_protocol() );

        //NOTE: We make the UDP and IPv4 exception for STUN reasons. However, since
        //we set the IP above, this is broken. If we make this exception, then
        //the contact uri will not have the correct port if we run on any port
        //except 5060.
        //
        //		if(ipv6 || socket->get_type() != MSOCKET_TYPE_UDP){

        // Update port if not UDP and IPv4
        contactUri.set_port( port );
        //		}

        contactUri.remove_parameter("minisip");
        contactp->set_uri( contactUri );
    }
}


void Sip_Layer_Transport::send_message(SRef<Sip_Message*> pack, const std::string &ip_addr,
                                       int32_t port, std::string branch, SRef<Sip_Transport*> transport, bool addVia )
{
    SRef<Socket *> socket;
    SRef<IPAddress *> tempAddr;
    SRef<Sip_Socket_Server *> server;
#ifdef DEBUG_OUTPUT
    Sip_Request* req = dynamic_cast<Sip_Request*>(*pack);
    Sip_Response* resp = dynamic_cast<Sip_Response*>(*pack);
    std::string str;
    if (req)
    {
        str = req->get_method();
    }
    if (resp)
    {
        str = itoa(resp->get_status_code());
    }
    cerr << "SipLayerTransport:  sendMessage addr=" << ip_addr << ", port=" << port << " " << str << std::endl;
#endif


    try{
        socket = pack->get_socket();
        SRef<IPAddress *>dest_addr;
        if( !socket )
        {
            // Lookup IPv4 or IPv6 address
            dest_addr = IPAddress::create(ip_addr);
        }
        else
        {
            // Lookup IPv4 or IPv6 depending on open socket
            int32_t type = socket->get_local_address()->get_type();
            dest_addr = IPAddress::create(ip_addr, type == IP_ADDRESS_TYPE_V6);
        }

        if( !dest_addr )
        {
            throw Host_Not_Found( ip_addr );
        }

        if( !socket )
        {
            find_socket(transport, **dest_addr, (uint16_t)port, server, socket);
            pack->set_socket( socket );

            if( !socket )
            {
                // TODO add sensible message
                std::cerr << "No socket!!" << std::endl;
                throw Network_Exception();
            }
        }

        update_contact( pack, server, socket );

        if (addVia)
        {
            add_via_header( pack, server, socket, branch );
        }

        std::string packetString = pack->get_string();

        SRef<Datagram_Socket *> dsocket = dynamic_cast<Datagram_Socket*>(*socket);
        SRef<Stream_Socket *> ssocket = dynamic_cast<Stream_Socket*>(*socket);

        if( ssocket )
        {
            /* At this point if socket != we send on a
             * streamsocket */
            if (sipdebug_print_packets)
            {
                printMessage("OUT (STREAM)", packetString);
            }
#ifdef ENABLE_TS
            //ts.save( PACKET_OUT );
            char tmp[12];
            tmp[11]=0;
            memcpy(&tmp[0], packetString.c_str() , 11);
            ts.save( tmp );
#endif
            if( ssocket->write( packetString ) == -1 )
            {
                throw Send_Failed( errno );
            }
        }
        else if( dsocket )
        {
            /* otherwise use the UDP socket */
            if (sipdebug_print_packets){
                printMessage("OUT (UDP)", packetString);
            }
#ifdef ENABLE_TS
            //ts.save( PACKET_OUT );
            char tmp[12];
            tmp[11]=0;
            memcpy(&tmp[0], packetString.c_str() , 11);
            ts.save( tmp );

#endif
            // 			SRef<IPAddress *>dest_addr = IPAddress::create(ip_addr);

#ifdef DEBUG_UDPPACKETDROPEMUL
            if (!drop_out())
#endif
                if( dsocket->sendto( **dest_addr, port,
                                     (const void*)packetString.c_str(),
                                     (int32_t)packetString.length() ) == -1 )
                {
                    throw Send_Failed( errno );
                }
        }
        else
        {
            std::cerr << "No valid socket!" << std::endl;
        }
    }
    catch( Network_Exception & exc )
    {
        std::string message = exc.what();
        std::string callId = pack->get_call_id();
#ifdef DEBUG_OUTPUT
        my_dbg("signaling/sip") << "Transport error in SipLayerTransport: " << message << std::endl;
        cerr << "SipLayerTransport: sendMessage: exception thrown! " << message << std::endl;
#endif
        Command_String transportError( branch + pack->get_cseq_method(),
                                       Sip_Command_String::transport_error,
                                       "SipLayerTransport: " + message );
        Sip_SMCommand transportErrorCommand( transportError,
                                             Sip_SMCommand::transport_layer,
                                             Sip_SMCommand::transaction_layer);

        if (_dispatcher)
            _dispatcher->enqueue_command( transportErrorCommand, LOW_PRIO_QUEUE );
        else
            my_dbg("signaling/sip")<< "SipLayerTransport: ERROR: NO SIP COMMAND RECEIVER - DROPPING COMMAND"<<std::endl;
    }
}

void Sip_Layer_Transport::set_dispatcher(SRef<Sip_Command_Dispatcher*> d)
{
    _dispatcher = d;
}


void Sip_Layer_Transport::add_socket(SRef<Stream_Socket *> sock)
{
    SRef<Stream_Thread_Data*> worker = new Stream_Thread_Data( sock, this );

    _manager->add_socket( *sock, dynamic_cast<Input_Ready_Handler*>(*worker) );
}

void Sip_Layer_Transport::remove_socket(SRef<Stream_Socket *> sock)
{
    _manager->remove_socket(*sock);
}

SRef<Stream_Socket *> Sip_Layer_Transport::find_stream_socket(IPAddress& address, uint16_t port)
{
    SRef<Socket *> sock = _manager->find_stream_socket_peer( address, port );

    if( !sock )
    {
        return NULL;
    }
    return dynamic_cast<Stream_Socket*>(*sock);
}


SRef<Certificate_Chain *> Sip_Layer_Transport::get_certificate_chain()
{
    return _cert_chain;
}

SRef<Certificate*> Sip_Layer_Transport::get_my_certificate()
{
    return _cert_chain->get_first();
}

SRef<Certificate_Set *> Sip_Layer_Transport::get_certificate_set ()
{
    return _cert_db;
}

void Sip_Layer_Transport::datagram_socket_read(SRef<Datagram_Socket *> sock)
{
    char buffer[UDP_MAX_SIZE];

    SRef<Sip_Message*> pack;
    int nread;

    if( sock )
    {
        SRef<IPAddress *> from;
        int port = 0;

        nread = sock->recvfrom((void *)buffer, UDP_MAX_SIZE, from, port);

        if( nread == -1 )
        {
            my_dbg("signaling/sip") << "Some error occured while reading from UdpSocket"<<std::endl;
            return;
        }

        if( nread == 0 )
        {
            // Connection was closed
            return; // FIXME
        }

#ifdef DEBUG_UDPPACKETDROPEMUL
        if( drop_in() )
        {
            return;
        }
#endif

        if( nread < (int)strlen("SIP/2.0") )
        {
            return;
        }


        try{
#ifdef ENABLE_TS
            //ts.save( PACKET_IN );
            char tmp[12];
            tmp[11]=0;
            memcpy(&tmp[0], buffer, 11);
            ts.save( tmp );

#endif
            std::string data = std::string(buffer, nread);
            if (sipdebug_print_packets)
            {
                printMessage("IN (UDP)", data);
            }
            pack = Sip_Message::create_message( data );

            pack->set_socket( *sock );
            updateVia(pack, from, (uint16_t)port);

            if (validate_incoming(pack))
            { // drop here if it does not look ok
                Sip_SMCommand cmd(pack,
                                  Sip_SMCommand::transport_layer,
                                  Sip_SMCommand::transaction_layer);
                if( _dispatcher )
                    _dispatcher->enqueue_command( cmd, LOW_PRIO_QUEUE );
                else
                    my_dbg("signaling/sip") << "SipLayerTransport: ERROR: NO SIP MESSAGE RECEIVER - DROPPING MESSAGE"<<std::endl;
            }
            pack = NULL;
        }

        catch(Sip_Exception_Invalid_Message & )
        {
            /* Probably we don't have enough data
                 * so go back to reading */
#ifdef DEBUG_OUTPUT
            my_dbg("signaling/sip") << "Invalid data on UDP socket, discarded" << std::endl;
#endif
            return;
        }

        catch(Sip_Exception_Invalid_Start & )
        {
            // This does not look like a SIP
            // packet, close the connection

#ifdef DEBUG_OUTPUT
            my_dbg("signaling/sip") << "Invalid data on UDP socket, discarded" << std::endl;
#endif
            return;
        }
    } // if event
}

void Sip_Layer_Transport::start_server( SRef<Sip_Transport*> transport, const std::string & ipString,
                                        const std::string & ip6String, int32_t &prefPort, int32_t externalUdpPort,
                                        SRef<Certificate_Chain *> certChain, SRef<Certificate_Set *> cert_db)
{
    SRef<Sip_Socket_Server *> server;
    SRef<Sip_Socket_Server *> server6;
    int32_t port = prefPort;

    server = transport->create_server( this, false, ipString, port, cert_db, certChain );

    if( !server )
    {
        my_dbg << "SipLayerTransport: startServer failed to create server" << std::endl;
        return;
    }

    if( externalUdpPort ){
        server->set_external_port( externalUdpPort );
    }

    if( transport->get_name() == "UDP" ){
        _contact_udp_port = server->get_external_port();
    }
    else if( transport->is_secure() ){
        _contact_sips_port = server->get_external_port();
    }
    else{
        _contact_sip_port = server->get_external_port();
    }

    if( ip6String != "" )
    {
        server6 = transport->create_server( this, true, ip6String, port, cert_db, certChain );
        // IPv6 doesn't need different external udp port
        // since it never is NATed.
    }

    add_server( server );
    if( server6 )
        add_server( server6 );

    prefPort = port;
}

void Sip_Layer_Transport::stop_server( SRef<Sip_Transport*> transport )
{
    SRef<Sip_Socket_Server *> server =
            find_server( transport->get_socket_type(), false );

    SRef<Sip_Socket_Server *> server6 =
            find_server( transport->get_socket_type(), true );

    // First stop both the IPv4 and IPv6 servers
    if( server )
        server->stop();
    if( server6 )
        server6->stop();

    // then wait for both threads to exit.
    if( server )
        server->join();
    if( server6 )
        server6->join();
}

int Sip_Layer_Transport::get_local_sip_port( const std::string &transport_name )
{
    if( transport_name == "UDP" || transport_name == "udp" ){
        return _contact_udp_port;
    }

    SRef<Sip_Transport*> transport =
            Sip_Transport_Registry::get_instance()->find_transport_by_name( transport_name );

    if( !transport )
    {
        return 0;
    }

    if( transport->is_secure() )
    {
        return _contact_sips_port;
    }
    else {
        return _contact_sip_port;
    }
}
