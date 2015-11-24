#include <iostream>
#include "sip_layer_dialog.h"

#include "sip_dialog.h"

#include "sip_command_dispatcher.h"

Sip_Layer_Dialog::Sip_Layer_Dialog(SRef<Sip_Command_Dispatcher*> dispatcher)
    : _dispatcher(dispatcher)
{
}

Sip_Layer_Dialog::~Sip_Layer_Dialog()
{
    std::map<std::string, SRef<Sip_Dialog*> >::iterator iter;
    std::map<std::string, SRef<Sip_Dialog*> >::iterator iter_end = _dialogs.end();

    _dialog_list_lock.lock();
    for(iter = _dialogs.begin(); iter != iter_end; ++iter )
    {
        (*iter).second->free_state_machine();
    }
    _dialog_list_lock.unlock();
}

void Sip_Layer_Dialog::set_default_dialog_command_handler(SRef<Sip_Default_Handler*> cb)
{
    _default_handler = cb;
}

SRef<Sip_Default_Handler*> Sip_Layer_Dialog::get_default_dialog_command_handler()
{
    return _default_handler;
}

void Sip_Layer_Dialog::add_dialog(SRef<Sip_Dialog*> d)
{
    my_assert(d);
    my_assert(d->_dialog_state._call_id != "");

    _dialog_list_lock.lock();
    _dialogs[d->_dialog_state._call_id] = d;
    _dialog_list_lock.unlock();
}

SRef<Sip_Dialog*> Sip_Layer_Dialog::get_dialog(std::string call_id)
{
    SRef<Sip_Dialog*> ret;
    std::map<std::string, SRef<Sip_Dialog*> >::iterator iter = _dialogs.find(call_id);

    if( iter != _dialogs.end() )
        ret = (*iter).second;
    return ret;
}

bool Sip_Layer_Dialog::remove_dialog(std::string call_id)
{
    SRef<Sip_Dialog*> ret = _dialogs[call_id];
    if( ret )
    {
        ret->free();
        ret->free_state_machine();
    }

    size_t n = _dialogs.erase(call_id);
#ifdef DEBUG_OUTPUT
    if( n != 1)
    {
        my_err << "WARNING: dialogs.erase should return 1, but returned "<< (short)n << std::endl;
    }
#endif
    return n == 1;
}

std::list<SRef<Sip_Dialog*> > Sip_Layer_Dialog::get_dialogs()
{
    std::list<SRef<Sip_Dialog*> > ret;

    std::map<std::string, SRef<Sip_Dialog*> >::iterator iter;
    std::map<std::string, SRef<Sip_Dialog*> >::iterator iter_end = _dialogs.end();

    _dialog_list_lock.lock();
    for( iter = _dialogs.begin(); iter != iter_end; ++iter )
    {
        ret.push_back((*iter).second);
        my_assert((*iter).second);
    }
    _dialog_list_lock.unlock();
    return ret;
}

bool Sip_Layer_Dialog::handle_command(const Sip_SMCommand &c)
{
    my_assert( c.get_destination() == Sip_SMCommand::dialog_layer );

#ifdef DEBUG_OUTPUT
    my_dbg("signaling/sip") << "SipLayerDialog: got command: "<< c <<std::endl;
#endif

    std::string cid = c.get_destination_id();

    try
    {
        SRef<Sip_Dialog *> dialog;
        if( cid.size() > 0 )
        {
            _dialog_list_lock.lock();
            std::map<std::string, SRef<Sip_Dialog*> >::iterator d = _dialogs.find(cid);
            if( d != _dialogs.end() )
            {
                dialog = (*d).second;
            }
            _dialog_list_lock.unlock();

            if ( dialog && dialog->handle_command(c) )
                return true;
        }
        else
        {
            std::map<std::string, SRef<Sip_Dialog*> >::iterator iter;
            std::map<std::string, SRef<Sip_Dialog*> >::iterator iter_end = _dialogs.end();
            _dialog_list_lock.lock();
            for( iter = _dialogs.begin(); iter != iter_end; ++iter )
            {
                dialog = (*iter).second;
                _dialog_list_lock.unlock();

                if ( dialog->handle_command(c) )
                {
                    return true;
                }
                _dialog_list_lock.lock();
            }
            _dialog_list_lock.unlock();
        }

        if( _default_handler )
        {
            //std::cerr << "SipLayerDialog: No dialog handled the message - sending to default handler"<<std::endl;
            return _default_handler->handle_command(c);
        }
        else
        {
            std::cerr << "ERROR: libmsip: SipLayerDialog::handleCommand: No default handler for dialog commands set!"<<std::endl;
            return false;
        }

    }
    catch(Exception &e)
    {
        std::cerr << "SipLayerDialog: caught exception: "<< e.what() << std::endl;
        return false;
    }
}


