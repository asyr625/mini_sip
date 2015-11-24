#ifndef SIP_H
#define SIP_H

#include "thread.h"
#include "subsystem_media.h"
#include "sip_dialog.h"

class Sip : public Runnable
{
public:
    Sip(SRef<Sip_Configuration*> config,
        SRef<Subsystem_Media*> subsystem_media);

    virtual ~Sip();

    virtual std::string get_mem_object_type() const {return "Sip";}

    SRef<Sip_Configuration*> get_config() { return _config; }

    bool start();
    void stop();

    void join();
    virtual void run();

    SRef<Sip_Stack*> get_sip_stack() { return _sip_stack; }

    void set_media_handler(SRef<Subsystem_Media*> subsystem_media);

private:
    SRef<Sip_Stack *> _sip_stack;
    SRef<Sip_Configuration*> _config;
    SRef<Subsystem_Media *> _subsystem_media;

    SRef<Thread *> _thread;
};

#endif // SIP_H
