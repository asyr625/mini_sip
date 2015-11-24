#include "key_agreement.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <list>

#include "hmac.h"
#include "rand.h"

#include "mikey_payload_sp.h"
#include "mikey_message.h"

#ifdef SCSIM_SUPPORT
#include "sip_sim_smart_cardgd.h"
#endif

using namespace std;

/* serves as define to split inkey in 256 bit chunks */
#define PRF_KEY_CHUNK_LENGTH		32
/* 160 bit of SHA1 take 20 bytes */
#define SHA_DIGEST_SIZE			20


ITgk::~ITgk()
{
}

Key_Agreement::Key_Agreement()
    : use_sim(false),
      tgk_ptr(NULL), tgk_length_value(0),
      rand_ptr(NULL), rand_length_value(0),
      csb_id_value(0),
      cs_id_map_ptr(NULL), ncs_value(0)
{
    kv_ptr = new Key_Validity_Null();
}

Key_Agreement::Key_Agreement(SRef<Sip_Sim *> s)
    : use_sim(true),
      tgk_ptr(NULL), tgk_length_value(0),
      rand_ptr(NULL), rand_length_value(0),
      csb_id_value(0),
      cs_id_map_ptr(NULL), ncs_value(0)
{
    kv_ptr = new Key_Validity_Null();
    this->sim = s;
}

Key_Agreement::~Key_Agreement()
{
    if( tgk_ptr )
        delete [] tgk_ptr;
    if( rand_ptr )
        delete [] rand_ptr;
    std::list<Policy_Type *>::iterator i;
    for( i = policy.begin(); i != policy.end() ; i++ )
        delete *i;
    policy.clear();
}


unsigned int Key_Agreement::rand_length()
{
    return rand_length_value;
}

byte_t * Key_Agreement::rand()
{
    return rand_ptr;
}

void Key_Agreement::set_rand( byte_t * randData, int randLength )
{
    this->rand_length_value = randLength;

    if( this->rand_ptr )
        delete [] this->rand_ptr;

    this->rand_ptr = new unsigned char[ randLength ];
    memcpy( this->rand_ptr, randData, randLength );
}


/* Described in rfc3830.txt Section 4.1.2 */
void p( unsigned char * s, unsigned int sLength,
        unsigned char * label, unsigned int labelLength,
        unsigned int m,
        unsigned char * output )
{
    unsigned int i;
    unsigned int hmac_output_length;
    byte_t * hmac_input = new byte_t[ labelLength + SHA_DIGEST_SIZE ];

    /* initial step
     * calculate A_1 and store in hmac_input */

    hmac_sha1( s, sLength,
               label, labelLength,
               hmac_input, &hmac_output_length );
    assert( hmac_output_length == SHA_DIGEST_SIZE );
    memcpy( &hmac_input[SHA_DIGEST_SIZE], label, labelLength );

    /* calculate P(s,label,1)
     * and store in output[0 ... SHA_DIGEST_SIZE -1] */

    hmac_sha1( s, sLength,
               hmac_input, labelLength + SHA_DIGEST_SIZE,
               output, &hmac_output_length );
    assert( hmac_output_length == SHA_DIGEST_SIZE );

    /* need key-length > SHA_DIGEST_SIZE * 8 bits? */
    for( i = 2; i <= m ; i++ )
    {
        /* calculate A_i = HMAC (s, A_(i-1))
         * A_(i-1) is found in hmac_input
         * and A_i is stored in hmac_input,
         * important: label in upper indices [SHA_DIGEST_SIZE ... labelLength + SHA_DIGEST_SIZE -1]
         * stays untouched and is repetitively reused! */

        hmac_sha1( s, sLength,
                   hmac_input, SHA_DIGEST_SIZE,
                   hmac_input, &hmac_output_length );
        assert( hmac_output_length == SHA_DIGEST_SIZE );

        /* calculate P(s,label,i), which is stored in
         * output[0 ... (i * SHA_DIGEST_SIZE) -1] */

        hmac_sha1( s, sLength,
                   hmac_input, labelLength + SHA_DIGEST_SIZE,
                   &output[ SHA_DIGEST_SIZE * (i-1) ], &hmac_output_length );
        assert( hmac_output_length == SHA_DIGEST_SIZE );
    }

    /* output now contains complete P(s,label,m)
     * in output[0 ... (m * SHA_DIGEST_SIZE) -1] */
    delete [] hmac_input;
}


/* Described in rfc3830.txt Section 4.1.2 */

void prf( unsigned char * inkey,  unsigned int inkeyLength,
      unsigned char * label,  unsigned int labelLength,
      unsigned char * outkey, unsigned int outkeyLength )
{
    unsigned int n;
    unsigned int m;
    unsigned int i;
    unsigned int j;
    unsigned char * p_output;
    n = ( inkeyLength + PRF_KEY_CHUNK_LENGTH -1 )/ PRF_KEY_CHUNK_LENGTH;
    m = ( outkeyLength + SHA_DIGEST_SIZE -1 )/ SHA_DIGEST_SIZE;

    p_output = new unsigned char[ m * SHA_DIGEST_SIZE ];

    memset( outkey, 0, outkeyLength );
    for( i = 1; i <= n-1; i++ )
    {
        p( &inkey[ (i-1)*PRF_KEY_CHUNK_LENGTH ], PRF_KEY_CHUNK_LENGTH, label, labelLength, m, p_output );
        for( j = 0; j < outkeyLength; j++ )
        {
            outkey[j] ^= p_output[j];
        }
    }

    /* Last step */
    p( &inkey[ (n-1)*PRF_KEY_CHUNK_LENGTH ], inkeyLength % PRF_KEY_CHUNK_LENGTH,
            label, labelLength, m, p_output );

    for( j = 0; j < outkeyLength; j++ )
    {
        outkey[j] ^= p_output[j];
    }
    delete [] p_output;
}


void Key_Agreement::key_deriv( byte_t cs_id, unsigned int csb_id, byte_t * inkey, unsigned int inkey_length, byte_t * key,
                               unsigned int key_length, int type )
{
#ifdef SCSIM_SUPPORT
    if (dynamic_cast<Sip_Sim_Smart_CardGD*>(*sim))
    {
        Sip_Sim_Smart_CardGD *gd=dynamic_cast<Sip_Sim_Smart_CardGD*>(*sim);
        gd->get_key(cs_id, csb_id, (byte_t*)rand_ptr, rand_length_value, key, key_length, type);
    }else
#endif
    {
        byte_t * label = new byte_t[4+4+1+rand_length_value];

        switch( type ){
            case KEY_DERIV_SALT:
                label[0] = 0x39;
                label[1] = 0xA2;
                label[2] = 0xC1;
                label[3] = 0x4B;
                break;
            case KEY_DERIV_TEK:
                label[0] = 0x2A;
                label[1] = 0xD0;
                label[2] = 0x1C;
                label[3] = 0x64;
                break;
            case KEY_DERIV_TRANS_ENCR:
                label[0] = 0x15;
                label[1] = 0x05;
                label[2] = 0x33;
                label[3] = 0xE1;
                break;
            case KEY_DERIV_TRANS_SALT:
                label[0] = 0x29;
                label[1] = 0xB8;
                label[2] = 0x89;
                label[3] = 0x16;
                break;
            case KEY_DERIV_TRANS_AUTH:
                label[0] = 0x2D;
                label[1] = 0x22;
                label[2] = 0xAC;
                label[3] = 0x75;
                break;
            case KEY_DERIV_ENCR:
                label[0] = 0x15;
                label[1] = 0x79;
                label[2] = 0x8C;
                label[3] = 0xEF;
                break;
            case KEY_DERIV_AUTH:
                label[0] = 0x1B;
                label[1] = 0x5C;
                label[2] = 0x79;
                label[3] = 0x73;
                break;
        }

        label[4] = cs_id;

        label[5] = (unsigned char)((csb_id>>24) & 0xFF);
        label[6] = (unsigned char)((csb_id>>16) & 0xFF);
        label[7] = (unsigned char)((csb_id>>8) & 0xFF);
        label[8] = (unsigned char)(csb_id & 0xFF);
        memcpy( &label[9], rand_ptr, rand_length_value );
        prf( inkey, inkey_length, label, 9 + rand_length_value, key, key_length );

        delete [] label;
    }
}

void Key_Agreement::gen_tek( byte_t cs_id, byte_t * tek, unsigned int tek_length )
{
#ifdef SCSIM_SUPPORT
    Sip_Sim_Smart_CardGD *gdSim = dynamic_cast<Sip_Sim_Smart_CardGD*>(*sim);
    if (gdSim)
    {
        gdSim->get_key(cs_id, csb_id_value, rand_ptr, rand_length_value, tek, tek_length, KEY_DERIV_TEK);
    }else
#endif
        key_deriv( cs_id, csb_id_value, tgk_ptr, tgk_length_value, tek, tek_length, KEY_DERIV_TEK );
}

void Key_Agreement::gen_salt( byte_t cs_id, byte_t * salt, unsigned int salt_length )
{
    key_deriv( cs_id, csb_id_value, tgk_ptr, tgk_length_value, salt, salt_length, KEY_DERIV_SALT );
}

void Key_Agreement::gen_encr( byte_t cs_id, byte_t * e_key, unsigned int e_keylength )
{
    key_deriv( cs_id, csb_id_value, tgk_ptr, tgk_length_value, e_key, e_keylength, KEY_DERIV_ENCR );
}

void Key_Agreement::gen_auth( byte_t cs_id, byte_t * a_key, unsigned int a_keylength )
{
    key_deriv( cs_id, csb_id_value, tgk_ptr, tgk_length_value, a_key, a_keylength, KEY_DERIV_AUTH );
}

unsigned int Key_Agreement::csb_id()
{
    return csb_id_value;
}

void Key_Agreement::set_csb_id( unsigned int csbIdValue)
{
    csb_id_value = csbIdValue;
}

void Key_Agreement::set_cs_id_map_type(uint8_t type)
{
    cs_id_map_type = type;
}

uint8_t Key_Agreement::get_cs_id_map_type()
{
    return cs_id_map_type;
}

SRef<Mikey_Cs_Id_Map *> Key_Agreement::cs_id_map()
{
    return cs_id_map_ptr;
}

void Key_Agreement::set_cs_id_map( SRef<Mikey_Cs_Id_Map *> idMap )
{
    cs_id_map_ptr = idMap;
}

byte_t Key_Agreement::ncs()
{
    return ncs_value;
}

void Key_Agreement::set_ncs(uint8_t value)
{
    ncs_value = value;
}

void Key_Agreement::set_tgk( byte_t * tgk, unsigned int tgkLength )
{
    if( this->tgk_ptr )
        delete [] this->tgk_ptr;

    this->tgk_length_value = tgkLength;
    this->tgk_ptr = new unsigned char[ tgkLength ];
    if( tgk )
    {
        memcpy( this->tgk_ptr, tgk, tgkLength );
    }
    else if(sim)
    {
        Rand::randomize( this->tgk_ptr, tgkLength, sim );
    }
    else
    {
        Rand::randomize( this->tgk_ptr, tgkLength );
    }
}

unsigned int Key_Agreement::tgk_length()
{
    return tgk_length_value;
}

byte_t * Key_Agreement::tgk()
{
    return tgk_ptr;
}

SRef<Key_Validity *> Key_Agreement::key_validity()
{
    return kv_ptr;
}

void Key_Agreement::set_key_validity( SRef<Key_Validity *> kv )
{
    this->kv_ptr = NULL;

    switch( kv->type() )
    {
    case KEYVALIDITY_NULL:
        this->kv_ptr = new Key_Validity_Null();
        break;
    case KEYVALIDITY_SPI:
        this->kv_ptr =
                new Key_Validity_SPI( *(Key_Validity_SPI *)(*kv) );
        break;
    case KEYVALIDITY_INTERVAL:
        this->kv_ptr = new Key_Validity_Interval( *(Key_Validity_Interval *)(*kv) );
        break;
    default:
        return;
    }
}

SRef<Mikey_Message *> Key_Agreement::initiator_data()
{
    return initiator_data_ptr;
}

void Key_Agreement::set_initiator_data( SRef<Mikey_Message *> data)
{
    initiator_data_ptr = data;
}

SRef<Mikey_Message *> Key_Agreement::responder_data()
{
    return responder_data_ptr;
}

void Key_Agreement::set_responder_data( SRef<Mikey_Message *> data)
{
    responder_data_ptr = data;
}

uint8_t Key_Agreement::set_policy_param_type(uint8_t prot_type, uint8_t policy_type, uint8_t length, byte_t * value)
{
    std::list<Policy_Type *>::iterator i;
    uint8_t policyNo = 0;
    i = policy.begin();
    while( i != policy.end() )
    {
        if( (*i)->policy_no == policyNo )
        {
            i = policy.begin();
            policyNo ++;
        }
        else
            i++;
    }
    policy.push_back (new Policy_Type(policyNo, prot_type, policy_type, length, value));
    return policyNo;
}

void Key_Agreement::set_policy_param_type(uint8_t policy_No, uint8_t prot_type, uint8_t policy_type, uint8_t length, byte_t * value)
{
    Policy_Type * pol;
    if ( (pol = get_policy_param_type(policy_No, prot_type, policy_type) ) == NULL)
        policy.push_back (new Policy_Type(policy_No, prot_type, policy_type, length, value));
    else
    {
        policy.remove(pol);
        delete pol;
        policy.push_back (new Policy_Type(policy_No, prot_type, policy_type, length, value));
    }
}

static byte_t ipsec4values[] = {MIKEY_IPSEC_SATYPE_ESP,MIKEY_IPSEC_MODE_TRANSPORT,MIKEY_IPSEC_SAFLAG_PSEQ,MIKEY_IPSEC_EALG_3DESCBC,24,MIKEY_IPSEC_AALG_SHA1HMAC,16};
static byte_t srtpvalues[] ={MIKEY_SRTP_EALG_AESCM,16,MIKEY_SRTP_AALG_SHA1HMAC,20,14,MIKEY_SRTP_PRF_AESCM,0,1,1,MIKEY_FEC_ORDER_FEC_SRTP,1,10,0};

uint8_t Key_Agreement::set_default_policy(uint8_t prot_type)
{
    std::list<Policy_Type *>::iterator iter;
    uint8_t policyNo = 0;
    iter = policy.begin();
    while( iter != policy.end() )
    {
        if( (*iter)->policy_no == policyNo )
        {
            iter = policy.begin();
            policyNo ++;
        }
        else
            iter++;
    }
    int i, arraysize;
    switch (prot_type)
    {
    case MIKEY_PROTO_SRTP:
        arraysize = 13;
        for(i=0; i< arraysize; i++)
            policy.push_back (new Policy_Type(policyNo, prot_type, i, 1, &srtpvalues[i]));
        break;
    case MIKEY_PROTO_IPSEC4:
        arraysize = 7;
        for(i=0; i< arraysize; i++)
            policy.push_back (new Policy_Type(policyNo, prot_type, i, 1, &ipsec4values[i]));
        break;
    }
    return policyNo;
}

Policy_Type * Key_Agreement::get_policy_param_type(uint8_t policy_No, uint8_t prot_type, uint8_t policy_type)
{
    std::list<Policy_Type *>::iterator i;
    for( i = policy.begin(); i != policy.end()  ; i++ )
        if( (*i)->policy_no == policy_No && (*i)->prot_type == prot_type && (*i)->policy_type == policy_type )
            return *i;
    return NULL;
}

uint8_t Key_Agreement::get_policy_param_type_value(uint8_t policy_No, uint8_t prot_type, uint8_t policy_type)
{
    list<Policy_Type *>::iterator i;
    for( i = policy.begin(); i != policy.end()  ; i++ )
        if( (*i)->policy_no == policy_No && (*i)->prot_type == prot_type && (*i)->policy_type == policy_type && (*i)->length == 1)
            return (uint8_t)(*i)->value[0];

    switch(prot_type)
    {
    case MIKEY_PROTO_SRTP:
        if (policy_type < sizeof(srtpvalues)/sizeof(srtpvalues[0]))
            return srtpvalues[policy_type];
        printf("MIKEY_PROTO_SRTP type out of range %d", policy_type);
        break;
    case MIKEY_PROTO_IPSEC4:
        if (policy_type < sizeof(ipsec4values)/sizeof(ipsec4values[0]))
            return ipsec4values[policy_type];
        printf("MIKEY_PROTO_IPSEC4 type out of range %d", policy_type);
        break;
    default:
        break;
    }
    return 0;
}


std::string Key_Agreement::auth_error()
{
    return auth_error_value;
}

void Key_Agreement::set_auth_error( std::string error )
{
    auth_error_value = error;
}

const std::string &Key_Agreement::uri() const
{
    return uri_value;
}

void Key_Agreement::set_uri( const std::string &uri )
{
    uri_value = uri;
}

const std::string &Key_Agreement::peer_uri() const
{
    return peer_uri_value;
}

void Key_Agreement::set_peer_uri( const std::string &peerUri )
{
    peer_uri_value = peerUri;
}

byte_t Key_Agreement::get_srtp_cs_id( uint32_t ssrc )
{
    Mikey_Cs_Id_Map_Srtp * csIdMap = dynamic_cast<Mikey_Cs_Id_Map_Srtp *>( *cs_id_map_ptr );

    if( csIdMap == NULL )
        return 0;

    return csIdMap->find_cs_id( ssrc );
}

uint32_t Key_Agreement::get_srtp_roc( uint32_t ssrc )
{
    Mikey_Cs_Id_Map_Srtp * csIdMap = dynamic_cast<Mikey_Cs_Id_Map_Srtp *>( *cs_id_map_ptr );

    if( csIdMap == NULL )
        return 0;

    return csIdMap->find_roc( ssrc );
}

uint8_t Key_Agreement::findpolicy_no( uint32_t ssrc )
{
    Mikey_Cs_Id_Map_Srtp * csIdMap = dynamic_cast<Mikey_Cs_Id_Map_Srtp *>( *cs_id_map_ptr );

    if( csIdMap == NULL )
        return 0;

    return csIdMap->findpolicy_no( ssrc );
}

void Key_Agreement::set_rtp_stream_ssrc( uint32_t ssrc, uint8_t csId )
{
    Mikey_Cs_Id_Map_Srtp * csIdMap = dynamic_cast<Mikey_Cs_Id_Map_Srtp *>( *cs_id_map_ptr );

    if( csIdMap == NULL )
        return ;
    csIdMap->set_ssrc( ssrc, csId );
}

void Key_Agreement::set_rtp_stream_roc( uint32_t roc, uint8_t csId )
{
    Mikey_Cs_Id_Map_Srtp * csIdMap = dynamic_cast<Mikey_Cs_Id_Map_Srtp *>( *cs_id_map_ptr );

    if( csIdMap == NULL )
        return ;
    csIdMap->set_roc( roc, csId );
}

void Key_Agreement::add_srtp_stream( uint32_t ssrc, uint32_t roc, byte_t policyNo, byte_t csId )
{
    Mikey_Cs_Id_Map_Srtp * csIdMap;

    if( !cs_id_map_ptr )
    {
        cs_id_map_ptr = new Mikey_Cs_Id_Map_Srtp();
        csIdMap = (Mikey_Cs_Id_Map_Srtp *)(*cs_id_map_ptr);
    }
    else
    {
        csIdMap = dynamic_cast<Mikey_Cs_Id_Map_Srtp *>( *cs_id_map_ptr );
    }

    csIdMap->add_stream( ssrc, roc, policyNo, csId );

    if( csId == 0 )
        ncs_value ++;
}

void Key_Agreement::add_ipsec_sa( uint32_t spi, uint32_t spiSrcaddr, uint32_t spiDstaddr, byte_t policyNo, byte_t csId)
{
    Mikey_Cs_Id_Map_IPSEC4 * csIdMap = dynamic_cast<Mikey_Cs_Id_Map_IPSEC4 *>( *cs_id_map_ptr );
    if( csIdMap == NULL )
    {
        cs_id_map_ptr = new Mikey_Cs_Id_Map_IPSEC4();
        csIdMap = (Mikey_Cs_Id_Map_IPSEC4 *)(*cs_id_map_ptr);
    }
    csIdMap->add_sa(spi, spiSrcaddr, spiDstaddr, policyNo, csId);
    if( csId == 0 )
        ncs_value ++;
}

SRef<Sip_Sim *> Key_Agreement::get_sim()
{
    return sim;
}


Policy_Type::Policy_Type(uint8_t policy_No, uint8_t prot_type, uint8_t policy_type, uint8_t length, byte_t * value)
{
    this->policy_no = policy_No;
    this->prot_type = prot_type;
    this->policy_type = policy_type;
    this->length = length;
    this->value = (byte_t*) calloc (length,sizeof(byte_t));
    for(int i=0; i< length; i++)
        this->value[i] = value[i];
}

Policy_Type::~Policy_Type()
{
    free(value);
}

