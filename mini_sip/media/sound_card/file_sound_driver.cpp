#include "file_sound_driver.h"

#include "sound_driver.h"
#include "sound_driver_registry.h"
#include "splugin.h"

#include "file_sound_device.h"

using namespace std;

static const char DRIVER_PREFIX[] = "file";

File_Sound_Driver::File_Sound_Driver( SRef<Library*> lib )
    : Sound_Driver( DRIVER_PREFIX, lib )
{
}

File_Sound_Driver::~File_Sound_Driver()
{
}

SRef<Sound_Device*> File_Sound_Driver::create_device( std::string deviceId )
{
    return new File_Sound_Device( deviceId, deviceId, FILESOUND_TYPE_RAW );
}


std::vector<Sound_Device_Name> File_Sound_Driver::get_device_names() const
{
    std::vector<Sound_Device_Name> names;

    return names;
}

bool File_Sound_Driver::get_default_input_device_name( Sound_Device_Name &name ) const
{
}

bool File_Sound_Driver::get_default_output_device_name( Sound_Device_Name &name ) const
{
}


uint32_t File_Sound_Driver::get_version() const
{
    return 0x00000001;
}
