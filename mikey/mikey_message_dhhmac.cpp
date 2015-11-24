#include "mikey_message_dhhmac.h"

#include "mikey_payload.h"
#include "mikey_exception.h"
#include "mikey_payload_err.h"
#include "mikey_payload_dh.h"
#include "mikey_payload_hdr.h"
#include "mikey_payload_id.h"
#include "mikey_payload_kemac.h"
#include "mikey_payload_rand.h"
#include "mikey_payload_t.h"
#include "rand.h"

#include<map>

using namespace std;

Mikey_Message_DHHMAC::Mikey_Message_DHHMAC()
{
}

Mikey_Message_DHHMAC::Mikey_Message_DHHMAC( Key_Agreement_DHHMAC * ka, int macAlg )
{
    unsigned int csbId = ka->csb_id();

    if( !csbId )
    {
        Rand::randomize( &csbId, sizeof( csbId ));
        ka->set_csb_id( csbId );
    }

    add_payload(
        new Mikey_Payload_HDR( HDR_DATA_TYPE_DHHMAC_INIT, 1,
            HDR_PRF_MIKEY_1, csbId,
            ka->ncs(), ka->get_cs_id_map_type(),
            ka->cs_id_map() ) );

    add_payload( new Mikey_Payload_T() );

    add_policy_to_payload( ka ); //Is in Mikey_Message.cxx

    Mikey_Payload_RAND * payload;
    add_payload( payload = new Mikey_Payload_RAND() );

    //keep a copy of the random value!
    ka->set_rand( payload->rand_data(), payload->rand_length() );

    // TODO add IDr

    add_payload( new Mikey_Payload_DH( ka->group(), ka->public_key(), ka->key_validity() ) );

    // Add KEMAC
    byte_t* encrKey = NULL;
    byte_t* iv = NULL;
    unsigned int encrKeyLength = 0;
    int encrAlg = MIKEY_ENCR_NULL;
    Mikey_Message::derive_transp_keys( ka, encrKey, iv, encrKeyLength, encrAlg, macAlg, 0, NULL );
    add_kemac_payload( NULL, 0, NULL, NULL, ka->auth_key, encrAlg, ka->mac_alg );

    if( encrKey )
        delete[] encrKey;
    if( iv )
        delete[] iv;
}


void Mikey_Message_DHHMAC::set_offer( Key_Agreement * kaBase )
{
    Key_Agreement_DHHMAC* ka = dynamic_cast<Key_Agreement_DHHMAC*>(kaBase);

    if( !ka )
    {
        throw Mikey_Exception_Message_Content( "Not a DHHMAC keyagreement" );
    }

    SRef<Mikey_Payload *> i = extract_payload( MIKEYPAYLOAD_HDR_PAYLOAD_TYPE );
    bool error = false;
    SRef<Mikey_Message *> errorMessage = new Mikey_Message();

    if( i.is_null() )
    {
        throw Mikey_Exception_Message_Content( "DHHMAC init message had no HDR payload" );
    }

#define hdr ((Mikey_Payload_HDR *)*i)
    if( hdr->data_type() != HDR_DATA_TYPE_DHHMAC_INIT )
        throw Mikey_Exception_Message_Content( "Expected DHHMAC init message" );

    ka->set_ncs( hdr->ncs() );
    ka->set_csb_id( hdr->csb_id() );


    if( hdr->cs_id_map_type() == HDR_CS_ID_MAP_TYPE_SRTP_ID || hdr->cs_id_map_type() == HDR_CS_ID_MAP_TYPE_IPSEC4_ID)
    {
        ka->set_cs_id_map( hdr->cs_id_map() );
        ka->set_cs_id_map_type( hdr->cs_id_map_type() );
    }
    else
    {
        throw Mikey_Exception_Message_Content( "Unknown type of CS ID map" );
    }
    payloads.remove( i );
#undef hdr


    errorMessage->add_payload(
            new Mikey_Payload_HDR( HDR_DATA_TYPE_ERROR, 0,
            HDR_PRF_MIKEY_1, ka->csb_id(),
            ka->ncs(), ka->get_cs_id_map_type(),
            ka->cs_id_map() ) );

    i = extract_payload( MIKEYPAYLOAD_T_PAYLOAD_TYPE );
    if( i.is_null() )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
    }

#define plT ((Mikey_Payload_T*)*i)
    if( plT->check_offset( MAX_TIME_OFFSET ) )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_INVALID_TS ) );
    }

    payloads.remove( i );
#undef plT

    add_policy_to_ka(ka); //Is in Mikey_Message.cxx

    i = extract_payload( MIKEYPAYLOAD_RAND_PAYLOAD_TYPE );

    if( i.is_null() )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
    }

#define plRand ((Mikey_Payload_RAND*)*i)
    ka->set_rand( plRand->rand_data(), plRand->rand_length() );

    payloads.remove( i );
#undef plRand


    //FIXME treat the case of an ID payload
    i = extract_payload( MIKEYPAYLOAD_ID_PAYLOAD_TYPE );
    if( !i.is_null() )
    {
        payloads.remove( i );
    }

    i = extract_payload( MIKEYPAYLOAD_DH_PAYLOAD_TYPE );

    if( i.is_null() )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
    }

#define plDH ((Mikey_Payload_DH*)*i)
    if( ka->group() != plDH->group() )
    {
        if( ka->set_group( plDH->group() ) )
        {
            error = true;
            errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_INVALID_DH ) );
        }
    }

    ka->set_peer_key( plDH->dh_key(), plDH->dh_key_length() );

    ka->set_key_validity( plDH->kv() );

    payloads.remove( i );
#undef plDH

    i = extract_payload( MIKEYPAYLOAD_KEMAC_PAYLOAD_TYPE );

#define plKemac ((Mikey_Payload_KEMAC*)*i)
    if( i.is_null() )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
    }
    else
    {
        ka->mac_alg = plKemac->mac_alg();;
    }
#undef plKemac

    if( error )
    {
        throw Mikey_Exception_Message_Content( errorMessage );
    }
}

SRef<Mikey_Message *> Mikey_Message_DHHMAC::build_response( Key_Agreement * kaBase )
{
    Key_Agreement_DHHMAC* ka = dynamic_cast<Key_Agreement_DHHMAC*>(kaBase);
    if( !ka )
    {
        throw Mikey_Exception_Message_Content( "Not a DHHMAC keyagreement" );
    }

    // Build the response message
    SRef<Mikey_Message *> result = new Mikey_Message();
    result->add_payload(
            new Mikey_Payload_HDR( HDR_DATA_TYPE_DHHMAC_RESP, 0,
            HDR_PRF_MIKEY_1, ka->csb_id(),
            ka->ncs(), ka->get_cs_id_map_type(),
            ka->cs_id_map() ) );

    result->add_payload( new Mikey_Payload_T() );

    // FIXME add IDi

    result->add_payload( new Mikey_Payload_DH( ka->group(), ka->public_key(), ka->key_validity() ) );

    result->add_payload( new Mikey_Payload_DH( ka->group(), ka->peer_key(), ka->key_validity() ) );

    // KEMAC
    byte_t * encrKey = NULL;
    byte_t * iv = NULL;
    unsigned int encrKeyLength = 0;
    int encrAlg = MIKEY_ENCR_NULL;

    derive_transp_keys( ka, encrKey, iv, encrKeyLength, encrAlg, ka->mac_alg, 0, NULL );

    result->add_kemac_payload( NULL, 0, NULL, NULL, ka->auth_key, encrAlg, ka->mac_alg );

    if( encrKey )
        delete[] encrKey;
    if( iv )
        delete[] iv;

    return result;
}

SRef<Mikey_Message *> Mikey_Message_DHHMAC::parse_response( Key_Agreement  * kaBase )
{
    Key_Agreement_DHHMAC* ka = dynamic_cast<Key_Agreement_DHHMAC*>(kaBase);

    if( !ka )
    {
        throw Mikey_Exception_Message_Content( "Not a DHHMAC keyagreement" );
    }

    SRef<Mikey_Payload *> i = extract_payload( MIKEYPAYLOAD_HDR_PAYLOAD_TYPE );
    bool error = false;
    bool gotDhi = false;
    SRef<Mikey_Message *> errorMessage = new Mikey_Message();
    SRef<Mikey_Cs_Id_Map *> csIdMap;
    uint8_t nCs;

    if( i.is_null() )
    {
        throw Mikey_Exception_Message_Content( "DHHMAC resp message had no HDR payload" );
    }

#define hdr ((Mikey_Payload_HDR *)(*i))
    if( hdr->data_type() != HDR_DATA_TYPE_DHHMAC_RESP )
    {
        throw Mikey_Exception_Message_Content( "Expected DHHMAC resp message" );
    }

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
    //FIXME look at the other fields!

    errorMessage->add_payload(
            new Mikey_Payload_HDR( HDR_DATA_TYPE_ERROR, 0,
            HDR_PRF_MIKEY_1, ka->csb_id(),
            nCs, ka->get_cs_id_map_type(),
            csIdMap ) );

    payloads.remove( i );
    i = extract_payload( MIKEYPAYLOAD_T_PAYLOAD_TYPE );

    if( i.is_null() )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
    }

#define plT ((Mikey_Payload_T *)(*i))
    if( plT->check_offset( MAX_TIME_OFFSET ) )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_INVALID_TS ) );
    }

    payloads.remove( i );
#undef plT

    add_policy_to_ka(ka); //Is in Mikey_Message.cxx

    i = extract_payload( MIKEYPAYLOAD_ID_PAYLOAD_TYPE );
    if( !i.is_null() ){
        payloads.remove( i );
    }

    i = extract_payload( MIKEYPAYLOAD_DH_PAYLOAD_TYPE );

    if( i.is_null() )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
    }

#define dh ((Mikey_Payload_DH *)*i)
    if( string( (const char *)dh->dh_key(), dh->dh_key_length() ) ==
        string( (const char *)ka->public_key(), ka->public_key_length() ) )
    {
        // This is the DHi payload
        gotDhi = true;
    }
    else
    {
        // This is the DHr payload
        ka->set_peer_key( dh->dh_key(), dh->dh_key_length() );
    }

    payloads.remove( i );
    i = extract_payload( MIKEYPAYLOAD_DH_PAYLOAD_TYPE );

    if( i.is_null() )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
    }

    if( gotDhi )
    {
        // this one should then be DHr
        ka->set_peer_key( dh->dh_key(), dh->dh_key_length() );
    }
    else
    {
        if( string( (const char *)dh->dh_key(), dh->dh_key_length() ) !=
                string( (const char *)ka->public_key(), ka->public_key_length() ) )
        {
            error = true;
            errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
        }
    }
#undef dh

    if( error )
    {
        throw Mikey_Exception_Message_Content( errorMessage );
    }

// 	ka->computeTgk();

    return NULL;
}


bool Mikey_Message_DHHMAC::authenticate( Key_Agreement  * kaBase )
{
    Key_Agreement_DHHMAC* ka = dynamic_cast<Key_Agreement_DHHMAC*>(kaBase);

    if( !ka )
    {
        throw Mikey_Exception_Message_Content( "Not a DHHMAC keyagreement" );
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

    if( is_initiator_message() || is_responder_message() )
    {
        if( payload->payload_type() != MIKEYPAYLOAD_KEMAC_PAYLOAD_TYPE)
        {
            cerr << "Last payload type = " << (int)payload->payload_type() << endl;
            throw Mikey_Exception( "DHHMAC init did not end with a KEMAC payload" );
        }

        ka->set_csb_id( csb_id() );

        if( !verify_kemac( ka, false ) )
        {
            return true;
        }
        return false;
    }
    else
    {
        throw Mikey_Exception( "Invalide type for a DHHMAC message" );
    }
}

bool Mikey_Message_DHHMAC::is_initiator_message() const
{
    return type() == MIKEY_TYPE_DHHMAC_INIT;
}

bool Mikey_Message_DHHMAC::is_responder_message() const
{
    return type() == MIKEY_TYPE_DHHMAC_RESP;
}

int32_t Mikey_Message_DHHMAC::key_agreement_type() const
{
    return KEY_AGREEMENT_TYPE_DHHMAC;
}
