#include "util_defines.h"
#include "thread.h"
#include "mutex.h"

#ifdef DARWIN
#define _XOPEN_SOURCE
#include <mach/mach_traps.h>
#endif

#include <unistd.h>
#include<sys/stat.h>
#include<fstream>

#ifdef HAVE_PTHREAD_H
#include<pthread.h>
#include<signal.h>
#endif	// HAVE_PTHREAD_H

#ifdef HAVE_EXECINFO_H
#include<execinfo.h>
#endif	// HAVE_EXECINFO_H

#include "dbg.h"
#include "my_error.h"
#include "exception.h"
#include "my_assert.h"
#include "my_time.h"

#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<list>
#include<time.h>
#include<sys/time.h>

#include<sys/types.h>
#ifndef DARWIN
#include<linux/unistd.h>
#endif

using namespace std;


#ifdef DEBUG_OUTPUT

pid_t gettid(void )
{
#ifdef DARWIN
    //return mach_thread_self();
    return 0;
#else
    return syscall(__NR_gettid);
#endif
}

class tinfo
{
public:
    uint64_t id;
    pid_t tid;
    string name;
};
static std::list<tinfo> thread_names;
static Mutex thread_names_list_mutex;

void add_thread(uint64_t id, pid_t tid, std::string name="")
{
    my_dbg << "THREAD: adding thread " << id << "ld";
    tinfo inf;
    inf.id = id;
    inf.tid = tid;
    inf.name = name;
    thread_names_list_mutex.lock();
    thread_names.push_back(inf);
    thread_names_list_mutex.unlock();
}

void del_thread(uint64_t id)
{
    my_dbg << "THREAD: removing thread " << id << "ld";
    list<tinfo>::iterator i;
    thread_names_list_mutex.lock();
    for (i = thread_names.begin(); i != thread_names.end(); i++)
    {
        if ((*i).id == id)
        {
            my_dbg << "THREAD: removing id=" << i->id << "ld tid=" << i->tid << " name=" << i->name;
            thread_names.erase(i);
            thread_names_list_mutex.unlock();
            return;
        }
    }
    thread_names_list_mutex.unlock();
    my_err << "THREAD: warning, tried removing thread not in list"<<endl;
}

void cleanup_func(void *arg)
{
    uint64_t id=*((uint64_t *)arg);
    //uint64_t id = pthread_self();
    del_thread(id);
}
#endif


void print_threads()
{
#ifdef DEBUG_OUTPUT
    list<tinfo>::iterator i;
    thread_names_list_mutex.lock();
    for (i=thread_names.begin(); i!=thread_names.end(); i++)
    {
        my_err << "THREAD: "<< (*i).id <<"\t"<<(*i).name<<"\ttid="<<(*i).tid<<endl;
    }
    thread_names_list_mutex.unlock();
#endif
}

void set_thread_name(string descr, uint64_t id)
{
#ifdef DEBUG_OUTPUT
    thread_names_list_mutex.lock();
    if (id==0)
        id=(uint64_t)pthread_self();
    list<tinfo>::iterator i;
    for (i=thread_names.begin(); i!=thread_names.end(); i++)
    {
        if ((*i).id == (uint64_t)pthread_self())
        {
            (*i).name = descr;
            thread_names_list_mutex.unlock();
            return;
        }
    }
    thread_names_list_mutex.unlock();
    add_thread(id, gettid(), descr);
#endif
}

Thread_Exception::Thread_Exception(const char *desc)
    : Exception(desc)
{
}


void start_function( void* (*f)() )
{
#ifdef DEBUG_OUTPUT
    uint64_t *id = new uint64_t(pthread_self());
    add_thread( *id, gettid() );
    pthread_cleanup_push( (cleanup_func) ,(void*)id );
#endif
    try{
        (*f)();
    }catch(Exception &e){
        my_err << "Thread: caught exception "<< flush << e.what()<<endl;
        my_err << "Thread: Stack trace:\n"+e.stack_trace()<<flush;
    }catch(Exception &e){
        my_err << "Thread: caught exception "<< flush << e.what()<<endl;
        my_err << "Thread: Stack trace:\n"+e.stack_trace()<<flush;
    }catch(exception &e){
        my_err << "Thread: caught exception:"<< flush << e.what()<<endl;
    }catch(exception &e){
        my_err << "Thread: caught exception:"<< flush << e.what()<<endl;
    }

#ifdef DEBUG_OUTPUT
    //	delThread( *id );
    pthread_cleanup_pop(1);
    delete id;
#endif
}

void *start_function_arg( void* (*f)(void*), void* arg)
{
#ifdef DEBUG_OUTPUT
    uint64_t *id = new uint64_t(pthread_self());
    add_thread( *id, gettid());
    pthread_cleanup_push( (cleanup_func) ,(void *)id );
#endif
    try{
        (*f)(arg);
    }catch(Exception &e){
        my_err << "Thread: caught exception "<< flush << e.what()<<endl;
        my_err << "Thread: Stack trace:\n"+e.stack_trace()<<flush;
    }catch(Exception &e){
        my_err << "Thread: caught exception "<< flush << e.what()<<endl;
        my_err << "Thread: Stack trace:\n"+e.stack_trace()<<flush;
    }catch(exception &e){
        my_err << "Thread: caught exception:"<< flush << e.what()<<endl;
    }catch(exception &e){
        my_err << "Thread: caught exception:"<< flush << e.what()<<endl;
    }
    return NULL;
#ifdef DEBUG_OUTPUT
    //	delThread( *id );
    pthread_cleanup_pop(1);
    delete id;
#endif
}


void start_runnable(SRef<Runnable*> r)
{
#ifdef DEBUG_OUTPUT
    uint64_t *id = new uint64_t(pthread_self());
    add_thread( *id, gettid() );
    pthread_cleanup_push( (cleanup_func) ,(void*)id );
#endif

    try{
        r->run();
    }catch(Exception &e){
        my_err << "Thread: caught exception "<< flush << e.what()<<endl;
        my_err << "Thread: Stack trace:\n"+e.stack_trace()<<flush;
    }catch(Exception &e){
        my_err << "Thread: caught exception "<< flush << e.what()<<endl;
        my_err << "Thread: Stack trace:\n"+e.stack_trace()<<flush;
    }catch(exception &e){
        my_err << "Thread: caught exception:"<< flush << e.what()<<endl;
    }catch(exception &e){
        my_err << "Thread: caught exception:"<< flush << e.what()<<endl;
    }

#ifdef DEBUG_OUTPUT
    //	delThread( *id );
    pthread_cleanup_pop(1);
    delete id;
#endif
}


typedef struct tmpstruct
{
    void *fun;
    void *arg;
} tmpstruct;

#ifdef HAVE_EXECINFO_H
#include <ucontext.h>
#include <dlfcn.h>

#if defined(REG_RIP)
#define REGFORMAT "%016lx"
#elif defined(REG_EIP)
#define REGFORMAT "%08x"
#else
#define REGFORMAT "%x"
#endif	// defined(REG_RIP)

#ifdef LOGGING_SUPPORT
FILE *crashFilePointer;

static std::string get_file_name()
{
    std::string filename = "MiniSIPCrash-";
    time_t timestamp;
    struct tm * timestampStruct;
    char timestampString[80];

    time(&timestamp);
    timestampStruct = localtime(&timestamp);
    strftime(timestampString, 20, "%s", timestampStruct);
    filename = string("/") + filename + timestampString + string(".report");
    return (filename);
}
#endif

//Thanks to Jaco Kroon for this function
//http://tlug.up.ac.za/wiki/index.php/Obtaining_a_stack_trace_in_C_upon_SIGSEGV
static void signal_handler(int signum, siginfo_t* info, void*ptr)
{

#ifdef LOGGING_SUPPORT
    struct stat st;
    std::string crashDirectoryName = string(getenv("HOME")) + "/.minisip" + "/crash_reports";
    std::string crashFileName = get_file_name();
    pid_t PID;
    char pathname[] = "/home/minisip/.minisip/minisip_crash";
#endif
    static const char *si_codes[3] = {"", "SEGV_MAPERR", "SEGV_ACCERR"};

    int f = 0;
    ucontext_t *ucontext = (ucontext_t*)ptr;
    Dl_info dlinfo;
    void **bp = 0;
    void *ip = 0;

#ifdef LOGGING_SUPPORT
    if(signum == SIGSEGV) { //FIXME: Why only for SIGSEGV?
        //Creating the sub folder within /HOME/.minisip folder (if it does not exist)
        if (stat(crashDirectoryName.c_str(), &st) == 0)
            cerr << "[Signal Handler] The directory is already present" << endl;
        else {
            //Creating a new directory
            if (mkdir(crashDirectoryName.c_str(), 0777) == -1) {
                cerr << "[Signal Handler] Error opening the new directory" << endl;
            }
        }
        //Opening the crash file
        crashFilePointer = fopen((crashDirectoryName + crashFileName).c_str(), "a+");
        if (crashFilePointer == NULL) {
            cerr << "[Signal Handler] Error opening the crash file " << (crashDirectoryName
                                                                         + crashFileName) << endl;
        } else {
            cerr << "[Signal Handler] Crash file opened successfully\n" << (crashDirectoryName
                                                                            + crashFileName) << endl;
        }
    }
#endif

    fprintf(stderr, "EXCEPTION CAUGHT:\n");
    fprintf(stderr, "info.si_signo = %d\n", signum);
    fprintf(stderr, "info.si_errno = %d\n", info->si_errno);
    fprintf(stderr, "info.si_code  = %d (%s)\n", info->si_code, si_codes[info->si_code]);
    fprintf(stderr, "info.si_addr  = %p\n", info->si_addr);
#ifdef LOGGING_SUPPORT
    fprintf(crashFilePointer,"EXCEPTION CAUGHT:\n");
    fprintf(crashFilePointer, "info.si_signo = %d\n", signum);
    fprintf(crashFilePointer, "info.si_errno = %d\n", info->si_errno);
    fprintf(crashFilePointer, "info.si_code  = %d (%s)\n", info->si_code, si_codes[info->si_code]);
    fprintf(crashFilePointer, "info.si_addr  = %p\n", info->si_addr);
#endif
    /* For register content, enable the following two lines:
     for(i = 0; i < NGREG; i++)
     fprintf(stderr, "reg[%02d]       = 0x" REGFORMAT "\n", i, ucontext->uc_mcontext.gregs[i]);
     */
#if defined(REG_RIP)
    ip = (void*)ucontext->uc_mcontext.gregs[REG_RIP];
    bp = (void**)ucontext->uc_mcontext.gregs[REG_RBP];
#elif defined(REG_EIP)
    ip = (void*)ucontext->uc_mcontext.gregs[REG_EIP];
    bp = (void**)ucontext->uc_mcontext.gregs[REG_EBP];
#else
    fprintf(stderr, "Unable to retrieve Instruction Pointer (not printing stack trace).\n");
#define SIGSEGV_NOSTACK
#endif	// defined(REG_RIP)

#ifndef SIGSEGV_NOSTACK
    fprintf(stderr, "Stack trace:\n");
#ifdef LOGGING_SUPPORT
    fprintf(crashFilePointer, "Stack trace:\n");
#endif
    while(bp && ip) {
        if(!dladdr(ip, &dlinfo))
            break;

#if __WORDSIZE == 64
        fprintf(stderr, "% 2d: %p <%s+%lu> (%s)\n",
                ++f,
                ip,
                dlinfo.dli_sname,
                (unsigned long)((unsigned long)ip - (unsigned long)dlinfo.dli_saddr),
                dlinfo.dli_fname);
#ifdef LOGGING_SUPPORT
        fprintf(crashFilePointer, "% 2d: %p <%s+%lu> (%s)\n",
                f,
                ip,
                dlinfo.dli_sname,
                (unsigned long)((unsigned long)ip - (unsigned long)dlinfo.dli_saddr),
                dlinfo.dli_fname);
#endif
#else
        fprintf(stderr, "% 2d: %p <%s+%u> (%s)\n",
                ++f,
                ip,
                dlinfo.dli_sname,
                (unsigned)((unsigned)ip - (unsigned)dlinfo.dli_saddr),
                dlinfo.dli_fname);
#ifdef LOGGING_SUPPORT
        fprintf(crashFilePointer, "% 2d: %p <%s+%u> (%s)\n",
                f,
                ip,
                dlinfo.dli_sname,
                (unsigned)((unsigned)ip - (unsigned)dlinfo.dli_saddr),
                dlinfo.dli_fname);
#endif
#endif
        if(dlinfo.dli_sname && !strcmp(dlinfo.dli_sname, "main"))
            break;

        ip = bp[1];
        bp = (void**)bp[0];
    }
    fprintf(stderr, "End of stack trace\n");
#ifdef LOGGING_SUPPORT
    fprintf(crashFilePointer, "End of stack trace\n");
    fclose(crashFilePointer);

    pid_t  pid;

    pid = fork();
    if (pid == 0){
        system("minisipcrashsender");
    }
#endif
#endif	// SIGSEGV_NOSTACK
}

#ifndef SA_ONESHOT
#define SA_ONESHOT SA_RESETHAND
#endif

static bool handle_signal(int sig)
{
    struct sigaction sa;
    sa.sa_handler = NULL;
    sa.sa_sigaction =  signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_ONESHOT | SA_SIGINFO;
    return sigaction(sig, &sa, NULL) == 0;
}

#endif	// HAVE_EXECINFO_H



#ifdef HAVE_EXECINFO_H
/**
 *
 *
 */
#define SIGNAL_HANDLER_DECLARED
void setup_default_signal_handling()
{
    if (!handle_signal(SIGSEGV)){
        my_err << "Thread: Could not install stack trace output for the SIGSEGV signal"<<endl;
    }
    if (!handle_signal(SIGBUS)){
        my_err << "Thread: Could not install stack trace output for the SIGSEGV signal"<<endl;
    }
    if (!handle_signal(SIGFPE)){
        my_err << "Thread: Could not install stack trace output for the SIGSEGV signal"<<endl;
    }
    if (!handle_signal(SIGILL)){
        my_err << "Thread: Could not install stack trace output for the SIGSEGV signal"<<endl;
    }
    if (!handle_signal(SIGSYS)){
        my_err << "Thread: Could not install stack trace output for the SIGSEGV signal"<<endl;
    }
}
#endif	// HAVE_EXECINFO_H

void *linux_thread_starter(void *arg)
{
#ifdef HAVE_EXECINFO_H
    setup_default_signal_handling();
#endif	// HAVE_EXECINFO_H
    /* Keep a reference to yourself as long as you run */
    SRef<Runnable *> self = *(static_cast<SRef <Runnable *> *>(arg));
    delete (static_cast<SRef <Runnable *> *>(arg));
#ifdef DEBUG_OUTPUT
    my_dbg << "LinuxThreadStarter: thread created"<< endl;
#endif	// DEBUG_OUTPUT

    start_runnable(self);
    //self->run();

#ifdef DEBUG_OUTPUT
    my_dbg <<"LinuxThreadStarter: thread terminated"<< endl;
#endif	// DEBUG_OUTPUT
    return NULL;
    //pthread_exit( (void *) 0 ); //cesc
}

void *linux_static_thread_starter(void *arg)
{
#ifdef HAVE_EXECINFO_H
    setup_default_signal_handling();
#endif	// HAVE_EXECINFO_H
#ifdef DEBUG_OUTPUT
    my_dbg << "LinuxStaticThreadStarter: thread created"<< endl;
#endif
    void* (*f)();
    f=(void* (*)())arg;
    //(*f)();
    start_function(f);
#ifdef DEBUG_OUTPUT
    my_dbg <<"LinuxStaticThreadStarter: thread terminated"<< endl;
#endif	// DEBUG_OUTPUT
    return NULL;
}

void *linux_static_thread_starter_arg(void *arg)
{
#ifdef HAVE_EXECINFO_H
    setup_default_signal_handling();
#endif	// HAVE_EXECINFO_H
#ifdef DEBUG_OUTPUT
    my_dbg << "LinuxStaticThreadStarter: thread created"<< endl;
#endif	// DEBUG_OUTPUT
    tmpstruct *tmp = (tmpstruct*)arg;
    void* (*f)(void*);
    f=(void* (*)(void*)) tmp->fun;
    void *argptr = tmp->arg;
    delete tmp;
    start_function_arg(f, argptr);

#ifdef DEBUG_OUTPUT
    my_dbg <<"LinuxStaticThreadStarter: thread terminated"<< endl;
#endif	// DEBUG_OUTPUT
    return NULL;
}

#ifndef SIGNAL_HANDLER_DECLARED
void setup_default_signal_handling()
{
#ifdef DEBUG_OUTPUT
    my_err << "libmutil: setup_default_signal_handling: No stack trace signal handler available"<<endl;
#endif	// DEBUG_OUTPUT
}
#endif	// SIGNAL_HANDLER_DECLARED


Thread::Thread(SRef<Runnable *> runnable, const Priority &_priority)
{
    my_assert(runnable);
    SRef<Runnable *> *self = new SRef<Runnable *>(runnable);

    my_assert(sizeof(pthread_t) <= 8);
    //handle_ptr = malloc(sizeof(pthread_t));
    //handle_ptr = new pthread_t;
    int ret;

    //set attribs for the threads ...
    /*
    pthread_attr_t attr;
    pthread_attr_init( &attr);
    pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );
    //pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL );
    //pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL );
*/
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if(_priority != Normal_Priority)
    {
        sched_param scheduleParameter;
        scheduleParameter.__sched_priority = sched_get_priority_min(SCHED_RR);
        if(_priority == High_Priority)
            scheduleParameter.__sched_priority = (sched_get_priority_max(SCHED_RR) + scheduleParameter.__sched_priority) / 2;
        if(pthread_attr_setschedpolicy(&attr, SCHED_RR) != 0 || pthread_attr_setschedparam(&attr, &scheduleParameter) != 0)
        {
            my_error("Thread::Thread: pthread_attr_setschedpolicy");
#ifdef DEBUG_OUTPUT
            my_err << "In Thread, linux part - unable to set thread priority" << endl;
#endif	// DEBUG_OUTPUT
        }
    }

    ret = pthread_create(
                //(pthread_t*)handle.hptr,
                (pthread_t*)((void*)&handle.handle),
                &attr, // either NULL or &attr,
                linux_thread_starter,
                self);

    pthread_attr_destroy(&attr);

    // 	pthread_attr_destroy( &attr );

    if ( ret != 0 )
    {
        my_error("Thread::Thread: pthread_create");
        delete self;
#ifdef DEBUG_OUTPUT
        my_err << "In Thread, linux part - thread NOT created" << endl;
#endif	// DEBUG_OUTPUT
        throw Thread_Exception("Could not create thread.");
    }
#ifdef DEBUG_OUTPUT
    my_dbg << "In Thread, linux part - thread created" << endl;
#endif	// DEBUG_OUTPUT
}

Thread::~Thread(){
    //	if( handle_ptr ){
    //		delete (pthread_t *)handle_ptr;
    //	}
    //	handle_ptr = NULL;
}


Thread_Handle Thread::create_thread(void f(), const Priority &_priority)
{
    //pthread_t threadHandle;
    Thread_Handle h;
#ifdef DEBUG_OUTPUT
    my_dbg << "Running createThread"<< endl;
#endif	// DEBUG_OUTPUT

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if(_priority != Normal_Priority)
    {
        sched_param scheduleParameter;
        scheduleParameter.__sched_priority = sched_get_priority_min(SCHED_RR);
        if(_priority == High_Priority)
            scheduleParameter.__sched_priority = (sched_get_priority_max(SCHED_RR) + scheduleParameter.__sched_priority) / 2;
        if(pthread_attr_setschedpolicy(&attr, SCHED_RR) != 0 || pthread_attr_setschedparam(&attr, &scheduleParameter) != 0)
        {
            my_error("Thread::Thread: pthread_attr_setschedpolicy");
#ifdef DEBUG_OUTPUT
            my_err << "In Thread, linux part - unable to set thread priority" << endl;
#endif	// DEBUG_OUTPUT
        }
    }
    pthread_create( /* (pthread_t*)h.hptr*/ (pthread_t*)((void*)&h.handle), &attr, linux_static_thread_starter, (void*)f);

    pthread_attr_destroy(&attr);
    return h;
}

Thread_Handle Thread::create_thread(void *f(void*), void *arg, const Priority &_priority)
{
    tmpstruct *argptr = new struct tmpstruct;
    argptr->fun = (void*)f;
    argptr->arg = arg;

    Thread_Handle h;
#ifdef DEBUG_OUTPUT
    my_dbg << "Running createThread" << endl;
#endif	// DEBUG_OUTPUT

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if(_priority != Normal_Priority)
    {
        sched_param scheduleParameter;
        scheduleParameter.__sched_priority = sched_get_priority_min(SCHED_RR);
        if(_priority == High_Priority)
            scheduleParameter.__sched_priority = (sched_get_priority_max(SCHED_RR) + scheduleParameter.__sched_priority) / 2;
        if(pthread_attr_setschedpolicy(&attr, SCHED_RR) != 0 || pthread_attr_setschedparam(&attr, &scheduleParameter) != 0)
        {
            my_error("Thread::Thread: pthread_attr_setschedpolicy");
#ifdef DEBUG_OUTPUT
            my_err << "In Thread, linux part - unable to set thread priority" << endl;
#endif	// DEBUG_OUTPUT
        }
    }
    pthread_create(/* (pthread_t*)h.hptr*/ (pthread_t*)((void*)&h.handle), &attr, linux_static_thread_starter_arg, argptr);

    pthread_attr_destroy(&attr);
    return h;
}

void * Thread::join()
{
    void * returnValue;
    int ret;

#ifdef DEBUG_OUTPUT
    my_dbg << "Thread::join(): before join" << endl;
#endif	// DEBUG_OUTPUT
    ret = pthread_join(
                //*((pthread_t *)handle.hptr),
                (pthread_t)handle.handle,
                &returnValue );

    if( ret != 0 ){
#ifdef DEBUG_OUTPUT
        my_error("Thread::join: pthread_join");
#endif	// DEBUG_OUTPUT
        return NULL;
    }
    return returnValue;
}

void Thread::join(const Thread_Handle &h)
{
    if( pthread_join( /* *((pthread_t*)h.hptr)*/ (pthread_t)h.handle, NULL) ){
#ifdef DEBUG_OUTPUT
        my_error("Thread::join: pthread_join");
#endif
    }
}

int Thread::msleep(int32_t ms)
{
    return ::my_sleep( ms );
}


bool Thread::kill( )
{
    int ret;

#ifdef DEBUG_OUTPUT
    my_dbg << "Thread::kill(): before cancel" << endl;
#endif	// DEBUG_OUTPUT
    //ret = pthread_cancel( *( (pthread_t *)handle) );
    ret = pthread_cancel( /* *( (pthread_t *)handle.hptr)*/ (pthread_t)handle.handle );

    if( ret != 0 ){
#ifdef DEBUG_OUTPUT
        my_err << "Thread::kill(): ERROR" << endl;
#endif	// DEBUG_OUTPUT
        return false;
    }
    return true;
}

bool Thread::kill( const Thread_Handle &h)
{
    int ret;

#ifdef DEBUG_OUTPUT
    my_dbg << "Thread::kill(): before cancel" << endl;
#endif	// DEBUG_OUTPUT
    ret = pthread_cancel( /* *((pthread_t*)h.hptr) */ (pthread_t)h.handle );

    if( ret != 0 )
    {
#ifdef DEBUG_OUTPUT
        my_err << "Thread::kill(): ERROR" << endl;
#endif	// DEBUG_OUTPUT
        return false;
    }
    return true;
}

Thread_Handle Thread::get_current()
{
    Thread_Handle th;
    //*((pthread_t*)th.hptr) = pthread_self();
    th.handle = (uint64_t)pthread_self();
    return th;
}

Thread_Handle::Thread_Handle()
{
    //hptr = (void*)new pthread_t;
    handle = 0;
}

Thread_Handle::~Thread_Handle()
{
    //	delete (pthread_t*)hptr;
    //	hptr=NULL;
}

Thread_Handle::Thread_Handle(const Thread_Handle &h)
{
    //hptr = (void*)new pthread_t;
    //*((pthread_t*)hptr)= *((pthread_t*)h.hptr);
    handle = h.handle;
}

