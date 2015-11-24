#ifndef SIP_DIALOG_CONFIG_H
#define SIP_DIALOG_CONFIG_H


#include "sobject.h"
#include "sip_uri.h"
#include "sip_stack.h"
#include "sip_sim.h"

#define DEFAULT_SIPPROXY_EXPIRES_VALUE_SECONDS 1000

#define KEY_MGMT_METHOD_NULL            0x00
#define KEY_MGMT_METHOD_MIKEY           0x10
#define KEY_MGMT_METHOD_MIKEY_DH        0x11
#define KEY_MGMT_METHOD_MIKEY_PSK       0x12
#define KEY_MGMT_METHOD_MIKEY_PK        0x13
#define KEY_MGMT_METHOD_MIKEY_DHHMAC    0x14
#define KEY_MGMT_METHOD_MIKEY_RSA_R     0x15

class Sip_Stack_Config;

class Sip_Credential : public SObject
{
public:
    Sip_Credential( const std::string &username,
               const std::string &password,
               const std::string &realm = "" );

    const std::string &get_realm() const;
    const std::string &get_username() const;
    const std::string &get_password() const;

    void set( const std::string &username,
          const std::string &password,
          const std::string &realm = "" );

private:
    std::string _realm;
    std::string _username;
    std::string _password;
};

class Sip_Registrar : public SObject
{
public:
    Sip_Registrar();
    Sip_Registrar(const Sip_Uri &addr, int port = -1);

    Sip_Registrar(const Sip_Uri &userUri, std::string transport);

    std::string get_debug_string();

    void set_register_expires( std::string _expires );
    void set_register_expires( int _expires );
    std::string get_register_expires( );
    int get_register_expires_int( );

    void set_default_expires( std::string _expires );
    void set_default_expires( int _expires );
    std::string get_default_expires();
    int get_default_expires_int();

    const Sip_Uri get_uri() const { return _uri; }

    std::string get_mem_object_type() const { return "SipRegistrar"; }

    bool auto_detect_settings;
protected:
    void set_registrar(const Sip_Uri &addr, int port=-1);
private:
    int _default_expires;
    int _register_expires; //in seconds

    Sip_Uri _uri;
};


class Sip_Identity : public SObject
{
public:
    Sip_Identity();
    Sip_Identity(const Sip_Uri &sipuri);

    void set_identity_name(std::string n);

    void set_sip_uri(const Sip_Uri &addr) { _sip_uri = addr; }

    const Sip_Uri &get_sip_uri() const { return _sip_uri; }

    SRef<Sip_Credential*> get_credential() const;
    void set_credential(SRef<Sip_Credential*> aCredential );

    SRef<Sip_Registrar *> get_sip_registrar();

    bool set_sip_registrar( SRef<Sip_Registrar *> proxy );

    std::string set_sip_proxy( bool autodetect, std::string userUri, std::string transport, std::string proxyAddr, int port );

    const std::list<Sip_Uri> &get_route_set() const;
    void set_route_set( const std::list<Sip_Uri> &routeSet );
    void add_route( const Sip_Uri &route );

    void set_do_register(bool f);

    bool get_do_register();

    void lock();
    void unlock();

    std::string get_debug_string();

    virtual std::string get_mem_object_type() const {return "Sip_Identity";}

    std::string get_id();

    std::string identity_identifier;

    bool _register_to_proxy;

    void set_is_registered( bool registerOk );

    bool is_registered();

    void set_registered_contacts( const std::list<Sip_Uri> &contacts );

    const std::list<Sip_Uri>& get_registered_contacts() const;

    void set_sim(SRef<Sip_Sim*> s){ sim = s;}

    SRef<Sip_Sim *> get_sim(){return sim;}

    std::string get_psk(){return psk;}

    void set_psk( std::string key );

    Sip_Uri get_contact_uri( SRef<Sip_Stack*> sipStack, bool useStun ) const;

    bool security_enabled;
    int ka_type;
    bool dh_enabled;
    bool psk_enabled;
    bool check_cert;
    bool use_zrtp;

private:
    Sip_Uri _sip_uri;

    SRef<Sip_Sim *> sim;
    std::string psk;


    SRef<Sip_Registrar *> sip_proxy;

    SRef<Sip_Credential *> credential;

    std::list<Sip_Uri> route_set;

    static int global_index;
    std::string identity_idx;

    bool currently_registered;

    std::list<Sip_Uri> registered_contacts;
    Mutex mutex;

    void init();
};

class Sip_Dialog_Config : public SObject
{
public:
    Sip_Dialog_Config(SRef<Sip_Stack*> stack);

    virtual std::string get_mem_object_type() const {return "SipDialogConfig";}

    void use_identity( SRef<Sip_Identity*> identity, std::string transport="UDP_X");

    Sip_Uri get_contact_uri( bool use_stun ) const;

public:
    SRef<Sip_Stack*> _sip_stack;
    unsigned int     _local_ssrc;
    SRef<Sip_Identity*> _sip_identity;
};

#endif // SIP_DIALOG_CONFIG_H
