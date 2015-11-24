#include <algorithm>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <cstdio>
#include <cstring>
#include <iostream>
using namespace std;

#include "socket_server.h"
#include "stream_socket.h"
#include "critical_section.h"
#include "network_exception.h"

class Equal_Addr_Port: public std::unary_function< std::pair<const SRef<Socket*>, SRef<Input_Ready_Handler*> >&, bool>{
public:
    Equal_Addr_Port( const IPAddress &theAddress,
                     unsigned short thePort )
        : address( theAddress ), port( thePort ){}

    bool operator() ( std::pair<const SRef<Socket*>, SRef<Input_Ready_Handler*> > &pair  ) {
        SRef<Socket *> sock = pair.first;
        if( sock.is_null() )
            return false;

        Stream_Socket *ssock =
                dynamic_cast<Stream_Socket*>(*sock);

        if( !ssock )
            return false;

        return ssock->matches_peer( address, port );
    }
private:
    const IPAddress &address;
    unsigned short port;
};

Input_Ready_Handler::~Input_Ready_Handler()
{
}

Socket_Server::Socket_Server() :
    _fd_signal(-1), _do_stop(false)
{
}

Socket_Server::~Socket_Server()
{
    stop();
    join();
}

void Socket_Server::start()
{
    Critical_Section cs(_cs_mutex);

    if( !_thread.is_null() )
        return ;
    if( _fd_signal < 0 )
        create_signal_pipe();

    _thread = new Thread(this, Thread::High_Priority);
}

void Socket_Server::stop()
{
    Critical_Section cs(_cs_mutex);
    if( !_thread.is_null() )
        return ;

    _do_stop = true;
    signal();
}

void Socket_Server::join()
{
    Critical_Section cs(_cs_mutex);
    if( !_thread.is_null() )
        return ;

    _thread->join();
    _thread = NULL;
}

void Socket_Server::add_socket( SRef<Socket*> socket, SRef<Input_Ready_Handler*> handler )
{
    Critical_Section cs(_cs_mutex);
    _sockets[socket] = handler;
}


void Socket_Server::remove_socket( SRef<Socket*> socket )
{
    Critical_Section cs(_cs_mutex);

    Sockets_Map::iterator iter = _sockets.find(socket);
    if( iter != _sockets.end() )
    {
        _sockets.erase(iter);
        signal();
    }
}

SRef<Socket*> Socket_Server::find_stream_socket_peer( const IPAddress &addr, unsigned short port )
{
    Critical_Section cs(_cs_mutex);

    Equal_Addr_Port pred(addr, port);

    Sockets_Map::iterator iter = std::find_if(_sockets.begin(), _sockets.end(), pred);

    if( iter == _sockets.end() )
        return NULL;

    return iter->first;
}

void Socket_Server::signal()
{
    char c = 0;

    if( _fd_signal < 0 )
        return;

#ifdef WIN32
    if( send( _fd_signal, &c, sizeof( c ), 0) < 0 ) {
#else
    if( write( _fd_signal, &c, sizeof( c )) < 0 ) {
#endif
        cerr << "Write failed " << _fd_signal << endl;
        throw Network_Exception( errno );
    }
}

int Socket_Server::build_fd_set( fd_set *set, int pipe_fd )
{
    int max_fd = -1;

    Sockets_Map::iterator iter;
    max_fd = pipe_fd;

    FD_ZERO( set );
    FD_SET( pipe_fd, set );

    for( iter = _sockets.begin(); iter != _sockets.end(); ++iter )
    {
        SRef<Socket*> socket = iter->first;
        int fd = socket->get_fd();

        FD_SET(fd, set);
        if( fd > max_fd )
            max_fd = fd;
    }
    return max_fd;
}

#ifdef WIN32
bool create_tcp_pipe(int fds[2])
{

}
#endif

void Socket_Server::create_signal_pipe()
{
    int32_t pipe_fds[2] = {-1,-1};

    if( _fd_signal >= 0 ){
        close( _fd_signal );
        _fd_signal = -1;
    }

#ifdef WIN32
    // Use TCP sockets since Windows pipes don't support select
    if( create_tcp_pipe( pipe_fds )) {
#else
    if( ::pipe( pipe_fds ) ){
#endif
        throw Exception( "Can't create pipe" );
    }

    _fd_signal = pipe_fds[1];
    _fd_signal_internal = pipe_fds[0];
}

void Socket_Server::close_sockets()
{
    Critical_Section cs(_cs_mutex);
    Sockets_Map::const_iterator iter;
    for(iter = _sockets.begin(); iter != _sockets.end(); ++iter)
    {
        SRef<Socket*> socket = iter->first;
        socket->close();
    }
}


void Socket_Server::run()
{
#ifdef DEBUG_OUTPUT
    set_thread_name("Socket_Server::run");
#endif

    struct timeval timeout;
    fd_set tmpl;
    fd_set set;

    int max_fd = -1;

    Sockets_Map::const_iterator iter;

    if( _fd_signal < 0 )
         create_signal_pipe();

    while( !_do_stop )
    {
        max_fd = build_fd_set( &tmpl, _fd_signal_internal );

        int avali;

        do
        {
            memcpy(&set, &tmpl, sizeof(set));

            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            avali = select(max_fd + 1, &set, NULL, NULL, &timeout);
            if( avali < 0 && !_do_stop )
                Thread::msleep(500);
        } while( avali < 0 && !_do_stop );

        if( avali == 0 )
        {
        }

        if( !_do_stop && FD_ISSET(_fd_signal_internal, &set ) )
        {
            char buf[255];
#ifdef WIN32
            if( recv( _fd_signal_internal, buf, sizeof(buf), 0 ) < 0){
#else
            if( read( _fd_signal_internal, buf, sizeof(buf) ) < 0){
#endif
                cerr << "Read failed" << endl;
                throw Network_Exception( errno );
            }
        }

        if( _do_stop )
            break;

        _cs_mutex.lock();
        Sockets_Map scopy = _sockets;
        _cs_mutex.unlock();

        for( iter = scopy.begin(); iter != scopy.end(); ++iter )
        {
            SRef<Socket*> socket = iter->first;
            SRef<Input_Ready_Handler*> handler = iter->second;

            if( FD_ISSET(socket->get_fd(), &set) )
            {
                if( !handler.is_null() )
                    handler->input_ready(socket);
            }
        }

    } // end while

    close(_fd_signal_internal);
    close(_fd_signal);
    _fd_signal = -1;
}
