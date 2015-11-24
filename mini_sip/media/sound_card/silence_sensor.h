#ifndef SILENCE_SENSOR_H
#define SILENCE_SENSOR_H

#include "my_types.h"

class Silence_Sensor
{
public:
    ~Silence_Sensor() {}
    virtual bool silence(uint16_t *buf, int n) = 0;
};

class Simple_Silence_Sensor : public Silence_Sensor
{
public:
    Simple_Silence_Sensor();
    virtual bool silence(uint16_t *buf, int n);

private:
    bool in_silence;
    float noice_level;
    int limit_on;
    int limit_off;
};

#endif // SILENCE_SENSOR_H
