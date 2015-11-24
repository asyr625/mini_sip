#ifndef GCONF_BACKEND_H
#define GCONF_BACKEND_H

#include "conf_backend.h"

typedef struct _GConfClient GConfClient;

class GConf_Backend : public Conf_Backend
{
public:
    GConf_Backend();
    ~GConf_Backend();
    virtual void save( const std::string &key, const std::string &value );
    virtual void save( const std::string &key, const int32_t value );
    void save_bool( const std::string &key, bool value );

    virtual std::string load_string( const std::string &key, const std::string &defaultValue="" );
    virtual int32_t load_int( const std::string &key, const int32_t defaultValue=0 );
    bool load_bool( const std::string &key, bool defaultValue=false );
    virtual void commit();

    virtual void reset(const std::string& key);
private:
    GConfClient * client;

    void sanitize_key( std::string &key );
};

class GConfig_Plugin : public Config_Plugin
{
public:
    GConfig_Plugin( SRef<Library *> lib );
    virtual SRef<Conf_Backend *> create_backend(const std::string &configPath=NULL)const;

    virtual std::string get_mem_object_type() const { return "GConfBackend"; }

    virtual std::string get_name()const;

    virtual std::string get_description()const;

    virtual uint32_t get_version()const;
};

#endif // GCONF_BACKEND_H
