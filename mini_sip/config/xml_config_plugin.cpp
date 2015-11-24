#include "xml_config_plugin.h"
#include "user_config.h"

#include "xml_parser.h"
#include "string_utils.h"
#include "dbg.h"

#include <stdlib.h>
#include <iostream>
using namespace std;

static std::list<std::string> pluginList;
static bool initialized;

extern "C"
std::list<std::string> *mxmlconf_LTX_listPlugins( SRef<Library*> lib )
{
    if( !initialized ){
        pluginList.push_back("getPlugin");
        initialized = true;
    }
    return &pluginList;
}

extern "C"
SPlugin * mxmlconf_LTX_getPlugin( SRef<Library*> lib )
{
    return new Xml_Config_Plugin( lib );
}

string searchReplace(const string &str, const string &search, const string &replace)
{
    string tmp = str;
    size_t pos = 0;
    size_t len = search.length();

    while( (pos = tmp.find(search, pos)) != string::npos )
    {
        tmp.erase(pos, len);
        tmp = tmp.insert(pos, replace);
        pos = pos + replace.length();
    }

    return tmp;
}


Xml_Conf_Backend::Xml_Conf_Backend(const std::string &path)
{
    if (path.size()>0)
        file_name = path;
    else
        file_name = get_default_config_filename();

    try{
        parser = new Xml_File_Parser( file_name );
    }
    catch( Xml_File_Not_Found & )
    {
        // Open a new one
        parser = new Xml_File_Parser( "" );
    }
    catch( Xml_Exception &exc )
    {
        my_dbg << "Xml_Conf_Backend caught XMLException: " << exc.what() << endl;
        cerr << "Caught XMLException" << endl;
        throw Conf_Backend_Exception();
    }
}

Xml_Conf_Backend::~Xml_Conf_Backend()
{
    if(parser)
        delete parser;
}

void Xml_Conf_Backend::save( const std::string &key, const std::string &value )
{
    try{
        string tmp = searchReplace( value, "&", "&amp;" );
        string xmlStr = searchReplace( tmp, "<", "&lt;" );
        parser->change_value( key, xmlStr );
    }
    catch( Xml_Exception &exc ){
        my_dbg << "MXmlConfBackend caught XMLException: " << exc.what() << endl;
        throw Conf_Backend_Exception();
    }
}

void Xml_Conf_Backend::save( const std::string &key, const int32_t value )
{
    try{
        parser->change_value( key, itoa( value ) );
    }
    catch( Xml_Exception &exc )
    {
        my_dbg << "Xml_Conf_Backend caught XMLException: " << exc.what() << endl;
        throw Conf_Backend_Exception();
    }
}

std::string Xml_Conf_Backend::load_string( const std::string &key, const std::string &defaultValue )
{
    std::string ret = "";

    try{
        string xmlStr = parser->get_value( key, defaultValue );
        string tmp = searchReplace( xmlStr, "&lt;", "<" );
        ret = searchReplace( tmp, "&amp;", "&" );
    }
    catch( Xml_Exception &exc ){
        my_dbg << "MXmlConfBackend caught XMLException: " << exc.what() << endl;
        throw Conf_Backend_Exception();
    }
    return ret;
}

int32_t Xml_Conf_Backend::load_int( const std::string &key, const int32_t defaultValue )
{
    int32_t ret = -1;

    try{
        ret = parser->get_int_value( key, defaultValue );
    }
    catch( Xml_Exception &exc ){
        my_dbg << "MXmlConfBackend caught XMLException: " << exc.what() << endl;
        throw Conf_Backend_Exception();
    }
    return ret;
}

void Xml_Conf_Backend::commit()
{
}

void Xml_Conf_Backend::reset(const std::string& /*key*/)
{
}

std::string Xml_Conf_Backend::get_default_config_filename()
{
    return User_Config::get_file_name( "minisip.conf" );
}

Xml_Config_Plugin::Xml_Config_Plugin( SRef<Library *> lib )
    : Config_Plugin( lib )
{
}

SRef<Conf_Backend *> Xml_Config_Plugin::create_backend(const std::string &configPath )const
{
    return new Xml_Conf_Backend(configPath);
}

std::string Xml_Config_Plugin::get_name()const
{
    return "mxmlconf";
}

std::string Xml_Config_Plugin::get_description()const
{
    return "MXmlConf backend";
}

uint32_t Xml_Config_Plugin::get_version()const
{
    return 0x00000001;
}
