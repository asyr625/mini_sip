#include "cond_var.h"
#include "util_defines.h"

#include "mutex.h"
#include "my_error.h"
#include "my_time.h"
#include "dbg.h"

#include <iostream>
using namespace std;

#if defined HAVE_PTHREAD_H
#include<pthread.h>
#include<sys/time.h>
#include<time.h>

#define INTERNAL_COND_WAIT ((pthread_cond_t *)internal_struct)
#define INTERNAL_MUTEX ((pthread_mutex_t *)internalMutexStruct)

#endif

Cond_Var::Cond_Var()
    : deleted(false), waiting_threads_count(0)
{
    current_mutex = condvar_mutex = new Mutex;
    internal_struct = new pthread_cond_t;

    pthread_cond_init( INTERNAL_COND_WAIT, NULL );
}

Cond_Var::~Cond_Var()
{
    condvar_mutex->lock();
    deleted = true;
    pthread_cond_broadcast( INTERNAL_COND_WAIT );
    condvar_mutex->unlock();
    if(pthread_cond_destroy( INTERNAL_COND_WAIT ) != 0)
        my_err << "CondVar::~CondVar() unsuccessfully called pthread_cond_destroy" << std::endl;

    // assure that no threads are using this object
    timespec requestedTime;
    requestedTime.tv_nsec = 20000; // 0.02 sec.
    condvar_mutex->lock();
    while(waiting_threads_count > 0)
    {
        condvar_mutex->unlock();
        //my_dbg << "CondVar::~CondVar() sleeping 20ms, because object is being used" << std::endl;
        nanosleep(&requestedTime, NULL);
        condvar_mutex->lock();
    }
    condvar_mutex->unlock();

    delete INTERNAL_COND_WAIT;
    delete condvar_mutex;
}

std::string Cond_Var::get_mem_object_type() const
{
    return "CondVar";
}

void Cond_Var::wait(Cond_Var &variable, bool &deleted, const uint32_t &timeout_ms)
{
    variable.condvar_mutex->lock();
    wait(variable, *variable.condvar_mutex, deleted, timeout_ms);
    variable.condvar_mutex->unlock();
}

void Cond_Var::wait(Cond_Var &variable, Mutex &mutex, bool &deleted, const uint32_t &timeout_ms)
{
    variable.wait(mutex, deleted, timeout_ms);
}

void Cond_Var::wait( Mutex &mutex, bool &deleted, const uint32_t &timeout )
{
    current_mutex = &mutex;
    ++waiting_threads_count;
    if( timeout == 0 )
    {
        pthread_cond_wait( INTERNAL_COND_WAIT, (pthread_mutex_t*)(mutex.handle_ptr) );
    }
    else
    {
        struct timeval now;
        struct timespec ts;
        gettimeofday(&now, NULL);
        ts.tv_sec = now.tv_sec;
        ts.tv_nsec = now.tv_usec*1000;

        ts.tv_sec += timeout / 1000;

        ts.tv_nsec += timeout % 1000 * 1000000;

        if (ts.tv_nsec > 999999999){
            ts.tv_sec++;
            ts.tv_nsec = ts.tv_nsec % 1000000000;
        }

        pthread_cond_timedwait( INTERNAL_COND_WAIT, (pthread_mutex_t*)mutex.handle_ptr, &ts );
    }
    --waiting_threads_count;
    deleted = this->deleted;
}

void Cond_Var::broadcast()
{
  /*
   * in order to make this work properly
   * this method should be made to use currentMutex instead,
   * however this makes minisip to panic on close,
   * probably because mutex, that was used to execute wait, was prematurely deleted.
   */
    condvar_mutex->lock();
    pthread_cond_broadcast( INTERNAL_COND_WAIT );
    condvar_mutex->unlock();
}
