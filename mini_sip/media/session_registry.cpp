#include "session_registry.h"


SRef<Session*> Session_Registry::get_session(std::string call_id)
{
    std::list<SRef<Session *> >::iterator iSession;

    _session_lock.lock();
    for( iSession = _sessions.begin(); iSession != _sessions.end(); iSession++ )
    {
        if( (*iSession)->get_call_id() == call_id )
        {
            _session_lock.unlock();
            return *iSession;
        }
    }
    _session_lock.unlock();
    return NULL;
}

std::list<SRef<Session*> > Session_Registry::get_all_session()
{
    std::list<SRef<Session *> > sessionsRet;
    _session_lock.lock();
    sessionsRet = _sessions;
    _session_lock.unlock();
    return sessionsRet;
}

void Session_Registry::register_session(SRef<Session*> session)
{
    _session_lock.lock();
    _sessions.push_back( session );
    _session_lock.unlock();
}

void Session_Registry::unregister_session(SRef<Session*> session)
{
    _session_lock.lock();
    _sessions.remove( session );
    _session_lock.unlock();
}
