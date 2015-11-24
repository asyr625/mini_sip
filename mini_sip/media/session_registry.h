#ifndef SESSION_REGISTRY_H
#define SESSION_REGISTRY_H

#include <list>
#include "sobject.h"
#include "mutex.h"

class Session;

class Session_Registry : public virtual SObject
{
    friend class Session;
public:
    SRef<Session*> get_session(std::string call_id);
    std::list<SRef<Session*> > get_all_session();

protected:
    void register_session(SRef<Session*> session);
    void unregister_session(SRef<Session*> session);

    std::list<SRef<Session*> >  _sessions;
    Mutex _session_lock;
};

#include "session.h"

#endif // SESSION_REGISTRY_H
