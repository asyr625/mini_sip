#include "mikey_message_psk.h"

#include "mikey_payload.h"
#include "mikey_payload_hdr.h"
#include "mikey_payload_t.h"
#include "mikey_payload_rand.h"
#include "mikey_exception.h"
#include "mikey_payload_cert.h"
#include "mikey_payload_key_data.h"
#include "mikey_payload_err.h"
#include "mikey_payload_id.h"
#include "mikey_payload_kemac.h"
#include "mikey_payload_v.h"
#include "mikey_payload_pke.h"
#include "rand.h"
using namespace std;

Mikey_Message_PSK::Mikey_Message_PSK()
{
}

Mikey_Message_PSK::Mikey_Message_PSK( Key_Agreement_PSK * ka, int encrAlg, int macAlg )
{
    unsigned int csbId = ka->csb_id();
    Mikey_Payload_T * tPayload;
    Mikey_Payload_RAND * randPayload;

    if( !csbId )
    {
        Rand::randomize( &csbId, sizeof( csbId ));
        ka->set_csb_id( csbId );
    }

    add_payload(
        new Mikey_Payload_HDR( HDR_DATA_TYPE_PSK_INIT, 1,
            HDR_PRF_MIKEY_1, csbId,
            ka->ncs(), ka->get_cs_id_map_type(),
            ka->cs_id_map() ) );

    add_payload( tPayload = new Mikey_Payload_T() );

    add_policy_to_payload( ka ); //Is in Mikey_Message.cxx

    // keep a copy of the time stamp
    uint64_t t = tPayload->ts();
    ka->set_tsent( t );

    add_payload( randPayload = new Mikey_Payload_RAND() );
    //keep a copy of the random value
    ka->set_rand(randPayload->rand_data(), randPayload->rand_length() );

    // Derive the transport keys from the PSK:
    byte_t * encrKey = NULL;
    byte_t * iv = NULL;
    unsigned int encrKeyLength = 0;

    derive_transp_keys( ka, encrKey, iv, encrKeyLength, encrAlg, macAlg, t, NULL );

    Mikey_Payload_Key_Data * keydata =
        new Mikey_Payload_Key_Data(
                KEYDATA_TYPE_TGK, ka->tgk(),
                ka->tgk_length(),
                ka->key_validity() );

    byte_t * rawKeyData = new byte_t[ keydata->length() ];
    keydata->write_data( rawKeyData, keydata->length() );

    add_kemac_payload( rawKeyData,
             keydata->length(),
             encrKey, iv, ka->auth_key,
             encrAlg, macAlg );

    if( encrKey != NULL )
        delete [] encrKey;

    if( iv != NULL )
        delete [] iv;

    delete keydata;
    delete [] rawKeyData;
}

void Mikey_Message_PSK::set_offer( Key_Agreement * kaBase )
{
    Key_Agreement_PSK* ka = dynamic_cast<Key_Agreement_PSK*>(kaBase);

    if( !ka )
    {
        throw Mikey_Exception_Message_Content( "Not a PSK keyagreement" );
    }

    SRef<Mikey_Payload *> i = extract_payload( MIKEYPAYLOAD_HDR_PAYLOAD_TYPE );
    bool error = false;
    //uint32_t csbId;
    SRef<Mikey_Cs_Id_Map *> csIdMap;
    SRef<Mikey_Message *> errorMessage = new Mikey_Message();
    //uint8_t nCs;

    if( i.is_null() || i->payload_type() != MIKEYPAYLOAD_HDR_PAYLOAD_TYPE )
    {
        throw Mikey_Exception_Message_Content( "PSK init message had no HDR payload" );
    }

#define hdr ((Mikey_Payload_HDR *)(*i))
    if( hdr->data_type() != HDR_DATA_TYPE_PSK_INIT )
    {
        throw Mikey_Exception_Message_Content( "Expected PSK init message" );
    }

    ka->set_ncs( hdr->ncs() );
    ka->set_csb_id( hdr->csb_id() );
    ka->set_v(hdr->v());

    if( hdr->cs_id_map_type() == HDR_CS_ID_MAP_TYPE_SRTP_ID || hdr->cs_id_map_type() == HDR_CS_ID_MAP_TYPE_IPSEC4_ID )
    {
        ka->set_cs_id_map( hdr->cs_id_map() );
        ka->set_cs_id_map_type( hdr->cs_id_map_type() );
    }
    else
    {
        throw Mikey_Exception_Message_Content( "Unknown type of CS ID map" );
    }


#undef hdr
    errorMessage->add_payload(
            new Mikey_Payload_HDR( HDR_DATA_TYPE_ERROR, 0,
            HDR_PRF_MIKEY_1, ka->csb_id(),
            ka->ncs(), ka->get_cs_id_map_type(),
            ka->cs_id_map() ) );

    //FIXME look at the other fields!

    remove( i );
    i = extract_payload( MIKEYPAYLOAD_T_PAYLOAD_TYPE );

    if( i.is_null() )
        throw Mikey_Exception_Message_Content( "PSK init message had no T payload" );

#define plT ((Mikey_Payload_T*)*i)
    if( plT->check_offset( MAX_TIME_OFFSET ) )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_INVALID_TS ) );
    }

    ka->t_received = plT->ts();

    remove( i );
#undef plT

    add_policy_to_ka(ka); //Is in Mikey_Message.cxx

    i = extract_payload( MIKEYPAYLOAD_RAND_PAYLOAD_TYPE );

    if( i.is_null() )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
    }
#define plRand ((Mikey_Payload_RAND*)*i)

    // FIXME i can be NULL
    ka->set_rand( plRand->rand_data(), plRand->rand_length() );

    remove( i );
#undef plRand

    i = extract_payload( MIKEYPAYLOAD_ID_PAYLOAD_TYPE );

    //FIXME treat the case of an ID payload
    if( !i.is_null() )
    {
        remove( i );
    }

    i = extract_payload( MIKEYPAYLOAD_KEMAC_PAYLOAD_TYPE );

    if( i.is_null() )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
    }

    // FIXME i can be NULL
#define kemac ((Mikey_Payload_KEMAC *)*i)
    int encrAlg = kemac->encr_alg();
    int macAlg  = kemac->mac_alg();
    ka->mac_alg = macAlg;

    // Derive the transport keys
    byte_t * encrKey = NULL;
    byte_t * iv = NULL;
    unsigned int encrKeyLength = 0;

    if( !derive_transp_keys( ka, encrKey, iv, encrKeyLength, encrAlg,
                   macAlg, ka->t_received, *errorMessage ) )
    {
        if( encrKey != NULL )
            delete [] encrKey;
        if( iv != NULL )
            delete [] iv;

        // We always build the error message with HMAC_SHA1, not
        // sure about this

        unsigned int authKeyLength = 20;
        byte_t* authKey = new byte_t[20];
        ka->gen_transp_auth_key( authKey, 20 );

        errorMessage->add_vpayload( MIKEY_MAC_HMAC_SHA1_160,
                ka->t_received, authKey, authKeyLength  );

        delete [] authKey;
        throw Mikey_Exception_Message_Content( errorMessage );
    }

    // decrypt the TGK
    // TODO handle parse failure.
    SRef<Mikey_Payloads*> subPayloads =
        kemac->decode_payloads( MIKEYPAYLOAD_KEYDATA_PAYLOAD_TYPE,
                 encrKey, encrKeyLength, iv );
    list<SRef<Mikey_Payload *> >::iterator iPayload = subPayloads->first_payload();

    Mikey_Payload_Key_Data *keyData = dynamic_cast<Mikey_Payload_Key_Data*>(**iPayload);
    // FIXME: assume only one KeyData subpayload, I don't know what
    // to do of more keys. Ask Ericsson
    int tgkLength = keyData->key_data_length();
    byte_t * tgk = keyData->key_data();

    ka->set_tgk( tgk, tgkLength );
    ka->set_key_validity( keyData->kv() );
#undef kemac

    if( encrKey != NULL )
        delete [] encrKey;
    if( iv != NULL )
        delete [] iv;
}

SRef<Mikey_Message *> Mikey_Message_PSK::build_response( Key_Agreement * kaBase )
{
    Key_Agreement_PSK* ka = dynamic_cast<Key_Agreement_PSK*>(kaBase);

    if( !ka )
    {
        throw Mikey_Exception_Message_Content( "Not a PSK keyagreement" );
    }

    if( ka->get_v() || ka->get_cs_id_map_type() == HDR_CS_ID_MAP_TYPE_IPSEC4_ID )
    {
        // Build the response message
        SRef<Mikey_Message *> result = new Mikey_Message();
        result->add_payload(
            new Mikey_Payload_HDR( HDR_DATA_TYPE_PSK_RESP, 0,
            HDR_PRF_MIKEY_1, ka->csb_id(),
            ka->ncs(), ka->get_cs_id_map_type(),
            ka->cs_id_map() ) );

        result->add_payload( new Mikey_Payload_T() );

        add_policy_to_payload( ka ); //Is in Mikey_Message.cxx

        result->add_vpayload( ka->mac_alg, ka->t_received,
                ka->auth_key, ka->auth_key_length );

        if( ka->auth_key != NULL )
        {
            delete [] ka->auth_key;
            ka->auth_key = NULL;
        }
        return result;
    }

    if( ka->auth_key != NULL )
    {
        delete [] ka->auth_key;
        ka->auth_key = NULL;
    }
    return NULL;
}

SRef<Mikey_Message *> Mikey_Message_PSK::parse_response( Key_Agreement  * kaBase )
{
    Key_Agreement_PSK* ka = dynamic_cast<Key_Agreement_PSK*>(kaBase);

    if( !ka )
    {
        throw Mikey_Exception_Message_Content( "Not a PSK keyagreement" );
    }

    SRef<Mikey_Payload *> i = extract_payload( MIKEYPAYLOAD_HDR_PAYLOAD_TYPE );
    bool error = false;
    SRef<Mikey_Message *> errorMessage = new Mikey_Message();
    SRef<Mikey_Cs_Id_Map *> csIdMap;
    uint8_t nCs;

    if( i.is_null() || i->payload_type() != MIKEYPAYLOAD_HDR_PAYLOAD_TYPE )
    {
        throw Mikey_Exception_Message_Content( "PSK response message had no HDR payload" );
    }

#define hdr ((Mikey_Payload_HDR *)(*i))
    if( hdr->data_type() != HDR_DATA_TYPE_PSK_RESP )
        throw Mikey_Exception_Message_Content( "Expected PSK response message" );

    if( hdr->cs_id_map_type() == HDR_CS_ID_MAP_TYPE_SRTP_ID || hdr->cs_id_map_type() == HDR_CS_ID_MAP_TYPE_IPSEC4_ID)
    {
        csIdMap = hdr->cs_id_map();
    }
    else
    {
        throw Mikey_Exception_Message_Content( "Unknown type of CS ID map" );
    }

    nCs = hdr->ncs();
#undef hdr
    ka->set_cs_id_map( csIdMap );

    errorMessage->add_payload(
            new Mikey_Payload_HDR( HDR_DATA_TYPE_ERROR, 0,
            HDR_PRF_MIKEY_1, ka->csb_id(),
            nCs, HDR_CS_ID_MAP_TYPE_SRTP_ID,
            csIdMap ) );


    remove( i );
    i = extract_payload( MIKEYPAYLOAD_T_PAYLOAD_TYPE );

    if( i.is_null() )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
    }

    // FIXME i can be NULL
#define plT ((Mikey_Payload_T*)*i)
    if( plT->check_offset( MAX_TIME_OFFSET ) )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_INVALID_TS ) );
    }

    uint64_t t_received = plT->ts();
#undef plT

    if( error )
    {
        byte_t authKey[20];
        unsigned int authKeyLength = 20;

        ka->gen_transp_auth_key( authKey, 20 );

        errorMessage->add_vpayload( MIKEY_MAC_HMAC_SHA1_160,
                t_received, authKey, authKeyLength  );

        throw Mikey_Exception_Message_Content( errorMessage );
    }
    add_policy_to_ka(ka); //Is in Mikey_Message.cxx
    return NULL;
}

bool Mikey_Message_PSK::authenticate( Key_Agreement  * kaBase )
{
    Key_Agreement_PSK* ka = dynamic_cast<Key_Agreement_PSK*>(kaBase);

    if( !ka )
    {
        throw Mikey_Exception_Message_Content( "Not a PSK keyagreement" );
    }

    SRef<Mikey_Payload *> payload = *(last_payload());

    if( ka->rand() == NULL )
    {
        SRef<Mikey_Payload *> pl = extract_payload(MIKEYPAYLOAD_RAND_PAYLOAD_TYPE );

        if( pl.is_null() )
        {
            ka->set_auth_error( "The MIKEY init has no RAND payload." );
            return true;
        }

        Mikey_Payload_RAND * randPayload;
        randPayload = (Mikey_Payload_RAND*)*pl;

        ka->set_rand( randPayload->rand_data(), randPayload->rand_length() );
    }

    if( is_initiator_message() )
    {
        if( payload->payload_type() != MIKEYPAYLOAD_KEMAC_PAYLOAD_TYPE)
        {
            throw Mikey_Exception( "PSK init did not end with a KEMAC payload" );
        }

        ka->set_csb_id( csb_id() );
        if( !verify_kemac( ka, false ) )
        {
            return true;
        }
        return false;
    }
    else if( is_responder_message() )
    {
        if( ka->csb_id() != csb_id() )
        {
            ka->set_auth_error( "CSBID mismatch\n" );
            return true;
        }

        if( !verifyV( ka ) )
        {
            return true;
        }
        return false;
    }
    else
    {
        throw Mikey_Exception( "Invalide type for a PSK message" );
    }
}

bool Mikey_Message_PSK::is_initiator_message() const
{
    return type() == MIKEY_TYPE_PSK_INIT;
}
bool Mikey_Message_PSK::is_responder_message() const
{
    return type() == MIKEY_TYPE_PSK_RESP;
}
int32_t Mikey_Message_PSK::key_agreement_type() const
{
    return KEY_AGREEMENT_TYPE_PSK;
}
