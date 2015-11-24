#ifndef SOUND_DRIVER_REGISTRY_H
#define SOUND_DRIVER_REGISTRY_H

#include <vector>

#include "sobject.h"
#include "splugin.h"
#include "ssingleton.h"
#include "sound_driver.h"
#include "sound_device.h"

class Sound_Driver_Registry : public SPlugin_Registry, public SSingleton<Sound_Driver_Registry>
{
public:
    virtual std::string get_plugin_type() { return "Sound_Driver"; }
    const std::vector<SRef<Sound_Driver*> > &get_drivers() const;
    std::vector<Sound_Device_Name> get_all_device_names() const;

    virtual SRef<Sound_Driver*> get_default_driver() const;

    SRef<Sound_Device*> create_device( std::string deviceName );
    virtual void register_plugin( SRef<SPlugin*> plugin );
    bool register_driver( SRef<Sound_Driver*> driver );
    bool unregister_driver( SRef<Sound_Driver*> driver );

    static SRef<Sound_Driver_Registry*> get_instance();

protected:
    Sound_Driver_Registry();

private:
    std::vector< SRef<Sound_Driver*> > drivers;

    friend class SSingleton<Sound_Driver_Registry>;
};

#endif // SOUND_DRIVER_REGISTRY_H
