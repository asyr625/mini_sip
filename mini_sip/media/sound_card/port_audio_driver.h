#ifndef PORT_AUDIO_DRIVER_H
#define PORT_AUDIO_DRIVER_H

#include "sound_driver.h"

class Port_Audio_Driver : public Sound_Driver
{
public:
    Port_Audio_Driver( SRef<Library*> lib );
    virtual ~Port_Audio_Driver();
    virtual SRef<Sound_Device*> create_device( std::string deviceId );
    virtual std::string get_description() const { return "PortAudio sound driver"; }

    virtual std::vector<Sound_Device_Name> get_device_names() const;

    virtual bool get_default_input_device_name( Sound_Device_Name &name ) const;

    virtual bool get_default_output_device_name( Sound_Device_Name &name ) const;

    virtual std::string get_name() const { return "PortAudio"; }

    virtual std::string get_mem_object_type() const { return get_name(); }

    virtual uint32_t get_version() const;
};

#endif // PORT_AUDIO_DRIVER_H
