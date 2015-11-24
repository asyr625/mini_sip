#ifndef CONF_BACKEND_H
#define CONF_BACKEND_H

#include "my_types.h"
#include "sobject.h"
#include "splugin.h"
#include "ssingleton.h"

class Online_Conf_Back;

class Conf_Backend : public SObject
{
public:
    virtual void save( const std::string &key, const std::string &value ) = 0;
    virtual void save( const std::string &key, const int32_t value ) = 0;
    virtual void save_bool( const std::string &key, bool value );

    virtual std::string load_string( const std::string &key, const std::string &defaultValue="" ) = 0;
    virtual int32_t load_int( const std::string &key, const int32_t defaultValue=0 ) = 0;
    virtual bool load_bool( const std::string &key, bool defaultValue=false );

    virtual void reset( const std::string & /* key */ ){}

    virtual void commit() = 0;

    virtual Online_Conf_Back * get_conf() { return NULL; }

    void save( const char * key, const std::string &value );
    void save( const char * key, const int32_t value );
    void save_bool( const char * key, bool value );

    std::string load_string( const char * key, const std::string &defaultValue="" );
    int32_t load_int( const char * key, const int32_t defaultValue=0 );
};

class Conf_Backend_Exception{};

class Config_Plugin : public SPlugin
{
public:
    virtual SRef<Conf_Backend *> create_backend( const std::string& argument = NULL )const = 0;

    virtual std::string get_plugin_type()const{ return "Config"; }

protected:
    Config_Plugin( SRef<Library *> lib );
};


class Config_Registry: public SPlugin_Registry
{
public:
    virtual ~Config_Registry();

    virtual std::string get_plugin_type() { return "Config"; }

    static SRef<Config_Registry*> get_instance();

    SRef<Conf_Backend*> create_backend( std::string backendName="" );

    virtual void register_plugin( SRef<SPlugin*> plugin );

protected:
    Config_Registry();
    void register_builtins();

private:
    static SRef<Config_Registry *> instance;
};

#endif // CONF_BACKEND_H
