#include "bell.h"

#include<stdlib.h>
#include<stdio.h>
#include<signal.h>
#include<sys/stat.h>
#include<sys/types.h>

#ifdef _MSC_VER
#else
#	include<sys/time.h>
#	include<unistd.h>
#endif

#ifdef IPAQ
#	include <sys/ioctl.h>
#endif

#include<fcntl.h>


#include<iostream>

using namespace std;

Bell::Bell()
{
    running = false;
}
Bell::~Bell()
{
}

void Bell::start()
{
    running = true;
    delay_index = 0;
#ifdef IPAQ
    if ((ipaq_buzzer = open("/dev/misc/buzzer",O_WRONLY))==-1){
#ifdef DEBUG_OUTPUT
        cerr << "Could not open IPAQ buzzer"<< endl;
#endif
        return;
    }
    struct buzzer_time t;
    t.on_time=500;
    t.off_time=1000;
    if (ioctl( ipaq_buzzer, IOC_SETBUZZER, &t)){
#ifdef DEBUG_OUTPUT
        cerr << "Could not start IPAQ buzzer"<< endl;
#endif
    }
#endif
}

void Bell::stop()
{
    running = false;
#ifdef IPAQ
    struct buzzer_time t;
    t.on_time = 0;
    t.off_time = 0;
    if (ipaq_buzzer>0){
        if (ioctl( ipaq_buzzer, IOC_SETBUZZER, &t)){
#ifdef DEBUG_OUTPUT
            cerr << "Could not stop IPAQ buzzer"<< endl;
#endif
        }
        close(ipaq_buzzer);
    }
#endif
}
