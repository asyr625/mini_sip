#ifndef SOUND_DRIVER_H
#define SOUND_DRIVER_H

#include <iterator>
#include <vector>

#include "sobject.h"
#include "splugin.h"

#include "sound_device.h"

class Sound_Device_Name
{
public:
    Sound_Device_Name(): max_input_channels(0), max_output_channels(0) {}

    Sound_Device_Name(std::string name, std::string description, int maxInputChannels = 2, int maxOutputChannels = 2 );

    std::string get_name() const;
    std::string get_description() const;

    int get_max_input_channels() const;

    int get_max_output_channels() const;

    int operator ==( const Sound_Device_Name &dev ) const;

private:
    std::string name;
    std::string description;
    int max_input_channels;
    int max_output_channels;
};

class Sound_Driver : public SPlugin
{
public:
    Sound_Driver( std::string driverId, SRef<Library *> lib = NULL );
    virtual ~Sound_Driver();
    virtual SRef<Sound_Device*> create_device( std::string deviceName ) = 0;
    std::string get_id() const;

    virtual std::string get_description() const = 0;

    std::string get_plugin_type() const { return "SoundDriver"; }

    virtual std::vector<Sound_Device_Name> get_device_names() const = 0;

    virtual bool get_default_input_device_name( Sound_Device_Name &name ) const = 0;

    virtual bool get_default_output_device_name( Sound_Device_Name &name ) const = 0;

    int operator == ( const Sound_Driver &driver ) const;
private:
    std::string id;
};

#endif // SOUND_DRIVER_H
