#ifndef SPLUGIN_H
#define SPLUGIN_H
#include <list>
#include "sobject.h"
#include "library.h"
#include "message_router.h"

class SPlugin_Registry;

class SPlugin : public virtual SObject
{
public:
    virtual ~SPlugin();

    virtual std::string get_name() const = 0;

    virtual unsigned int get_version() const = 0;

    virtual std::string get_description() const = 0;

    virtual std::string get_plugin_type() const = 0;

    typedef std::list<std::string> * (* lister)(SRef<Library*> lib);

    typedef SPlugin * (* creator)(SRef<Library*> lib);

    virtual std::string get_mem_object_type() const = 0;

protected:
    SPlugin(SRef<Library*> lib);
    SPlugin();

private:
    SRef<Library *> library;
};


class SPlugin_Manager : public SObject
{
public:
    virtual ~SPlugin_Manager();

    static SRef<SPlugin_Manager*> get_instance();

    int load_from_directory( const std::string &path );

    SRef<SPlugin *> load_from_library( const std::string &file, const std::string &entry_point );

    static SRef<SPlugin *> load_from_library( SRef<Library *> lib, const std::string &entry_point );

    int load_from_file( const std::string &filename );

    std::list< std::string > * get_list_from_library( SRef<Library *> lib );

    bool register_plugin( SRef<SPlugin *> p );

    void add_registry( SPlugin_Registry * registry );
    void remove_registry( SPlugin_Registry * registry );

    bool set_search_path( const std::string &search_path );

protected:
    SPlugin_Manager();

private:
    static SRef<SPlugin_Manager*> instance;
    std::list< SRef<Library *> > libraries;

    std::list< SPlugin_Registry * > registries;
};


class SPlugin_Registry : public SObject
{
public:
    typedef std::list< SRef<SPlugin*> >::const_iterator const_iterator;

    SPlugin_Registry();
    virtual ~SPlugin_Registry();
    virtual std::string get_plugin_type() = 0;

    virtual void register_plugin( SRef<SPlugin *> p );

    const_iterator begin() const;
    const_iterator end() const;
    void add_listener(SRef<Command_Receiver*>);

protected:
    virtual SRef<SPlugin*> find_plugin( std::string name ) const;

    std::list< SRef<SPlugin *> > plugins;
    std::list< SRef<Command_Receiver*> > listeners;

private:
    SRef<SPlugin_Manager*> manager;
};

#endif // SPLUGIN_H
