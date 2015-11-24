#ifndef MINI_SIP_H
#define MINI_SIP_H

#include <list>

#include "sobject.h"
#include "splugin.h"
#include "ssingleton.h"

#include "subsystem_media.h"
class Gui;
class Sip;
class Console_Debugger;
class Mini_Sip;

class Generic_Minisip_Plugin_Instance : public SObject
{
public:
    Generic_Minisip_Plugin_Instance(Mini_Sip* minisip);
protected:
    Mini_Sip* minisip;
};

class Mini_Sip : public SObject
{
public:
    Mini_Sip( SRef<Gui *> gui, int argc, char**argv );
    virtual ~Mini_Sip();

    std::string get_mem_object_type() const {return "Minisip";}

    void set_configuration_location(std::string c)
    {
        conf_path = c;
    }

    int stop();
    int join();
    int exit();

    int start_sip();
    int run_gui();

    void start_debugger();
    void stop_debugger();
    static void do_load_plugins(char **argv);
    static std::string get_plugin_path() {return plugin_path;}

    Subsystem_Media* get_subsystem_media();

    SRef<Sip_Configuration *> get_config() {return phone_conf;}

    SRef<Message_Router*> get_message_router();

private:
    static bool plugins_loaded;
    static std::string plugin_path;
    static void load_plugins(const std::string &argv0);
    static void register_builtin_plugins();

    int init_parse_config();

    std::string conf_path;
    SRef<Subsystem_Media *> subsystem_media;
    SRef<Gui *> gui;
    SRef<Sip_Configuration *> phone_conf;
    SRef<Sip *> sip;
    SRef<Message_Router*> message_router;
    SRef<Console_Debugger *> console_dbg;

    std::list<SRef<Generic_Minisip_Plugin_Instance*> > generic_plugins;
};

class Generic_Minisip_Plugin: public SPlugin
{
public:
    virtual SRef<Generic_Minisip_Plugin_Instance*> new_instance(Mini_Sip* minisip) = 0;
};


class Generic_Minisip_Plugin_Registry: public SPlugin_Registry, public SSingleton<Generic_Minisip_Plugin_Registry>
{
public:
    std::list<SRef<Generic_Minisip_Plugin_Instance*> > create_plugins(Mini_Sip* m);
    virtual std::string get_plugin_type(){ return "genericplugin"; }

protected:
    Generic_Minisip_Plugin_Registry();
private:
    friend class SSingleton<Generic_Minisip_Plugin_Registry>;
};

#endif // MINI_SIP_H
