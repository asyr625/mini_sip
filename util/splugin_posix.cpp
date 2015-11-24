#include "splugin.h"

#include "dbg.h"
#include "library.h"

// Use ltdl symbols defined in ltdl.c
#define LT_SCOPE extern
#include<ltdl.h>
#include <iostream>
using namespace std;

SRef<SPlugin_Manager*> SPlugin_Manager::instance;

SPlugin::SPlugin(SRef<Library*> lib)
    : library( lib )
{
}

SPlugin::SPlugin(): library( NULL )
{
}

SPlugin::~SPlugin()
{
}

string SPlugin::get_mem_object_type() const
{
    return "SPlugin";
}


SPlugin_Manager::SPlugin_Manager()
{
    lt_dlinit();
    libraries.clear();
}

SPlugin_Manager::~SPlugin_Manager()
{
    libraries.clear();
    registries.clear();
    lt_dlexit();
}

SRef<SPlugin_Manager*> SPlugin_Manager::get_instance()
{
    if( !instance )
    {
        instance = new SPlugin_Manager();
    }
    return instance;
}

list<string> * SPlugin_Manager::get_list_from_library( SRef<Library *> lib )
{
    SPlugin::lister listerFunction;

    listerFunction = (SPlugin::lister)lib->get_function_ptr( "listPlugins" );

    if( listerFunction )
    {
        return listerFunction( lib );
    }

    return NULL;
}

int32_t SPlugin_Manager::load_from_file( const std::string &filename )
{
    int32_t nPlugins = 0;
    list<string> * entryPoints;
    SRef<Library *> lib;
    list< SRef<Library *> >::iterator iLib;

    lib = Library::open( filename );

    if( !lib )
    {
        my_dbg << "SPlugin_Manager: Can't load " << filename << endl;
        // Continue;
        return -1;
    }

    for( iLib = libraries.begin(); iLib != libraries.end(); iLib++ )
    {
        SRef<Library *> cur = *iLib;

        if( cur->get_path() == lib->get_path() )
        {
            //			my_dbg << "SPlugin_Manager: Already loaded " << filename << endl;
            return -1;
        }
    }

    entryPoints = get_list_from_library( lib );

    if( entryPoints )
    {
        list<string>::iterator iEP;
        for( iEP = entryPoints->begin(); iEP!= entryPoints->end(); iEP ++ )
        {
            SRef<SPlugin *> p;

            p = load_from_library( lib, *iEP );
            if( p )
            {
                if( register_plugin( p ) )
                {
                    nPlugins ++;
                }
            }
            else
            {
                my_err << "SPlugin_Manager: No plugin for ep: " << *iEP << endl;
            }
        }
    }
    else{
        my_err << "SPlugin_Manager: No entrypoints in " << filename << endl;
    }

    if( nPlugins > 0 )
    {
        libraries.push_back( lib );
    }
    else
    {
        my_dbg << "SPlugin_Manager: No plugins loaded from " << lib->get_path() << endl;
    }
    return nPlugins;
}

struct ltdl_info
{
    ltdl_info() : manager(NULL),nTotalPlugins(0){}
    ~ltdl_info() { manager = NULL; }

    SRef<SPlugin_Manager *> manager;
    int32_t nTotalPlugins;
};

static int ltdl_callback(const char *filename, lt_ptr data)
{
    ltdl_info *info = (ltdl_info*)data;
    int32_t nPlugins;

    nPlugins = info->manager->load_from_file( filename );

    if( nPlugins > 0 )
    {
        info->nTotalPlugins += nPlugins;
    }

    // Continue;
    return 0;
}

int32_t SPlugin_Manager::load_from_directory( const string &path )
{
    ltdl_info info;
    info.manager = this;

    int res = lt_dlforeachfile(path.c_str(), ltdl_callback, &info);
    if( res < 0 )
    {
        my_err << lt_dlerror() << endl;
    }
    return info.nTotalPlugins;
}

SRef<SPlugin *> SPlugin_Manager::load_from_library( const string &file, const string &entryPoint )
{
    SRef<Library *> lib = NULL;
    list< SRef<Library *> >::iterator iLib;
    SRef<SPlugin *> p;
    bool newLib = false;

    for( iLib = libraries.begin(); iLib != libraries.end(); iLib ++ )
    {
        if( (*iLib)->get_path() == file )
        {
            lib = *iLib;
        }
    }

    /* This library hasn't been opened yet */
    if( !lib )
    {
        lib = Library::open( file );
        if( lib )
        {
            newLib = true;
        }
    }

    p =  load_from_library( lib, entryPoint );

    if( p && newLib )
    {
        libraries.push_back( lib );
    }

    return p;
}


SRef<SPlugin *> SPlugin_Manager::load_from_library( SRef<Library *> lib, const string &entryPoint )
{
    SPlugin::creator creatorFunction;

    if( !lib ){
        /* This library doen't exist or couldn't be opened */
        return NULL;
    }

    creatorFunction = (SPlugin::creator)(lib->Library::get_function_ptr( entryPoint ));

    if( creatorFunction )
    {
        SRef<SPlugin *> pp = creatorFunction( lib );

        if( !pp.is_null() )
        {
            my_dbg << "SPlugin_Manager: loaded " << pp->get_name() << "(" << pp->get_description() << ")" << endl;
            return pp;
        }
    }

    return NULL;
}

bool SPlugin_Manager::register_plugin( SRef<SPlugin *> p )
{
    std::list< SPlugin_Registry * >::iterator iReg;

    for( iReg = registries.begin(); iReg != registries.end(); iReg ++ )
    {
        if( (*iReg)->get_plugin_type() == p->get_plugin_type() )
        {
            (*iReg)->register_plugin( p );
            return true;
        }
    }

    my_err << "SPlugin_Manager: Can't find registry for " << p->get_plugin_type() << endl;
    return false;
}

void SPlugin_Manager::add_registry( SPlugin_Registry * registry )
{
    registries.push_back( registry );
    //  cout << "registries size, after addition: " << registries.size() << endl;
}

void SPlugin_Manager::remove_registry( SPlugin_Registry * registry )
{
    registries.remove( registry );
}

bool SPlugin_Manager::set_search_path( const std::string &searchPath )
{
    my_dbg << "SPlugin_Manager: set_search_path " << searchPath << endl;

    bool res = lt_dlsetsearchpath( searchPath.c_str() );
    return res;
}


void SPlugin_Registry::add_listener(SRef<Command_Receiver*> l)
{
    listeners.push_back(l);
}

void SPlugin_Registry::register_plugin( SRef<SPlugin *> p )
{
    plugins.push_back( p );
    std::list< SRef<Command_Receiver*> >::iterator cRes;
    for (cRes = listeners.begin(); cRes!=listeners.end(); cRes++)
    {
        (*cRes)->handle_command("",Command_String("","newplugin",p->get_name(), p->get_plugin_type()));
    }
}

SPlugin_Registry::SPlugin_Registry()
{
    manager = SPlugin_Manager::get_instance();
    manager->add_registry( this );
}

SPlugin_Registry::~SPlugin_Registry()
{
    plugins.clear();
    manager->remove_registry( this );
}

SRef<SPlugin*> SPlugin_Registry::find_plugin( std::string name ) const
{
    list< SRef<SPlugin *> >::const_iterator iter;
    list< SRef<SPlugin *> >::const_iterator last = plugins.end();

    for( iter = plugins.begin(); iter != last; iter++ )
    {
        SRef<SPlugin *> plugin = *iter;

        if( plugin->get_name() == name )
        {
            return plugin;
        }
    }

    return NULL;
}

SPlugin_Registry::const_iterator SPlugin_Registry::begin() const
{
    return plugins.begin();
}

SPlugin_Registry::const_iterator SPlugin_Registry::end() const
{
    return plugins.end();
}
