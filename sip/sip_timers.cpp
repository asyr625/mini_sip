#include "sip_timers.h"

#define _T1 500
#define _T4 5000

Sip_Timers::Sip_Timers()
{
    T1 = _T1;
    T2 = 4000;
    T4 = _T4;
    A = _T1; //=T1
    B = 64 * _T1;
    C = 4*60*1000; //4min (should be >3min)
    D = 60000; // 60s (should be >32s)
    E = _T1;
    F = 64 * _T1;
    G = _T1;
    H = 64*_T1;
    I = _T4;
    J = 64*_T1;
    K = _T4;
}
