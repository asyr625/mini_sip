#include "my_semaphore.h"


#include "my_error.h"
#include "string_utils.h"
#include "dbg.h"

#include<stdlib.h>
#include<stdio.h>
#include<errno.h>

#include<iostream>
using namespace std;

#include<semaphore.h>
#define SEMHANDLE (((sem_t*)(handle_ptr)))


Semaphore_Exception::Semaphore_Exception(std::string w)
    : Exception(w)
{
}

#ifdef DARWIN
static int X=0;
#endif

Semaphore::Semaphore()
{
// Unnamed semaphores are not implemented (although the functions exist)
// on OS X resulting in run-time exception. Use named semaphore instead.
#ifdef DARWIN
    X+=1000000;
    string name="/mutil"+itoa(rand()+X)+itoa(time(NULL));
    printf("EE: creating semaphore <%s>\n", name.c_str() );
    if ( (handle_ptr=sem_open(name.c_str(), O_CREAT | O_EXCL, 0777, 0 ))==SEM_FAILED)
    {
        my_error("Semaphore::Semaphore: sem_open");
        throw Semaphore_Exception("Could not create Semaphore (sem_open())");

    }
#else
    handle_ptr = (void*) new sem_t;
    if (sem_init( SEMHANDLE, 0, 0))
    {
        my_error("Semaphore::Semaphore: sem_init");
        throw Semaphore_Exception("Could not create Semaphore (sem_init())");
    }
#endif
}

Semaphore::~Semaphore()
{
#ifdef DARWIN
    if (sem_close(SEMHANDLE))
    {
        my_error("Semaphore::~Semaphore: sem_close");
    }
#else
    if (sem_destroy(SEMHANDLE))
    {
        my_error("Semaphore::~Semaphore: sem_destroy");
    }
    delete (sem_t*)handle_ptr;
#endif
}

void Semaphore::inc()
{
    if( sem_post( SEMHANDLE ) )
    {
        my_error("Semaphore::inc: sem_post");
        throw Semaphore_Exception("sem_post() failed");
    }
}

void Semaphore::dec()
{
    do{}while(!dec_try(true));
}

bool Semaphore::dec_try(const bool &blockUntilAvailable)
{
    int (*sem_acquire)(sem_t *) = blockUntilAvailable ? sem_wait : sem_trywait;
    if(sem_acquire(SEMHANDLE))
    {
        switch( errno )
        {
            case EINTR:
            case EAGAIN:
                return false;
            default:
                my_error("Semaphore::dec: sem_wait");
                throw Semaphore_Exception("sem_wait() failed");
        }
    }
    return true;
}
