#include "alsa_sound_driver.h"

#include "sound_driver.h"
#include "sound_driver_registry.h"
#include "splugin.h"
#include "dbg.h"

#include "alsa_sound_device.h"

static const char DRIVER_PREFIX[] = "alsa";
static std::list<std::string> pluginList;
static int initialized;

extern "C"
std::list<std::string> *malsa_LTX_listPlugins( SRef<Library*> lib )
{
    if( !initialized )
    {
        pluginList.push_back("getPlugin");
        initialized = true;
    }
    return &pluginList;
}

extern "C"
SPlugin * malsa_LTX_getPlugin( SRef<Library*> lib )
{
    return new Alsa_Sound_Driver( lib );
}

Alsa_Sound_Driver::Alsa_Sound_Driver( SRef<Library*> lib )
    : Sound_Driver( DRIVER_PREFIX, lib )
{
}

Alsa_Sound_Driver::~Alsa_Sound_Driver()
{
}

SRef<Sound_Device*> Alsa_Sound_Driver::create_device( std::string deviceId )
{
    return new Alsa_Sound_Device( deviceId );
}

std::vector<Sound_Device_Name> Alsa_Sound_Driver::get_device_names() const
{
    std::vector<Sound_Device_Name> names;

    my_dbg << "AlsaSoundDriver::getDeviceNames not implemented" << std::endl;

    return names;
}

bool Alsa_Sound_Driver::get_default_input_device_name( Sound_Device_Name &name ) const
{
    name = Sound_Device_Name( "alsa:default", "Default input device" );
    return true;
}

bool Alsa_Sound_Driver::get_default_output_device_name( Sound_Device_Name &name ) const
{
    name = Sound_Device_Name( "alsa:default", "Default output device" );
    return true;
}

uint32_t Alsa_Sound_Driver::get_version() const
{
    return 0x00000001;
}
