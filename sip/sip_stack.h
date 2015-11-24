#ifndef SIP_STACK_H
#define SIP_STACK_H
#include <list>

#include "thread.h"
#include "message_router.h"
#include "state_machine.h"

#include "sip_timers.h"
#include "sip_smcommand.h"
#include "cert.h"
#include "sip_stack_internal.h"

#define HIGH_PRIO_QUEUE 2
#define LOW_PRIO_QUEUE 4

class Sip_Request;

class Sip_Dialog;

class Certificate_Chain;

class Certificate_Set;

class Sip_Default_Handler : public Sip_SMCommand_Receiver, public Command_Receiver
{
public:
    virtual void handle_command(std::string subsystem, const Command_String& cmd) = 0;
    virtual Command_String handle_command_resp(std::string subsystem, const Command_String &cmd) = 0;
    virtual bool handle_command(const Sip_SMCommand& cmd) = 0;
};

class Sip_Transport_Config : public SObject
{
public:
    Sip_Transport_Config() {}
    Sip_Transport_Config(const std::string &name);

    std::string get_name() const;

    bool is_enabled() const;

    void set_enable(bool enable);

    int operator==(const Sip_Transport_Config& dev) const;

private:
    std::string _name;
    bool _enabled;
};


class Sip_Stack_Config : public SObject
{
public:
    Sip_Stack_Config();
    virtual std::string get_mem_object_type() const { return "Sip_Stack_Config"; }

    std::string local_ip_string;
    std::string local_ip6_string;
    std::string external_contact_ip;
    int external_contact_udp_port;

    int prefered_local_sip_port;
    int used_local_sip_port;
    int prefered_local_sips_port;
    bool auto_answer;

    bool use_100_rel;

    SRef<Certificate_Chain*>  cert;
    SRef<Certificate_Set*>  cert_db;
    std::string instance_id;
    std::list<SRef<Sip_Transport_Config*> > transports;
};

class Sip_Stack : public Command_Receiver, public Runnable
{
public:
    Sip_Stack(SRef<Sip_Stack_Config*> stack_config);
    ~Sip_Stack();

    void free();

    void set_transaction_handles_ack(bool trans_handle_ack);

    void set_default_dialog_command_handler(SRef<Sip_Default_Handler*> cb);

    virtual void run();
    virtual void stop_running();

    void handle_command(std::string subsystem, const Command_String &cmd);

    Command_String handle_command_resp(std::string subsystem, const Command_String &cmd);

    bool handle_command(const Command_String &cmd);
    bool handle_command(const Sip_SMCommand &cmd);

    void enqueue_timeout(SRef<Sip_Dialog*> receiver, const Sip_SMCommand &cmd);
    void enqueue_command(const Sip_SMCommand &cmd, int queue = LOW_PRIO_QUEUE);

    std::string create_client_transaction(SRef<Sip_Request*> req);

    void set_callback(SRef<Command_Receiver*> callback);

    SRef<Command_Receiver*> get_callback();
    void set_conf_callback(SRef<Command_Receiver*> callback);

    SRef<Command_Receiver *> get_conf_callback();

    void add_dialog(SRef<Sip_Dialog*> d);

    SRef<Sip_Timers*> get_timers();

    SRef<Sip_Stack_Config*> get_stack_config();

    void add_supported_extension(std::string extension);

    std::string get_all_supported_extensions_str();
    bool supports(std::string extension);

    SRef<Timeout_Provider<std::string, SRef<State_Machine<Sip_SMCommand,std::string>*> > *> get_timeout_provider();

    void set_dialog_management(SRef<Sip_Dialog*> mgmt);

    std::list<SRef<Sip_Dialog *> > get_dialogs();

    void start_servers();

    void start_udp_server();
    void start_tcp_server();
    void start_tls_server();

    void start_server( const std::string &transport_name );

    int get_local_sip_port(bool uses_stun=false, const std::string &transport="UDP");


    void set_debug_print_packets(bool enable);
    bool get_debug_print_packets();

    std::string get_stack_status_debug_string();

    void set_inform_transaction_terminate(bool doInform);

     SRef<Sip_Dialog *> get_dialog (std::string call_id);

private:
    friend class Sip_Dialog;
    void *_sip_stack_internal;
};

#endif // SIP_STACK_H
