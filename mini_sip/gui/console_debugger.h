#ifndef CONSOLE_DEBUGGER_H
#define CONSOLE_DEBUGGER_H

#include "sobject.h"
#include "thread.h"
#include "sip_configuration.h"
#include "subsystem_media.h"

class Console_Debugger : public Runnable
{
public:
    Console_Debugger(SRef<Sip_Configuration*> conf);
    ~Console_Debugger();

    std::string get_mem_object_type() const { return "ConsoleDebugger"; }

    void show_help();
    void show_mem();
    void show_mem_summary();
    void show_stat();
    void show_config();

    void send_management_command( std::string str );
    void send_command_to_media_handler( std::string str );

    SRef<Thread *> start();

    virtual void run();

    void stop();

    void join();

    void set_media_handler( SRef<Subsystem_Media*> r )
    {
        media_handler = r;
    }

private:
    SRef<Sip_Stack*> sip_stack;
    SRef<Subsystem_Media *> media_handler;

    SRef<Thread *> thread;
    bool keep_running;
    SRef<Sip_Configuration*> config;
};

#endif // CONSOLE_DEBUGGER_H
