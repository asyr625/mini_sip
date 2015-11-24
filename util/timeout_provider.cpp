#include "timeout_provider.h"
#include "my_time.h"

#include "my_types.h"
#include "string_utils.h"

template<class TOCommand, class TOSubscriber>
TPRequest<TOCommand, TOSubscriber>::TPRequest( TOSubscriber tsi, int timeout_ms, const TOCommand &cmd)
    : _subscriber(tsi) ,
      _command(cmd)
{
    _when_ms = my_time();
    _when_ms += timeout_ms;
}

template<class TOCommand, class TOSubscriber>
bool TPRequest<TOCommand, TOSubscriber>::happens_before(uint64_t t)
{
    if (_when_ms < t)
        return true;
    if (_when_ms > t)
        return false;
    return false;
}

template<class TOCommand, class TOSubscriber>
bool TPRequest<TOCommand, TOSubscriber>::happens_before(const TPRequest &req)
{
    return happens_before(req._when_ms);
}

template<class TOCommand, class TOSubscriber>
int TPRequest<TOCommand, TOSubscriber>::get_ms_to_timeout()
{
    uint64_t now = my_time();
    if (happens_before(now))
        return 0;
    else
        return (int)(_when_ms - now);
}

template<class TOCommand, class TOSubscriber>
TOCommand TPRequest<TOCommand, TOSubscriber>::get_command()
{
    return _command;
}

template<class TOCommand, class TOSubscriber>
TOSubscriber TPRequest<TOCommand, TOSubscriber>::get_subscriber()
{
    return _subscriber;
}

template<class TOCommand, class TOSubscriber>
bool TPRequest<TOCommand, TOSubscriber>::operator==(const TPRequest<TOCommand, TOSubscriber> &req)
{
    if (req._subscriber == _subscriber &&
            req._command == _command &&
            req._when_ms == _when_ms)
        return true;
    else
        return false;
}


template<class TOCommand, class TOSubscriber>
Timeout_Provider<TOCommand, TOSubscriber>::Timeout_Provider()
    : _requests(), _wait_cond(new Cond_Var()), _synch_lock(), _stop(false)
{
    _thread = new Thread(this, Thread::Above_Normal_Priority);
}

template<class TOCommand, class TOSubscriber>
Timeout_Provider<TOCommand, TOSubscriber>::~Timeout_Provider()
{
    delete _wait_cond;
    delete _thread;
}

template<class TOCommand, class TOSubscriber>
std::string Timeout_Provider<TOCommand, TOSubscriber>::get_mem_object_type() const
{
    return "TimeoutProvider";
}

template<class TOCommand, class TOSubscriber>
std::string Timeout_Provider<TOCommand, TOSubscriber>::get_timeouts()
{
    std::string ret;
    _synch_lock.lock();

    for( int i=0 ; i < _requests.size(); i++ )
    {
        int ms = _requests[i].get_ms_to_timeout();

        TOSubscriber receiver = _requests[i].get_subscriber();
        ret = ret + "      "
                + std::string("Command: ") + _requests[i].get_command()
                + "  Time: " + itoa(ms/1000) + "." + itoa(ms%1000)
                + "  Receiver ObjectId: "+itoa((int)receiver)
                +"\n";
    }
    _synch_lock.unlock();
    return ret;
}

template<class TOCommand, class TOSubscriber>
std::list<TPRequest<TOCommand, TOSubscriber> > Timeout_Provider<TOCommand, TOSubscriber>::get_timeout_requests()
{
    std::list<TPRequest<TOCommand, TOSubscriber> > retlist;

    _synch_lock.lock();
    for (int i = 0; i< _requests.size(); i++)
        retlist.push_back(_requests[i]);
    _synch_lock.unlock();

    return retlist;
}

template<class TOCommand, class TOSubscriber>
void Timeout_Provider<TOCommand, TOSubscriber>::stop_thread()
{
    _synch_lock.lock();
    _stop = true;
    wake();
    _synch_lock.unlock();

    _thread->join();
    _requests.empty();
}

template<class TOCommand, class TOSubscriber>
void Timeout_Provider<TOCommand, TOSubscriber>::run()
{
#ifdef DEBUG_OUTPUT
    set_thread_name("TimeoutProvider");
#endif
    loop();
}

template<class TOCommand, class TOSubscriber>
void Timeout_Provider<TOCommand, TOSubscriber>::request_timeout(int time_ms, TOSubscriber subscriber, const TOCommand &command)
{
    my_assert(subscriber);
    TPRequest<TOCommand, TOSubscriber> request(subscriber, time_ms, command);

    _synch_lock.lock();

    if( _requests.size() == 0 )
    {
        _requests.push_front(request);
        wake();
        _synch_lock.unlock();
        return;
    }

    if (request.happens_before(_requests[0]))
    {
        _requests.push_front(request);
        wake();
        _synch_lock.unlock();
        return;
    }

    if (_requests[_requests.size()-1].happens_before(request))
    {
        _requests.push_back(request);
        wake();
        _synch_lock.unlock();
        return;
    }

    int i=0;
    while (_requests[i].happens_before(request))
        i++;

    _requests.insert(i,request);

    wake();
    _synch_lock.unlock();
}

template<class TOCommand, class TOSubscriber>
void Timeout_Provider<TOCommand, TOSubscriber>::cancel_request(TOSubscriber subscriber, const TOCommand &command)
{
    _synch_lock.lock();
    int loop_count = 0;
    for (int i=0; loop_count < _requests.size(); i++)
    {
        if (_requests[i].get_subscriber() == subscriber && _requests[i].get_command() == command)
        {
            _requests.remove(i);
            i = 0;
        }
        loop_count++;
    }
    _synch_lock.unlock();
}

template<class TOCommand, class TOSubscriber>
int Timeout_Provider<TOCommand, TOSubscriber>::get_timeout(TOSubscriber subscriber, const TOCommand &command)
{
    _synch_lock.lock();
    for(int i = 0; i < _requests.size(); i++)
    {
        if (_requests[i].get_subscriber() == subscriber && _requests[i].get_command() == command)
        {
            int ret = _requests[i].get_ms_to_timeout();
            _synch_lock.unlock();
            return ret;
        }
    }
    _synch_lock.unlock();
    return -1;
}


template<class TOCommand, class TOSubscriber>
void Timeout_Provider<TOCommand, TOSubscriber>::wake()
{
    _wait_cond->broadcast();
}

template<class TOCommand, class TOSubscriber>
void Timeout_Provider<TOCommand, TOSubscriber>::sleep(int ms)
{
    if( ms > 0 )
    {
        bool unused;
        Cond_Var::wait(*_wait_cond, _synch_lock, unused, ms);
    }
}

template<class TOCommand, class TOSubscriber>
void Timeout_Provider<TOCommand, TOSubscriber>::loop()
{
    _synch_lock.lock();
    do
    {
        int32_t time = 3600000;
        int32_t size = 0;
        if( ( size = _requests.size()) > 0 )
            time = _requests[0].get_ms_to_timeout();

        if( time == 0 && size > 0 )
        {
            if( _stop )
            {
                _synch_lock.unlock();
                return ;
            }

            TPRequest<TOCommand, TOSubscriber> req = _requests[0];
            TOSubscriber subs = req.get_subscriber();
            TOCommand command = req.get_command();

            _requests.remove(req);
            _synch_lock.unlock();

            my_assert(subs);
            subs->timeout(command);
            _synch_lock.lock();
        }
        else
        {
            if( _stop )
            {
                _synch_lock.unlock();
                return ;
            }
            this->sleep(time);
            if( _stop )
            {
                _synch_lock.unlock();
                return ;
            }
        }
    } while(true);
}
