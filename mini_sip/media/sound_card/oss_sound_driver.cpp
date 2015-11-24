#include "oss_sound_driver.h"

#include "sound_driver.h"
#include "sound_driver_registry.h"
#include "splugin.h"
#include "dbg.h"

#include "oss_sound_device.h"

using namespace std;

static const char DRIVER_PREFIX[] = "oss";
static std::list<std::string> pluginList;
static int initialized;

extern "C"
std::list<std::string> *moss_LTX_listPlugins( SRef<Library*> lib )
{
    if( !initialized )
    {
        pluginList.push_back("getPlugin");
        initialized = true;
    }
    return &pluginList;
}

extern "C"
SPlugin * moss_LTX_getPlugin( SRef<Library*> lib )
{
    return new OssSoundDriver( lib );
}


Oss_Sound_Driver::Oss_Sound_Driver( SRef<Library*> lib )
    : Sound_Driver( DRIVER_PREFIX, lib )
{

}

Oss_Sound_Driver::~Oss_Sound_Driver()
{
}

SRef<Sound_Device*> Oss_Sound_Driver::create_device( std::string deviceId )
{
    return new Oss_Sound_Device( deviceId );
}

std::vector<Sound_Device_Name> Oss_Sound_Driver::get_device_names() const
{
    std::vector<Sound_Device_Name> names;

    my_dbg << "Oss_Sound_Driver::get_device_names not implemented" << endl;
    return names;
}

bool Oss_Sound_Driver::get_default_input_device_name( Sound_Device_Name &name ) const
{
    name = Sound_Device_Name( "/dev/dsp", "/dev/dsp" );
    return true;
}

bool Oss_Sound_Driver::get_default_output_device_name( Sound_Device_Name &name ) const
{
    name = Sound_Device_Name( "/dev/dsp", "/dev/dsp" );
    return true;
}

uint32_t Oss_Sound_Driver::get_version() const
{
    return 0x00000001;
}
