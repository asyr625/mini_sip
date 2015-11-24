#ifndef THREAD_H
#define THREAD_H

#include "sobject.h"
#include "exception.h"

class Thread_Exception : public Exception
{
public:
    Thread_Exception(const char* description);
};

class Runnable : public virtual SObject
{
public:
    virtual void run() = 0;
};

class Thread_Handle
{
public:
    Thread_Handle();
    Thread_Handle(const Thread_Handle&);
    ~Thread_Handle();

    unsigned long as_longint() const
    {
        return handle;
    }
private:
    unsigned long handle;
    friend class Thread;
};

class Thread : public SObject
{
public:
    enum Priority { Normal_Priority, Above_Normal_Priority, High_Priority };


    Thread(SRef<Runnable*> runnable, const Priority& priority);

    ~Thread();

    std::string get_mem_object_type() const { return "Thread"; }

    static Thread_Handle create_thread(void f(), const Priority& priority);

    static Thread_Handle create_thread(void* f(void*),void* arg, const Priority& priority);

    void* join();

    static void join(const Thread_Handle& handle);

    bool kill();

    static bool kill(const Thread_Handle& handle);

    Thread_Handle get_handle() { return handle;}

    static Thread_Handle get_current();

    static int msleep(int msec);

private:
    Thread_Handle handle;
};

void setup_default_signal_handling();

void set_thread_name(std::string descr, unsigned long tid = 0);

void print_threads();

#endif // THREAD_H
