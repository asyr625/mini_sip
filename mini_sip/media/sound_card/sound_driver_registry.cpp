#include "sound_driver_registry.h"

#include<iostream>
#include<algorithm>

using namespace std;

#include "dbg.h"
#include "file_sound_driver.h"

#ifdef _MSC_VER
#include "direct_sound_device.h"
#endif

#ifdef ENABLE_OSS
#include "oss_sound_driver.h"
#endif


#ifdef DEBUG_OUTPUT
#if 0
static void dump_all_names( SRef<Sound_Driver_Registry*> instance )
{
    std::vector<Sound_Device_Name> names = instance->get_all_device_names();

    std::vector<Sound_Device_Name>::iterator iter;
    std::vector<Sound_Device_Name>::iterator stop = names.end();

    my_dbg << "Dumping sound device names:" << endl;
    for( iter = names.begin(); iter != stop; iter++ )
    {
        my_dbg << iter->get_name() << " " << iter->get_description() << " in:" <<
                  iter->get_max_input_channels() << ", out:" << iter->get_max_output_channels() << endl;
    }
}
#endif
#endif


Sound_Driver_Registry::Sound_Driver_Registry()
{
    register_plugin( new File_Sound_Driver( NULL ) );
#ifdef _MSC_VER
    register_plugin( new DirectSoundDriver(NULL) );
#endif
#ifdef ENABLE_OSS
    register_plugin( new Direct_Sound_Driver( NULL ) );
#endif
}


const std::vector<SRef<Sound_Driver*> > &Sound_Driver_Registry::get_drivers() const
{
    return drivers;
}

std::vector<Sound_Device_Name> Sound_Driver_Registry::get_all_device_names() const
{
    std::vector<Sound_Device_Name> allNames;

    std::vector< SRef<Sound_Driver*> >::const_iterator iter;
    std::vector< SRef<Sound_Driver*> >::const_iterator end = drivers.end();

    for( iter = drivers.begin(); iter != end; iter++ )
    {
        SRef<Sound_Driver*> driver = *iter;
        std::vector<Sound_Device_Name> names = driver->get_device_names();

        allNames.insert( allNames.end(), names.begin(), names.end() );
    }
    return allNames;
}

SRef<Sound_Driver*> Sound_Driver_Registry::get_default_driver() const
{
    const char *driverPriority[] =
        { "PortAudio", "DirectSound", "AlsaSound", "OssSound", NULL };
    int i;

    for( i = 0;; i++ )
    {
        const char *name = driverPriority[i];

        if( !name )
            return NULL;

        SRef<SPlugin*> plugin = find_plugin( name );
        if( plugin )
        {
            SRef<Sound_Driver*> driver;
            driver = dynamic_cast<Sound_Driver*>(*plugin);
            if( driver )
                return driver;
        }
    }
    return NULL;
}

SRef<Sound_Device*> Sound_Driver_Registry::create_device( std::string deviceName )
{
    std::string driverId;
    std::string deviceId;

#ifdef DEBUG_OUTPUT
    my_dbg << "Sound_Driver_Registry: deviceName =  " << deviceName << endl;
#endif
    size_t pos = deviceName.find( ':', 0 );
    if( pos == string::npos )
    {
        driverId = "oss";
        deviceId = deviceName;
    }
    else
    {
        driverId = deviceName.substr( 0, pos );
        deviceId = deviceName.substr( pos + 1 );
    }
#ifdef DEBUG_OUTPUT
    my_dbg << "Sound_Driver_Registry: deviceId =  " << deviceId << endl;
    my_dbg << "Sound_Driver_Registry: driverId =  " << driverId << endl;
#endif

    std::vector< SRef<Sound_Driver*> >::iterator iter;
    std::vector< SRef<Sound_Driver*> >::iterator stop = drivers.end();

    for( iter = drivers.begin(); iter != stop; iter++ )
    {
        SRef<Sound_Driver*> driver = *iter;

        if( driver->get_id() == driverId )
        {
            my_dbg << "Sound_Driver_Registry: device id found!!! =  " << deviceId << endl;
            return driver->create_device( deviceId );
        }
    }

    my_dbg << "Sound_Driver_Registry: device not found " << deviceName << endl;
    return NULL;
}

void Sound_Driver_Registry::register_plugin( SRef<SPlugin*> plugin )
{
    SPlugin_Registry::register_plugin( plugin );

    SRef<Sound_Driver *> driver = dynamic_cast<Sound_Driver*>(*plugin);

    if( driver )
    {
        register_driver( driver );
    }
    else
        my_err << "Not SoundDriver!" << endl;
}

bool Sound_Driver_Registry::register_driver( SRef<Sound_Driver*> driver )
{
    std::vector< SRef<Sound_Driver*> >::iterator iter;

    iter = find( drivers.begin(), drivers.end(), driver );

    if ( iter != drivers.end() )
    {
        my_err << "Sound_Driver_Registry register_driver: Driver already registered: " << driver->get_id() << endl;
        return false;
    }

    my_dbg << "Sound_Driver_Registry: registering " << driver->get_description() << " as " << driver->get_id() << endl;
    drivers.push_back( driver );
    return true;
}

bool Sound_Driver_Registry::unregister_driver( SRef<Sound_Driver*> driver )
{
    std::vector< SRef<Sound_Driver*> >::iterator iter;

    iter = find( drivers.begin(), drivers.end(), driver );

    if ( iter == drivers.end() )
    {
        my_err << "Sound_Driver_Registry unregister_driver: Driver not registered: " << driver->get_id() << endl;
        return false;
    }

    drivers.erase( iter, iter + 1 );
    return true;
}

SRef<Sound_Driver_Registry*> Sound_Driver_Registry::get_instance()
{
    return SSingleton<Sound_Driver_Registry>::get_instance();
}
