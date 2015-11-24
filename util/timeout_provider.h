#ifndef TIMEOUT_PROVIDER_H
#define TIMEOUT_PROVIDER_H
#include <list>
#include "thread.h"
#include "mutex.h"
#include "cond_var.h"
#include "mini_list.h"

template<class TOCommand, class TOSubscriber>
class TPRequest{
public:
    typedef unsigned long long uint64_t;
    TPRequest( TOSubscriber tsi, int timeout_ms, const TOCommand &cmd);

    bool happens_before(uint64_t t);

    bool happens_before(const TPRequest &req);

    int get_ms_to_timeout();

    TOCommand get_command();

    TOSubscriber get_subscriber();

    bool operator==(const TPRequest<TOCommand, TOSubscriber> &req);
private:
    TOSubscriber _subscriber;
    uint64_t _when_ms;
    TOCommand _command;
};

template<class TOCommand, class TOSubscriber>
class Timeout_Provider : public Runnable
{
public:
    Timeout_Provider();
    ~Timeout_Provider();
    std::string get_mem_object_type() const;

    std::string get_timeouts();

    std::list<TPRequest<TOCommand, TOSubscriber> > get_timeout_requests();

    void stop_thread();

    void run();

    void request_timeout(int time_ms, TOSubscriber subscriber, const TOCommand &command);

    void cancel_request(TOSubscriber subscriber, const TOCommand &command);

    int get_timeout(TOSubscriber subscriber, const TOCommand &command);

private:
    void wake();

    void sleep(int ms);

    void loop();

    Mini_List<TPRequest<TOCommand, TOSubscriber> > _requests;

    Cond_Var *_wait_cond;

    Mutex _synch_lock;

    Thread *_thread;
    bool _stop;
};

#endif // TIMEOUT_PROVIDER_H
