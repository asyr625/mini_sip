#include "resampler.h"
#include "simple_resampler.h"
#include "dbg.h"

#include <iostream>

Resampler_Plugin::Resampler_Plugin(SRef<Library*> lib)
    : SPlugin( lib )
{
}

Resampler_Plugin::~Resampler_Plugin()
{
}

Resampler_Registry::Resampler_Registry()
{
    register_plugin( new Simple_Resampler_Plugin( NULL ) );
}

SRef<Resampler *> Resampler_Registry::create( uint32_t inputFreq, uint32_t outputFreq,
                          uint32_t duration, uint32_t nChannels )
{
    SRef<SPlugin *> plugin;

    plugin = find_plugin("float_resampler");

    if( !plugin )
        plugin = find_plugin("simple_resampler");

    if( !plugin )
    {
        my_err("Can't create resampler");
        return NULL;
    }

    SRef<Resampler_Plugin *> resampler = dynamic_cast<Resampler_Plugin *>(*plugin);

    if( !resampler )
    {
        my_err("dynamic_cast<ResamplerPlugin *> failed");
        return NULL;
    }

    my_dbg << "Creating resampler " << resampler->get_name() << std::endl;

    return resampler->create_resampler( inputFreq, outputFreq,
                       duration, nChannels );
}
