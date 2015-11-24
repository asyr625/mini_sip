#ifndef KEY_AGREEMENT_H
#define KEY_AGREEMENT_H

#include<assert.h>
#include<iostream>

#include "sobject.h"
#include "mikey_defs.h"
#include "key_validity.h"
#include "mikey_cs_id_map.h"
#include "mikey_message.h"
#include "sip_sim.h"

// different type of key derivation defined in MIKEY
#define KEY_DERIV_TEK          0
#define KEY_DERIV_SALT         1
#define KEY_DERIV_TRANS_ENCR   2
#define KEY_DERIV_TRANS_SALT   3
#define KEY_DERIV_TRANS_AUTH   4
#define KEY_DERIV_ENCR		5
#define KEY_DERIV_AUTH		6

#define KEY_AGREEMENT_TYPE_DH 	0
#define KEY_AGREEMENT_TYPE_PSK 	1
#define KEY_AGREEMENT_TYPE_PK 	2
#define KEY_AGREEMENT_TYPE_DHHMAC 3
#define KEY_AGREEMENT_TYPE_RSA_R 4


class Mikey_Message;

class Policy_Type
{
public:
    Policy_Type(uint8_t policy_No, uint8_t prot_type, uint8_t policy_type, uint8_t length, byte_t * value);
    ~Policy_Type();
    uint8_t policy_no;
    uint8_t prot_type;
    uint8_t policy_type;
    uint8_t length;
    byte_t * value;
};

class ITgk
{
public:
    virtual ~ITgk();
    /**
      * If tgk == NULL, generate random TGK of specified size
      */
    virtual void set_tgk( byte_t * tgk, unsigned int tgkLength ) = 0;
    virtual unsigned int tgk_length() = 0;
    virtual byte_t * tgk() = 0;
};

class Key_Agreement : public SObject, public virtual ITgk
{
public:
    Key_Agreement();
    Key_Agreement(SRef<Sip_Sim *> s);
    ~Key_Agreement();
    virtual int32_t type()=0;

    unsigned int rand_length();
    byte_t * rand();
    void set_rand( byte_t * randData, int randLength );

    void gen_tek( byte_t cs_id, byte_t * tek, unsigned int tek_length );
    void gen_salt( byte_t cs_id, byte_t * salt, unsigned int salt_length );

    void gen_encr( byte_t cs_id, byte_t * e_key, unsigned int e_keylength );
    void gen_auth( byte_t cs_id, byte_t * a_key, unsigned int a_keylength );

    unsigned int csb_id();
    virtual void set_csb_id( unsigned int );

    void set_cs_id_map_type(uint8_t type);
    uint8_t get_cs_id_map_type();
    SRef<Mikey_Cs_Id_Map *> cs_id_map();
    void set_cs_id_map( SRef<Mikey_Cs_Id_Map *> idMap );

    byte_t ncs();
    void set_ncs(uint8_t value);

    void set_tgk( byte_t * tgk, unsigned int tgkLength );
    unsigned int tgk_length();
    byte_t * tgk();

    SRef<Key_Validity *> key_validity();
    void set_key_validity( SRef<Key_Validity *> kv );

    SRef<Mikey_Message *> initiator_data();
    void set_initiator_data(SRef<Mikey_Message *> data);
    SRef<Mikey_Message *> responder_data();
    void set_responder_data(SRef<Mikey_Message *> data);

    uint8_t set_policy_param_type(uint8_t prot_type, uint8_t policy_type, uint8_t length, byte_t * value);

    void set_policy_param_type(uint8_t policy_No, uint8_t prot_type, uint8_t policy_type, uint8_t length, byte_t * value);

    uint8_t set_default_policy(uint8_t prot_type);

    Policy_Type * get_policy_param_type(uint8_t policy_No, uint8_t prot_type, uint8_t policy_type);

    uint8_t get_policy_param_type_value(uint8_t policy_No, uint8_t prot_type, uint8_t policy_type);
    std::list <Policy_Type *> * get_policy() { return &policy; }

    std::string auth_error();
    void set_auth_error( std::string error );

    const std::string &uri() const;
    void set_uri( const std::string &uri );

    const std::string &peer_uri() const;
    void set_peer_uri( const std::string &peerUri );

    virtual std::string get_mem_object_type() const {return "KeyAgreement";}

    /* IPSEC Specific */
    void add_ipsec_sa( uint32_t spi, uint32_t spiSrcaddr, uint32_t spiDstaddr, byte_t policyNo, byte_t csId = 0);
    byte_t get_srtp_cs_id( uint32_t ssrc );
    uint32_t get_srtp_roc( uint32_t ssrc );
    uint8_t findpolicy_no( uint32_t ssrc );

    void set_rtp_stream_ssrc( uint32_t ssrc, uint8_t csId );
    void set_rtp_stream_roc( uint32_t roc, uint8_t csId );

    void add_srtp_stream( uint32_t ssrc, uint32_t roc=0, byte_t policyNo=0, byte_t csId=0 );

    virtual Mikey_Message* create_message() = 0;

    bool use_sim;
    SRef<Sip_Sim *> get_sim();

protected:
    void key_deriv( byte_t cs_id, unsigned int csb_id, byte_t * inkey, unsigned int inkey_length, byte_t * key,
                    unsigned int key_length, int type );

    SRef<Sip_Sim *> sim;

private:
    std::list<Policy_Type *> policy; //Contains the security policy

    byte_t * tgk_ptr;
    unsigned int tgk_length_value;
    byte_t * rand_ptr;
    unsigned int rand_length_value;

    unsigned int csb_id_value;

    SRef<Key_Validity *> kv_ptr;
    SRef<Mikey_Cs_Id_Map *> cs_id_map_ptr;
    uint8_t ncs_value;
    uint8_t	cs_id_map_type;

    SRef<Mikey_Message *> initiator_data_ptr;
    SRef<Mikey_Message *> responder_data_ptr;

    std::string auth_error_value;

    std::string uri_value;
    std::string peer_uri_value;
};

#endif // KEY_AGREEMENT_H
