#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <map>
#include <sys/select.h>

#include "socket.h"
#include "thread.h"
#include "mutex.h"

class Input_Ready_Handler: public virtual SObject {
    public:
        virtual ~Input_Ready_Handler();
        virtual void input_ready( SRef<Socket*> socket )=0;
};

class Socket_Server : public Runnable
{
public:
    Socket_Server();
    virtual ~Socket_Server();
    void close_sockets();
    void start();
    void stop();

    void join();

    void add_socket( SRef<Socket*> socket, SRef<Input_Ready_Handler*> handler );
    void remove_socket( SRef<Socket*> socket );

    SRef<Socket*> find_stream_socket_peer( const IPAddress &addr, unsigned short port );

protected:
    void run();
    void signal();
    int build_fd_set(fd_set *set, int pipe_fd );

private:
    void create_signal_pipe();
    typedef std::map< SRef<Socket*>, SRef<Input_Ready_Handler*> > Sockets_Map;
    Mutex _cs_mutex;
    SRef<Thread *> _thread;
    Sockets_Map _sockets;
    int _fd_signal;

    int _fd_signal_internal;
    bool _do_stop;
};

#endif // SOCKET_SERVER_H
