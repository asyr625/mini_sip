#include <stdlib.h>
#include <iostream>
#include "sip_dialog.h"
#include "sip_stack_internal.h"
#include "sip_command_dispatcher.h"
#include "sip_command_string.h"
#include "sip_layer_transaction.h"
#include "string_utils.h"
#include "sip_authentication_digest.h"

#include "sip_header_from.h"
#include "sip_header_to.h"
#include "sip_header_rack.h"
#include "sip_header_call_id.h"
#include "sip_header_cseq.h"
#include "sip_header_max_forwards.h"
#include "sip_header_rseq.h"
#include "sip_header_refer_to.h"
#include "sip_header_contact.h"
#include "sip_header_proxy_authenticate.h"
#include "sip_header_www_authenticate.h"

using namespace std;

bool Sip_Dialog_State::update_state( SRef<Sip_Request*> inv )
{
    if( _route_set.size() == 0 )
    {
        //merr << "dialog state has NO routeset" << end;
        /*SipMessage::getRouteSet returns the top-to-bottom ordered
        Record-Route headers.
        As a server, we can use this directly as route set (no reversing).
        */
        _route_set = inv->get_route_set();
        for( std::list<string>::iterator iter = _route_set.begin(); iter != _route_set.end(); iter++ )
        {
            //my_err << "Sip_Dialog_State:inv:  " << (*iter) << end;
        }
    }
    else
    {
        //my_err << "CESC: dialog state has a routeset" << end;
    }

    _is_early = false;
    SRef<Sip_Header_Value_Contact *> c = inv->get_header_value_contact();
    if( c )
    {
        _remote_target = c->get_uri().get_string();
    }
    _remote_uri = inv->get_header_value_from()->get_uri().get_string();
    _local_uri = inv->get_header_value_to()->get_uri().get_string();

    _remote_tag = inv->get_header_value_from()->get_parameter("tag");
    //The local tag needs to be computed locally ... for voip dialog,
    // done in the constructor

    _remote_seq_no = inv->get_cseq();
    //the callid already has a value, given at the constructor of
    //the dialog ... we repeat, just in case
    _call_id = inv->get_call_id();
    _secure = false; //FIXME: check if secure call ...
    _is_established = true;
    return true;
}

bool Sip_Dialog_State::update_state( SRef<Sip_Response*> resp)
{
    if( resp->get_status_code() < 101 || resp->get_status_code() >= 300 )
    {
        return false;
    }

    const string toTag = resp->get_header_value_to()->get_parameter("tag");

    if( resp->get_status_code() < 200 )
    {
        if( toTag == "" )
        {
            return false;
        }
        if( _is_established && toTag != _remote_tag )
        {
            cerr << "SipDialogState: Multiple early dialogs unsupported" << endl;
            return false;
        }

        _is_early = true;
    }
    else
    {
        _is_early = false;
    }

    //from here on, only 101-199 with to tag and 2xx responses
    //merr << "SipDialogState: updating state ... " << end;
    if( _route_set.size() == 0 )
    {
        //merr << "dialog state has NO routeset" << end;
        /*SipMessage::getRouteSet returns the top-to-bottom ordered
        Record-Route headers.
        As a client, we must reverse this directly as route set
        */
        _route_set = resp->get_route_set();
        _route_set.reverse();
        for( list<string>::iterator iter = _route_set.begin(); iter != _route_set.end(); iter++ )
        {
            //my_err << "SipDialogState:resp:  " << (*iter) << end;
        }
    }
    else
    {
        //my_err << "dialog state has a routeset" << end;
    }

    SRef<Sip_Header_Value_Contact *> c = resp->get_header_value_contact();
    if( c )
    {
        _remote_target = c->get_uri().get_string();
    }

    if( _is_established && ( _is_early || toTag == _remote_tag ) )
    {
        // Update only route set and target for an existing dialog
        return true;
    }

    _remote_uri = resp->get_header_value_to()->get_uri().get_string();
    _local_uri = resp->get_header_value_from()->get_uri().get_string();

    _remote_tag = toTag;
    _local_tag = resp->get_header_value_from()->get_parameter("tag");

    _seq_no = resp->get_cseq();
    //the callid already has a value, given at the constructor of
    //the dialog ... we repeat, just in case
    _call_id = resp->get_call_id();
    _secure = false; //FIXME: check if secure call ...
    _is_established = true;
    return true;
}

std::string Sip_Dialog_State::get_remote_target()
{
    if( _remote_target != "" )
    {
        return _remote_target;
    }
    else
    {
        //my_err << "Sip_Dialog_State::get_remote_target : remote target empty! returning remote uri .." << end;
        return _remote_uri;
    }
}

Sip_Dialog::Sip_Dialog(SRef<Sip_Stack*> stack, SRef<Sip_Identity*> identity, std::string call_id)
    : State_Machine<Sip_SMCommand, std::string>(stack->get_timeout_provider())
{
    my_assert(stack);
    _call_config = new Sip_Dialog_Config(stack);

    if( call_id == "" )
        _dialog_state._call_id = itoa(rand()) + "@" + stack->get_stack_config()->external_contact_ip;
    else
        _dialog_state._call_id = call_id;

    if( identity )
        _call_config->use_identity(identity);

    this->_dialog_state._seq_no = 100 * (rand() % 9 + 1);
    _dialog_state._local_tag = itoa(rand());
    _dialog_state._remote_seq_no = -1;
    _dialog_state._secure = false;

    _dialog_state._is_early = false;
    _dialog_state._is_established  = false;
    _dialog_state._is_terminated  = false;
    _dialog_state._rseq_no = (uint32_t)-1;
}

Sip_Dialog::~Sip_Dialog()
{
}

void Sip_Dialog::free()
{
}

SRef<Sip_Dialog_Config*> Sip_Dialog::get_dialog_config()
{
    return _call_config;
}

std::list<std::string> Sip_Dialog::get_route_set()
{
    return _dialog_state._route_set;
}


std::string Sip_Dialog::get_call_id()
{
    return _dialog_state._call_id;
}


void Sip_Dialog::handle_timeout(const std::string &c)
{
    Sip_SMCommand cmd(Command_String(_dialog_state._call_id, c),
                      Sip_SMCommand::dialog_layer,
                      Sip_SMCommand::dialog_layer);
    (*(SRef<Sip_Stack_Internal*>*)_call_config->_sip_stack->_sip_stack_internal)->get_dispatcher()->enqueue_timeout(this, cmd);
}


void Sip_Dialog::signal_if_no_transactions()
{
    if( _dialog_state._is_terminated || get_current_state_name() == "termwait" )
    {
        std::list<SRef<Sip_Transaction*> > t = get_transactions();

        if( t.size() == 0 )
        {
            Sip_SMCommand cmd(Command_String(_dialog_state._call_id, Sip_Command_String::no_transactions),
                              Sip_SMCommand::dialog_layer,
                              Sip_SMCommand::dialog_layer);
            (*(SRef<Sip_Stack_Internal*>*)_call_config->_sip_stack->_sip_stack_internal)->get_dispatcher()->handle_command(cmd);
        }
    }
}

void Sip_Dialog::add_route( SRef<Sip_Request *> req )
{
    if( !_dialog_state._is_established || req->get_type() == "cancel" )
    {
        const std::list<Sip_Uri>& route_set = get_dialog_config()->_sip_identity->get_route_set();
        req->add_routes(route_set);
    }
    else if( _dialog_state._route_set.size() > 0 )
    {
        //add route headers, if needed
        SRef<Sip_Header_Value_Route *> rset = new Sip_Header_Value_Route (_dialog_state._route_set);
        req->add_header(new Sip_Header(*rset) );
    }
    else
    {

    }
}

std::list<SRef<Sip_Transaction*> > Sip_Dialog::get_transactions()
{
    return (*(SRef<Sip_Stack_Internal*>*)_call_config->_sip_stack->_sip_stack_internal)->get_dispatcher()->get_layer_transaction()->get_transactions_with_call_id(get_call_id());
}

bool Sip_Dialog::handle_command(const Sip_SMCommand &command)
{
    my_dbg("signaling/sip") << "SipDialog("<<get_mem_object_type()<<")::handleCommand got command "<< command << "(" << get_name() << ")" << std::endl;

    if(! (command.get_destination() == Sip_SMCommand::dialog_layer) )
    {
        my_dbg("signaling/sip") << "SipDialog::handleCommand: returning false based on command destination"<< std::endl;
        return false;
    }

    if( command.get_type() == Sip_SMCommand::COMMAND_PACKET && _dialog_state._call_id != ""
            && _dialog_state._call_id != command.get_command_packet()->get_call_id() )
    {
        my_dbg("signaling/sip") << "SipDialog: denying command based on destination id"<< std::endl;
        return false;
    }

    my_dbg("signaling/sip") << "SipDialog::handleCommand: sending command to state machine" << std::endl;

    bool ret = State_Machine<Sip_SMCommand,std::string>::handle_command(command);

    my_dbg("signaling/sip") << "SipDialog::handleCommand returning "<< ret << std::endl;

    return ret;
}

SRef<Sip_Stack*> Sip_Dialog::get_sip_stack()
{
    return _call_config->_sip_stack;
}

SRef<Sip_Request*> Sip_Dialog::create_sip_message( const std::string &method )
{
    return create_sip_message_seq(method, _dialog_state._seq_no);
}

SRef<Sip_Request*> Sip_Dialog::create_sip_message_seq( const std::string &method, int seq_no )
{
    SRef<Sip_Request*> req = new Sip_Request(method, _dialog_state.get_remote_target());
    req->set_uri(_dialog_state.get_remote_target());

    req->add_header( new Sip_Header(new Sip_Header_Value_Max_Forwards(70)) );

    Sip_Uri from_uri( _dialog_state._local_uri );
    SRef<Sip_Header_Value_From*> from = new Sip_Header_Value_From( from_uri );
    from->set_parameter( "tag", _dialog_state._local_tag );
    req->add_header(new Sip_Header( *from ));

    Sip_Uri to_uri( _dialog_state._remote_uri );
    SRef<Sip_Header_Value_To*> to = new Sip_Header_Value_To( to_uri );
    to->set_parameter( "tag", _dialog_state._remote_tag );
    req->add_header(new Sip_Header( *to ));

    req->add_header(new Sip_Header(new Sip_Header_Value_CSeq( method, seq_no)));

    req->add_header(new Sip_Header(new Sip_Header_Value_Call_ID( _dialog_state._call_id)));

    add_route( req );

    if( method != "ACK" && method != "CANCEL" )
        add_authorizations( req );

    return req;
}

SRef<Sip_Request*> Sip_Dialog::create_sip_message_ack( SRef<Sip_Request *> orig_req )
{
    SRef<Sip_Request*> ack = create_sip_message_seq("ACK", orig_req->get_cseq());

    int no_headers = orig_req->get_no_headers();
    for(int i = 0; i < no_headers; ++i)
    {
        SRef<Sip_Header*> header = orig_req->get_header_no(i);
        int header_type = header->get_type();
        switch (header_type)
        {
        case SIP_HEADER_TYPE_AUTHORIZATION:
        case SIP_HEADER_TYPE_PROXYAUTHORIZATION:
            ack->add_header( header );
            break;
        default:
            /*don't copy other headers*/
            break;
        }
    }
    return ack;
}

SRef<Sip_Request*> Sip_Dialog::create_sip_message_bye()
{
    return create_sip_message("BYE");
}

SRef<Sip_Request*> Sip_Dialog::create_sip_message_prack( SRef<Sip_Response*> resp )
{
    SRef<Sip_Header_Value *> value = resp->get_header_value_no( SIP_HEADER_TYPE_RSEQ, 0 );

    if( !value )
    {
        my_dbg("signaling/sip") << "SipDialog: Missing RSeq in response" << std::endl;
        return NULL;
    }

    SRef<Sip_Request*> req = create_sip_message("PRACK");
    SRef<Sip_Header_Value_RSeq *> rseq = dynamic_cast<Sip_Header_Value_RSeq*>( *value );
    /* Add RAck header */
    req->add_header( new Sip_Header( new Sip_Header_Value_RAck( resp->get_cseq_method(), rseq->get_rseq(), resp->get_cseq() )));

    return req;
}

SRef<Sip_Request*> Sip_Dialog::create_sip_message_refer( const std::string &referred_uri )
{
    SRef<Sip_Request*> req = create_sip_message("REFER");

    /* Add the Refer-To: header */
    req->add_header(new Sip_Header(new Sip_Header_Value_Refer_To(referred_uri)));
    return req;
}

SRef<Sip_Response*> Sip_Dialog::create_sip_response( SRef<Sip_Request*> req, int status, const std::string &reason )
{
    SRef<Sip_Response*> resp = new Sip_Response( status, reason, *req );
    // FIXME don't change the To tag if it's already present in the request.
    resp->get_header_value_to()->set_parameter("tag",_dialog_state._local_tag);
    return resp;
}

void Sip_Dialog::send_sip_message( SRef<Sip_Message*> msg, int queue )
{
    Sip_SMCommand cmd(*msg, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
    (*(SRef<Sip_Stack_Internal*>*)_call_config->_sip_stack->_sip_stack_internal)->get_dispatcher()->enqueue_command(cmd, queue);
}

void Sip_Dialog::clear_authentications()
{
    _dialog_state._auths.clear();
}

bool Sip_Dialog::update_authentication(SRef<Sip_Header_Value_Proxy_Authenticate*> auth)
{
    bool authenticationInfoHeaderValue = auth->get_auth_method().empty();
    if(str_case_cmp(auth->get_auth_method().c_str(), "DIGEST") && !auth->get_auth_method().empty() /* Authentication-Info header value has no authMethod */)
        return false;

    bool changed = false;

    SRef<Sip_Authentication_Digest*> challenge;
    challenge = new Sip_Authentication_Digest( auth );

    bool found = false;
    list<SRef<Sip_Authentication_Digest*> >::iterator j;

    for ( j = _dialog_state._auths.begin(); j != _dialog_state._auths.end(); j++ )
    {
        SRef<Sip_Authentication_Digest*> item = *j;

        if( item->get_realm() == challenge->get_realm() )
        {
            item->update( auth );

            if( item->get_stale() || auth->get_auth_method().empty() /* Authentication-Info header value has no stale */){
                changed = true;
            }
            else
            {
                // Clear invalid credential
                item->set_credential( NULL );
            }
            found = true;
            break;
        }
    }

    if( !found )
    {
        _dialog_state._auths.push_back( challenge );

        SRef<Sip_Credential*> cred =
                get_dialog_config()->_sip_identity->get_credential();

        challenge->set_credential( cred );
        changed = true;
    }
    return changed;
}

bool Sip_Dialog::update_authentications( SRef<Sip_Response*> resp )
{
    bool changed = false;

    int i;
    for( i = 0;; i++ )
    {
        SRef<Sip_Header_Value_WWW_Authenticate*> auth;
        auth = resp->get_header_value_wwwauthenticate( i );

        if( !auth )
            break;

        changed |= update_authentication(*auth);
    }

    for( i = 0;; i++ )
    {
        SRef<Sip_Header_Value_Proxy_Authenticate*> auth;
        auth = resp->get_header_value_proxy_authenticate( i );

        if( !auth )
            break;

        changed |= update_authentication(auth);
    }

    for( i = 0;; i++ )
    {
        SRef<Sip_Header_Value_Authentication_Info *> auth;
        auth = resp->get_header_value_authentication_info( i );

        if( !auth )
            break;

        changed |= update_authentication(*auth);
    }
    return changed;
}

void Sip_Dialog::add_authorizations( SRef<Sip_Request*> req )
{
    list<SRef<Sip_Authentication_Digest*> >::iterator j;

    for ( j = _dialog_state._auths.begin(); j != _dialog_state._auths.end(); j++ )
    {
        SRef<Sip_Authentication_Digest*> digest = *j;

        SRef<Sip_Header_Value_Authorization*> authHeader = digest->create_authorization( req );
        req->add_header( new Sip_Header( *authHeader ) );
    }
}

const std::string & Sip_Dialog::find_unauthenticated_realm() const
{
    static const string empty;
    list<SRef<Sip_Authentication_Digest*> >::const_iterator j;

    for ( j = _dialog_state._auths.begin(); j != _dialog_state._auths.end(); j++ )
    {
        SRef<Sip_Authentication_Digest*> digest = *j;

        if( digest->get_credential().is_null() )
        {
            return digest->get_realm();
        }
    }

    return empty;
}
bool Sip_Dialog::add_credential( SRef<Sip_Credential*> credential )
{
    bool found = false;
    list<SRef<Sip_Authentication_Digest*> >::iterator j;

    for ( j = _dialog_state._auths.begin();
          j != _dialog_state._auths.end(); j++ )
    {
        SRef<Sip_Authentication_Digest*> digest = *j;

        if( digest->get_credential().is_null() )
        {
            // Needs update
            if( credential->get_realm() == "" || credential->get_realm() == digest->get_realm() )
            {
                digest->set_credential( credential );
                found = true;
            }
        }
    }
    return found;
}

std::string Sip_Dialog::get_dialog_debug_string()
{
    string ret;
    list <TPRequest<string,SRef<State_Machine<Sip_SMCommand,string>*> > > torequests =
        _call_config->_sip_stack->get_timeout_provider()->get_timeout_requests();

    ret = get_name() + "   State: " + get_current_state_name()+"\n";


    ret+= "        SipDialogState: \n"
          "            secure=" + string(_dialog_state._secure?"true":"false")
            + string("; callId=") + _dialog_state._call_id
            + "; localTag=" + _dialog_state._local_tag
            + "; remoteTag=" + _dialog_state._remote_tag
            + "; seqNo=" + itoa(_dialog_state._seq_no)
            + "; remoteSeqNo=" + itoa(_dialog_state._remote_seq_no)
            + "; remoteUri=" + _dialog_state._remote_uri
            + "; remoteTarget=" + _dialog_state._remote_target
            + string("; isEarly=") + string(_dialog_state._is_early?"true":"false")
            + string("; isEstablished=") + string(_dialog_state._is_established?"true":"false")
            + string("; isTerminated=") + string(_dialog_state._is_terminated?"true":"false")
            + "\n";
    ret+= "            route_set: ";

    list<string>::iterator i;
    for (i=_dialog_state._route_set.begin(); i!= _dialog_state._route_set.end(); i++)
    {
        if (i!=_dialog_state._route_set.begin())
            ret += ",";
        ret+= *i;
    }
    ret+="\n";

    ret+= "        Identity: \n";
    ret+= "            " + get_dialog_config()->_sip_identity->get_debug_string();
    ret+="\n";
    return ret;
}

std::list<std::string> Sip_Dialog::get_required_unsupported( SRef<Sip_Message*> msg )
{
    list<string> required = msg->get_required();
    list<string> unsupported;

    list<string>::iterator i;
    list<string>::iterator last = required.end();

    for( i = required.begin(); i != last; i++ )
    {
        const string &req = *i;

        if( !_call_config->_sip_stack->supports( req ) )
            unsupported.push_back( req );
    }

    return unsupported;
}

bool Sip_Dialog::reject_unsupported( SRef<Sip_Request*> req )
{
    list<string> unsuppored = get_required_unsupported( *req );

    if( !unsuppored.empty() )
    {
        // Send 420 Bad Extension
        SRef<Sip_Response*> resp =
            create_sip_response( req, 420, "Bad Extension" );

        resp->add_unsupported( unsuppored );
        send_sip_message( *resp );
        return true;
    }

    return false;
}
