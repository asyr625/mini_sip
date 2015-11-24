#include "my_time.h"
#include<time.h>

#include "my_types.h"
#include "string_utils.h"

uint64_t startTime=0;

unsigned long long utime(const bool &start_at_epoch)
{
    struct timespec current_timespec;
    clock_gettime(start_at_epoch ? CLOCK_REALTIME : CLOCK_MONOTONIC, &current_timespec);
    return ((uint64_t)current_timespec.tv_sec) * (uint64_t)1000000 + ((uint64_t)current_timespec.tv_nsec) / (uint64_t)1000;
}

unsigned long long my_time(const bool &start_at_epoch)
{
    return utime(start_at_epoch) / 1000;
}

int my_sleep(int ms)
{
    struct timespec request;
    request.tv_sec = ms/1000;
    request.tv_nsec = (long) (ms%1000) * 1000 * 1000;
    return nanosleep( &request, NULL );
}


std::string now_str()
{
    if (startTime==0)
        startTime = my_time();
    uint64_t t;
    t=my_time();
    int64_t sec = t / 1000 - startTime / 1000;
    int64_t msec = t - startTime;
    msec = msec%1000;

    std::string header = (sec<100?std::string("0"):std::string("")) +
            (sec<10?"0":"") +
            itoa((int)sec)+
            ":"+
            (msec<10?"0":"")+
            (msec<100?"0":"")+
            itoa((int)msec);

    return header;

}
