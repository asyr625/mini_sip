#ifndef XML_CONFIG_PLUGIN_H
#define XML_CONFIG_PLUGIN_H

#include "conf_backend.h"
class Xml_File_Parser;

class Xml_Conf_Backend : public Conf_Backend
{
public:
    Xml_Conf_Backend(const std::string &path);
    ~Xml_Conf_Backend();
    virtual void save( const std::string &key, const std::string &value );
    virtual void save( const std::string &key, const int32_t value );

    virtual std::string load_string( const std::string &key, const std::string &defaultValue="" );
    virtual int32_t load_int( const std::string &key, const int32_t defaultValue=0 );

    virtual void commit();

    virtual void reset(const std::string& key);


     std::string get_mem_object_type() const {return "MXmlConfBackend";}
private:
    std::string get_default_config_filename();
    std::string file_name;
    Xml_File_Parser * parser;
};

class Xml_Config_Plugin : public Config_Plugin
{
public:
    Xml_Config_Plugin( SRef<Library *> lib );
    virtual SRef<Conf_Backend *> create_backend(const std::string &configPath=NULL)const;

    virtual std::string get_mem_object_type() const { return "MXmlConfBackend"; }

    virtual std::string get_name()const;

    virtual std::string get_description()const;

    virtual uint32_t get_version()const;
};

#endif // XML_CONFIG_PLUGIN_H
