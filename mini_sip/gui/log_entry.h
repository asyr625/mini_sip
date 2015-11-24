#ifndef LOG_ENTRY_H
#define LOG_ENTRY_H

#include <string>
#include "sobject.h"

class Log_Entry_Handler;

class Log_Entry : public SObject
{
public:
    Log_Entry();

    virtual std::string get_mem_object_type() const { return "Log_Entry"; }

    int type;
    std::string peer_sip_uri;
    time_t start;
    static SRef<Log_Entry_Handler *> handler;
    void handle();
};

class Log_Entry_Success : public Log_Entry
{
public:
    time_t duration;
    bool secured;
    int mos;
};

class Log_Entry_Failure : public Log_Entry
{
public:
    std::string error;
};

class  Log_Entry_Incoming{};
class  Log_Entry_Outgoing{};

class Log_Entry_Missed_Call:public Log_Entry_Failure, public Log_Entry_Incoming
{
};

class Log_Entry_Call_Rejected : public Log_Entry_Failure, public Log_Entry_Outgoing
{
};

class  Log_Entry_Incoming_Completed_Call : public Log_Entry_Incoming, public Log_Entry_Success
{
};

class Log_Entry_Outgoing_Completed_Call : public Log_Entry_Outgoing, public Log_Entry_Success
{
};

class  Log_Entry_Handler : public virtual SObject
{
public:
    virtual void handle( SRef<Log_Entry *> ) = 0;

    virtual std::string get_mem_object_type() const { return "LogEntryHandler";}
};

#endif // LOG_ENTRY_H
