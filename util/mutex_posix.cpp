#include "mutex.h"

#include<stdio.h>
#include<stdlib.h>
#include "my_assert.h"

// BSD 5.x: malloc.h has been replaced by stdlib.h
// #include<malloc.h>

#if HAVE_PTHREAD_H
#include<pthread.h>
#endif

#include<iostream>
using namespace std;

///See: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dllproc/base/mutex_objects.asp

//TODO: Check return values
Mutex::Mutex()
{
    create_mutex();
    my_assert(handle_ptr);
}


//FIXME: Verify and comment this method!
Mutex& Mutex::operator=(const Mutex &)
{
    //Do not copy the Mutex reference - keep our own.
    return *this;
}

Mutex::Mutex(const Mutex &)
{
    create_mutex();
    my_assert(handle_ptr);
}

void Mutex::create_mutex()
{

    pthread_mutexattr_t *attr = NULL;

#ifdef DEBUG_OUTPUT
    pthread_mutexattr_t errorCheck;
    pthread_mutexattr_init( &errorCheck );
    pthread_mutexattr_settype( &errorCheck, PTHREAD_MUTEX_ERRORCHECK );
    attr = &errorCheck;
#endif

    handle_ptr = new pthread_mutex_t;
    int ret = pthread_mutex_init( (pthread_mutex_t*)handle_ptr, attr );
    my_assert(ret==0);


#ifdef DEBUG_OUTPUT
    pthread_mutexattr_destroy( &errorCheck );
#endif
}

Mutex::~Mutex()
{
    my_assert(handle_ptr);
    if(pthread_mutex_destroy((pthread_mutex_t*)handle_ptr) == 0)
    delete (pthread_mutex_t*)handle_ptr;
}

void Mutex::lock()
{
    int ret;
    my_assert(handle_ptr);
    ret = pthread_mutex_lock((pthread_mutex_t*)handle_ptr);
    my_assert( ret == 0 );
}

void Mutex::unlock()
{
    int ret;
    my_assert(handle_ptr);
    ret = pthread_mutex_unlock((pthread_mutex_t*)handle_ptr);
    my_assert( ret == 0 );
}
