#include "mikey.h"

#include "timestamp.h"
#include "dbg.h"

#include "sip_sim.h"

#include "key_agreement_dh.h"
#include "key_agreement_dhhmac.h"
#include "key_agreement_pke.h"
#include "key_agreement_psk.h"
#include "key_agreement_rsar.h"

#include "mikey_exception.h"
#include "mikey_message.h"

#include <string.h>
using namespace std;

#ifdef _WIN32_WCE
#	include "minisip_wce_extra_includes.h"
#endif

#define MIKEY_PROTO_SRTP	0

IMikey_Config::~IMikey_Config()
{
}

Mikey::Mikey( SRef<IMikey_Config*> _config )
    : state(STATE_START), config(_config)
{
}

Mikey::~Mikey()
{
}

bool Mikey::responder_authenticate( const std::string &message, const std::string &peerUri )
{
    set_state( STATE_RESPONDER );

    if(message.substr(0,6) == "mikey ")
    {
        std::string b64Message = message.substr(6, message.length()-6);

        if( message == "" )
            throw Mikey_Exception( "No MIKEY message received" );
        else
        {
            try{
                SRef<Mikey_Message *> init_mes = Mikey_Message::parse(b64Message);
                create_key_agreement( init_mes->key_agreement_type() );
                if( !ka )
                {
                    throw Mikey_Exception( "Can't handle key agreement" );
                }

                ka->set_peer_uri( peerUri );
                ka->set_initiator_data( init_mes );

#ifdef ENABLE_TS
                ts.save( AUTH_START );
#endif
                if( init_mes->authenticate( *ka ) )
                {
                    string msg = "Authentication of the MIKEY init message failed: " + ka->auth_error();
                    throw Mikey_Exception_Authentication( msg.c_str() );
                }

#ifdef ENABLE_TS
                ts.save( TMP );
#endif
                if( config->is_cert_check_enabled() )
                {
                    Peer_Certificates *peers = dynamic_cast<Peer_Certificates*>(*ka);
                    if( peers )
                    {
                        if( peers->control_peer_certificate( ka->peer_uri() ) == 0)
                        {
                            throw Mikey_Exception_Authentication( "Certificate check failed in the incoming MIKEY message" );
                        }
                    }
                }
#ifdef ENABLE_TS
                ts.save( AUTH_END );
#endif
                secured = true;
                set_state( STATE_AUTHENTICATED );
            }
            catch( Certificate_Exception &e )
            {
                // TODO: Tell the GUI
                my_err << "Could not open certificate " << e.what() << endl;
                set_state( STATE_ERROR );
            }
            catch( Mikey_Exception_Unacceptable &exc )
            {
                my_err << "Mikey_Exception caught: "<<exc.what()<<endl;
                //FIXME! send SIP Unacceptable with Mikey Error message
                set_state( STATE_ERROR );
            }
            // Authentication failed
            catch( Mikey_Exception_Authentication &exc )
            {
                my_err << "Mikey_Exception_Authentication caught: "<<exc.what()<<endl;
                //FIXME! send SIP Authorization failed with Mikey Error message
                set_state( STATE_ERROR );
            }
            // Message was invalid
            catch( Mikey_Exception_Message_Content &exc )
            {
                SRef<Mikey_Message *> error_mes;
                my_err << "Mikey_ExceptionMesageContent caught: " << exc.what() << endl;
                error_mes = exc.error_message();
                if( !error_mes.is_null() )
                {
                    //FIXME: send the error message!
                }
                set_state( STATE_ERROR );
            }
            catch( Mikey_Exception & exc )
            {
                my_err << "Mikey_Exception caught: " << exc.what() << endl;
                set_state( STATE_ERROR );
            }
        }
    }
    else
    {
        my_err << "Unknown type of key agreement" << endl;
        secured = false;
        set_state( STATE_AUTHENTICATED );
    }

    return state == STATE_AUTHENTICATED;
}

std::string Mikey::responder_parse()
{
    if( !ka )
    {
        my_err << "Unknown type of key agreement" << endl;
        set_state( STATE_ERROR );
        return "";
    }

    SRef<Mikey_Message *> responseMessage = NULL;
    SRef<Mikey_Message *> initMessage = ka->initiator_data();

    if( initMessage.is_null() )
    {
        my_err << "Uninitialized message, this is a bug" << endl;
        set_state( STATE_ERROR );
        return "";
    }

    try{
#ifdef ENABLE_TS
        ts.save( MIKEY_PARSE_START );
#endif
        add_streams_to_ka();

        responseMessage = initMessage->build_response( *ka );
#ifdef ENABLE_TS
        ts.save( MIKEY_PARSE_END );
#endif
    }
    catch( Certificate_Exception &e )
    {
        // TODO: Tell the GUI
        my_err << "Could not open certificate " << e.what() << endl;
        set_state( STATE_ERROR );
    }
    catch( Mikey_Exception_Unacceptable & exc )
    {
        my_err << "Mikey_Exception caught: "<<exc.what()<<endl;
        //FIXME! send SIP Unacceptable with Mikey Error message
        set_state( STATE_ERROR );
    }
    // Message was invalid
    catch( Mikey_Exception_Message_Content & exc )
    {
        SRef<Mikey_Message *> error_mes;
        my_err << "Mikey_ExceptionMesageContent caught: " << exc.what() << endl;
        error_mes = exc.error_message();
        if( !error_mes.is_null() )
        {
            responseMessage = error_mes;
        }
        set_state( STATE_ERROR );
    }
    catch( Mikey_Exception & exc )
    {
        my_err << "Mikey_Exception caught: " << exc.what() << endl;
        set_state( STATE_ERROR );
    }

    if( !responseMessage.is_null() )
    {
        //my_err << "Created response message" << responseMessage->get_string() << endl;
        return responseMessage->b64_message();
    }
    else
    {
        //my_err << "No response message" << end;
        return string("");
    }
}

std::string Mikey::initiator_create( int type, const std::string &peerUri )
{
    SRef<Mikey_Message *> message;

    set_state( STATE_INITIATOR );

    try{
        create_key_agreement( type );
        if( !ka )
        {
            throw Mikey_Exception( "Can't create key agreement" );
        }

        ka->set_peer_uri( peerUri );
        message = ka->create_message();

        string b64Message = message->b64_message();
        return "mikey " + b64Message;
    }
    catch( Certificate_Exception &e )
    {
        // FIXME: tell the GUI
        my_err << "Could not open certificate " << e.what() << endl;
        set_state( STATE_ERROR );
        return "";
    }
    catch( Mikey_Exception & exc )
    {
        my_err << "Mikey_Exception caught: " << exc.what() << endl;
        set_state( STATE_ERROR );
        return "";
    }
}

bool Mikey::initiator_authenticate( std::string message )
{
    if (message.substr(0,6) == "mikey ")
    {
        // get rid of the "mikey "
        message = message.substr(6,message.length()-6);
        if(message == "")
        {
            my_err << "No MIKEY message received" << endl;
            return false;
        }
        else
        {
            try{
                SRef<Mikey_Message *> resp_mes = Mikey_Message::parse( message );
                ka->set_responder_data( resp_mes );

#ifdef ENABLE_TS
                ts.save( AUTH_START );
#endif
                if( resp_mes->authenticate( *ka ) )
                {
                    throw Mikey_Exception_Authentication( "Authentication of the response message failed" );
                }

#ifdef ENABLE_TS
                ts.save( TMP );
#endif
                if( config->is_cert_check_enabled() )
                {
                    Peer_Certificates *peers = dynamic_cast<Peer_Certificates*>(*ka);
                    if( peers )
                    {
                        if( peers->control_peer_certificate( ka->peer_uri() ) == 0)
                        {
                            throw Mikey_Exception_Authentication( "Certificate control failed" );
                        }
                    }
                }
#ifdef ENABLE_TS
                ts.save( AUTH_END );
#endif
                secured = true;
                set_state( STATE_AUTHENTICATED );
            }
            catch(Mikey_Exception_Authentication &exc)
            {
                my_err << "Mikey_Exception caught: " << exc.what() << endl;
                //FIXME! send SIP Authorization failed with Mikey Error message
                set_state( STATE_ERROR );
            }
            catch(Mikey_Exception_Message_Content &exc)
            {
                SRef<Mikey_Message *> error_mes;
                my_err << "Mikey_Exception_Message_Content caught: " << exc.what() << endl;
                error_mes = exc.error_message();
                if( !error_mes.is_null() )
                {
                    //FIXME: send the error message!
                }
                set_state( STATE_ERROR );
            }

            catch(Mikey_Exception &exc)
            {
                my_err << "Mikey_Exception caught: " << exc.what() << endl;
                set_state( STATE_ERROR );
            }
        }
    }
    else
    {
        my_err << "Unknown key management method" << endl;
        set_state( STATE_ERROR );
    }

    return state == STATE_AUTHENTICATED;
}

std::string Mikey::initiator_parse()
{
    if( !ka )
    {
        my_err << "Unknown type of key agreement" << endl;
        set_state( STATE_ERROR );
        return "";
    }

    SRef<Mikey_Message *> responseMessage = NULL;

    try{
        SRef<Mikey_Message *> initMessage = ka->responder_data();

        if( initMessage.is_null() )
        {
            my_err << "Uninitialized MIKEY init message, this is a bug" << endl;
            set_state( STATE_ERROR );
            return "";
        }

#ifdef ENABLE_TS
        ts.save( MIKEY_PARSE_START );
#endif
        responseMessage = initMessage->parse_response( *ka );
#ifdef ENABLE_TS
        ts.save( MIKEY_PARSE_END );
#endif

    }
    catch( Certificate_Exception &e )
    {
        // TODO: Tell the GUI
        my_err << "Could not open certificate " << e.what() << endl;
        set_state( STATE_ERROR );
    }
    catch( Mikey_Exception_Unacceptable &exc )
    {
        my_err << "Mikey_Exception caught: "<<exc.what()<<endl;
        //FIXME! send SIP Unacceptable with Mikey Error message
        set_state( STATE_ERROR );
    }
    // Message was invalid
    catch( Mikey_Exception_Message_Content &exc )
    {
        SRef<Mikey_Message *> error_mes;
        my_err << "Mikey_ExceptionMesageContent caught: " << exc.what() << endl;
        error_mes = exc.error_message();
        if( !error_mes.is_null() )
        {
            responseMessage = error_mes;
        }
        set_state( STATE_ERROR );
    }
    catch( Mikey_Exception & exc )
    {
        my_err << "Mikey_Exception caught: " << exc.what() << endl;
        set_state( STATE_ERROR );
    }

    if( !responseMessage.is_null() )
    {
        return responseMessage->b64_message();
    }
    else
        return string("");
}

void Mikey::add_streams_to_ka()
{
    Streams::iterator iSender;
    ka->set_cs_id_map_type(HDR_CS_ID_MAP_TYPE_SRTP_ID);
    uint8_t j = 1;
    for( iSender = media_stream_senders.begin(); iSender != media_stream_senders.end(); iSender ++, j++ )
    {
        uint32_t ssrc = *iSender;

        if( is_initiator() )
        {
            uint8_t policyNo = ka->set_default_policy( MIKEY_PROTO_SRTP );
            ka->add_srtp_stream( ssrc, 0/*ROC*/,
                               policyNo );
            /* Placeholder for the receiver to place his SSRC */
            ka->add_srtp_stream( 0, 0/*ROC*/,
                               policyNo );
        }
        else
        {
            ka->set_rtp_stream_ssrc( ssrc, 2*j );
            ka->set_rtp_stream_roc ( 0, 2*j );
        }
    }
}

void Mikey::set_mikey_offer()
{
    SRef<Mikey_Message *> initMessage = ka->initiator_data();
    initMessage->set_offer( *ka );
}

bool Mikey::is_secured() const
{
    return secured && !error();
}

bool Mikey::is_initiator() const
{
    return state == STATE_INITIATOR;
}

bool Mikey::error() const
{
    return state == STATE_ERROR;
}

std::string Mikey::auth_error() const
{
    return ka ? ka->auth_error() : "";
}

SRef<Key_Agreement*> Mikey::get_key_agreement() const
{
    return ka;
}

void Mikey::add_sender( uint32_t ssrc )
{
    media_stream_senders.push_back( ssrc );
}

const std::string &Mikey::peer_uri() const
{
    static std::string empty;

    if( state != STATE_AUTHENTICATED )
        return empty;

    return ka->peer_uri();
}

void Mikey::set_state( State newState )
{
    state = newState;
}

void Mikey::create_key_agreement( int type )
{
    ka = NULL;

    if( !config->is_method_enabled( type ) )
    {
        throw Mikey_Exception( "Cannot handle key agreement method" );
    }

    SRef<Sip_Sim*> sim = config->get_sim();
    SRef<Certificate_Chain*> cert_chain = sim->get_certificate_chain();
    SRef<Certificate_Chain*> peer_chain;
    // 		config->getPeerCertificate();
    SRef<Certificate_Set*> cert_db = sim->get_cas();
    const byte_t* psk = config->get_psk();
    size_t psk_len = config->get_psk_length();

    switch( type )
    {
    case KEY_AGREEMENT_TYPE_DH:
    {
        if ( cert_chain.is_null() )
        {
            throw Mikey_Exception( "No certificate provided for DH key agreement" );
        }

        Key_Agreement_DH *kaDH = new Key_Agreement_DH( sim );

        if( is_initiator() )
        {
            kaDH->set_group( DH_GROUP_OAKLEY5 );
        }

        ka = kaDH;
        break;
    }
    case KEY_AGREEMENT_TYPE_PK:
        if( cert_chain.is_null() )
        {
            throw Mikey_Exception( "No certificate provided for Public-Key method" );
        }

        if( is_initiator() )
        {
            if( peer_chain.is_null() )
            {
                throw Mikey_Exception( "No peer certificate provided for Public-Key init" );
            }
            ka = new Key_Agreement_PKE( cert_chain, peer_chain );
        }
        else
        {
            if( cert_db.is_null() )
            {
                throw Mikey_Exception( "No CA db provided for Public-Key responce" );
            }
            ka = new Key_Agreement_PKE( cert_chain, cert_db );
        }
        break;
    case KEY_AGREEMENT_TYPE_RSA_R:
        if( cert_chain.is_null() )
        {
            throw Mikey_Exception( "No certificate provided for RSA-R method" );
        }

        if( cert_db.is_null() )
        {
            throw Mikey_Exception( "No CA db provided for RSA-R method" );
        }

        ka = new Key_Agreement_RSAR( cert_chain, cert_db );
        break;
    case KEY_AGREEMENT_TYPE_PSK:
    case KEY_AGREEMENT_TYPE_DHHMAC:
        if (!psk || psk_len <= 0)
        {
            throw Mikey_Exception( "No pre-shared key provided" );
        }

        if( type == KEY_AGREEMENT_TYPE_PSK )
        {
            ka = new Key_Agreement_PSK(psk, (int)psk_len);
        }
        else
        {
            Key_Agreement_DHHMAC *kaDH =
                    new Key_Agreement_DHHMAC(psk, (int)psk_len);
            if( is_initiator() )
            {
                kaDH->set_group( DH_GROUP_OAKLEY5 );
            }
            ka = kaDH;
        }
        break;
    default:
        throw Mikey_Exception_Unimplemented( "Unsupported type of KA" );
    }

    if( ka->type() != KEY_AGREEMENT_TYPE_DHHMAC )
    {
        // Generate TGK for PSK, PK and RSA-R
        Key_Agreement_PSK* pskKa = dynamic_cast<Key_Agreement_PSK*>(*ka);
        if( pskKa )
        {
            cerr << "Generate Tgk" << endl;
            pskKa->generate_tgk();
        }
    }

    ka->set_uri( config->get_uri() );

    if( is_initiator() )
    {
        add_streams_to_ka();
    }
}
