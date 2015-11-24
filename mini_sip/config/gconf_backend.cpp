#include "gconf_backend.h"

#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <string.h>
#include <iostream>

#ifdef MAEMO_SUPPORT
#	define KEY_ROOT "/apps/maemo/minisip/"
#else
#	define KEY_ROOT "/apps/minisip/"
#endif

//#define GCONF_BOOL_AS_STRINGS

using namespace std;

static std::list<std::string> pluginList;
static int initialized;

extern "C"
std::list<std::string> *mgconf_LTX_listPlugins( SRef<Library*> lib )
{
    if( !initialized ){
        pluginList.push_back("getPlugin");
        initialized = true;
    }
    return &pluginList;
}

extern "C"
SPlugin * mgconf_LTX_getPlugin( SRef<Library*> lib )
{
    return new GConfig_Plugin( lib );
}

GConf_Backend::GConf_Backend()
{
}
GConf_Backend::GConf_Backend();
GConf_Backend::~GConf_Backend();
void GConf_Backend::save( const std::string &key, const std::string &value );
void GConf_Backend::save( const std::string &key, const int32_t value );
void GConf_Backend::save_bool( const std::string &key, bool value );

std::string GConf_Backend::load_string( const std::string &key, const std::string &defaultValue="" );
int32_t GConf_Backend::load_int( const std::string &key, const int32_t defaultValue=0 );
bool GConf_Backend::load_bool( const std::string &key, bool defaultValue=false );
void GConf_Backend::commit();

void GConf_Backend::reset(const std::string& key)
{

}
void GConf_Backend::sanitize_key( std::string &key )
{
    size_t n = 0;
    do{
        n = key.find( '[', n );
        if( n!= string::npos ){
            key[n] = '_';
        }
    } while( n != string::npos );

    n = 0;
    do{
        n = key.find( ']', n );
        if( n!= string::npos ){
            key[n] = '_';
        }
    } while( n != string::npos );
}

GConfig_Plugin::GConfig_Plugin( SRef<Library *> lib )
    : Config_Plugin( lib )
{
}

SRef<Conf_Backend *> GConfig_Plugin::create_backend(const std::string & /*configPath*/ )const
{
    return new GConf_Backend();
}

std::string GConfig_Plugin::get_name()const
{
    return "gconf";
}

std::string GConfig_Plugin::get_description()const
{
    return "GConf backend";
}

uint32_t GConfig_Plugin::get_version()const
{
    return 0x00000001;
}
