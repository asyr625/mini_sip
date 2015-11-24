#ifndef SOUND_IO_PLC_INTERFACE_H
#define SOUND_IO_PLC_INTERFACE_H

#include "my_types.h"

class Sound_IO_PLC_Interface
{
public:
    virtual short *get_plc_sound(uint32_t &ret_size) = 0;
    virtual ~Sound_IO_PLC_Interface() {}
};

#endif // SOUND_IO_PLC_INTERFACE_H
