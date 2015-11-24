#include "sound_driver.h"

Sound_Device_Name::Sound_Device_Name(std::string name_, std::string descr, int maxInputChannels, int maxOutputChannels)
    : name( name_ ),
      description( descr ),
      max_input_channels( maxInputChannels ),
      max_output_channels( maxOutputChannels )
{

}

std::string Sound_Device_Name::get_name() const
{
    return name;
}
std::string Sound_Device_Name::get_description() const
{
    return description;
}

int Sound_Device_Name::get_max_input_channels() const
{
    return max_input_channels;
}

int Sound_Device_Name::get_max_output_channels() const
{
    return max_output_channels;
}

int Sound_Device_Name::operator ==( const Sound_Device_Name &dev ) const
{
    return name == dev.name;
}



Sound_Driver::Sound_Driver( std::string driverId, SRef<Library *> lib )
    : SPlugin( lib ), id( driverId )
{
}

Sound_Driver::~Sound_Driver()
{
}

std::string Sound_Driver::get_id() const
{
    return id;
}

int Sound_Driver::operator == ( const Sound_Driver &driver ) const
{
    return id == driver.id;
}
