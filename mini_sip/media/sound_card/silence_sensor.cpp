#include "silence_sensor.h"

static int iabs(int i)
{
    if (i<0)
        return -i;
    return i;
}

static int energy(uint16_t *buf, int n)
{
    long ret=0;
    for (int i=0; i< n; i++)
        ret+=iabs(buf[i]);
    return ret/n;
}

Simple_Silence_Sensor::Simple_Silence_Sensor()
    : in_silence(false), noice_level(0.0), limit_on(15), limit_off(7)
{
}

bool Simple_Silence_Sensor::silence(uint16_t *buf, int n)
{
    int e = energy(buf, n);

    if (in_silence && e >= limit_on)
        in_silence=false;

    if (!in_silence && e<=limit_off)
        in_silence = true;

    return in_silence;
}
