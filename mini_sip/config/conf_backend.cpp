#include "conf_backend.h"
#include "online_conf_back.h"
#include "xml_config_plugin.h"
#include "dbg.h"
#include "my_error.h"

#include <iostream>
using namespace std;

SRef<Conf_Backend *> Config_Registry::create_backend( std::string backendName )
{
    string backendArgument;
    if (backendName.find(':')!=string::npos)
    {
        backendArgument = backendName.substr(backendName.find(':')+1);
        backendName = backendName.substr(0, backendName.find(':') );
    }

    try{
        SRef<SPlugin *> plugin;

        if( !backendName.empty() )
        {
            plugin = find_plugin( backendName );
        }
        else
        {
            plugin = find_plugin( "onlineconf" );

            if( !plugin)
                plugin = find_plugin( "gconf" );

            if( !plugin){
                plugin = find_plugin( "mxmlconf" );
            }
        }

        if( !plugin )
        {
            my_err << "ConfigRegistry: Can't create config backend " << backendName << endl;
            return NULL;
        }

        Config_Plugin *config = dynamic_cast<Config_Plugin*>(*plugin);
        if( !config )
        {
            my_err << "ConfigRegistry: Not a config plugin " << plugin->get_name() << endl;
            return NULL;
        }

        return config->create_backend( backendArgument );
    }
    catch( Conf_Backend_Exception & ){
        return NULL;
    }
}
void Conf_Backend::save_bool( const std::string &key, bool value )
{
    save( key, value ? std::string("yes") : std::string("no") );
}

bool Conf_Backend::load_bool( const std::string &key, bool defaultValue )
{
    return load_string( key, defaultValue ? "yes" : "no" ) == "yes";
}
void Conf_Backend::save( const char * key, const std::string &value )
{
    save( std::string( key ), value );
}

void Conf_Backend::save( const char * key, const int32_t value )
{
    save( std::string( key ), value );
}

void Conf_Backend::save_bool( const char * key, bool value )
{
    save_bool( std::string( key ), value );
}

std::string Conf_Backend::load_string( const char * key, const std::string &defaultValue )
{
    return load_string( std::string( key ), defaultValue );
}

int32_t Conf_Backend::load_int( const char * key, const int32_t defaultValue )
{
    return load_int( std::string( key ), defaultValue );
}


Config_Plugin::Config_Plugin( SRef<Library *> lib )
    : SPlugin( lib )
{
}

SRef<Config_Registry *> Config_Registry::instance;

Config_Registry::Config_Registry()
{
}

Config_Registry::~Config_Registry()
{
}

void Config_Registry::register_builtins()
{
    register_plugin( new Xml_Config_Plugin( NULL ) );
}

SRef<Config_Registry *> Config_Registry::get_instance()
{
    if( !instance )
    {
        instance = new Config_Registry();
        instance->register_builtins();
    }
    return instance;
}

void Config_Registry::register_plugin( SRef<SPlugin*> plugin )
{
    Config_Plugin *config = dynamic_cast<Config_Plugin *>(*plugin);

    if( config )
    {
        SPlugin_Registry::register_plugin( plugin );
    }
    else {
        my_err << "Config_Registry: Not a config plugin " << plugin->get_name() << endl;
    }
}
