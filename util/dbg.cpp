#include <stdio.h>
#include <stdlib.h>

#include "dbg.h"
#include "string_utils.h"

Mutex Dbg::_lock = Mutex();

Dbg my_out("mout");
Dbg my_err("merr", false);
Dbg my_dbg("my_dbg", true, false);

bool output_state_machine_debug = false;

Dbg::Dbg(std::string name, bool error_output, bool enabled)
    : _name(name),
      _error_out(error_output),
      _enabled(enabled),
      _debug_handler(NULL),
      _default_include(true),
      _filter_blocking(false),
      _print_name(false),
      _print_timestamp(false)
{
}

Dbg::~Dbg()
{
}

void Dbg::set_print_stream_name(bool b)
{
    _print_name = b;
}

void Dbg::set_print_timestamp(bool b)
{
    _print_timestamp = b;
}

void Dbg::set_enabled(bool enabled)
{
    _enabled = enabled;
}

bool Dbg::get_enabled()
{
    return _enabled;
}


Dbg &Dbg::operator<<( const std::string& )
{
    return *this;
}

Dbg &Dbg::operator<<( std::ostream&(*arg)(std::ostream&) )
{
    (*this) << "\n";
    _lock.lock();
    _cur_class = "";
    update_filter();
    _lock.unlock();
    return (*this);
}

Dbg &Dbg::operator<<( int i)
{
    return (*this)<<itoa(i);
}

Dbg &Dbg::operator<<( unsigned int i)
{
    return (*this)<<itoa(i);
}

Dbg &Dbg::operator<<( long long ll)
{
    return (*this)<<itoa(ll);
}

Dbg &Dbg::operator<<( uint64_t ll)
{
    return (*this)<<itoa(ll);
}

Dbg &Dbg::operator<<( const char c)
{
    return (*this)<<std::string("") + c;
}

Dbg &Dbg::operator<<( const char *c)
{
    return (*this)<<std::string(c);
}

Dbg &Dbg::operator<<( void *p)
{
    return (*this)<<(uint64_t)p;
}


void Dbg::set_external_handler(Dbg_Handler * dbg_handler)
{
    _debug_handler = dbg_handler;
}


static bool inSet( std::set< std::string > &set, std::string filter){
    std::set< std::string >::const_iterator i;
    for (i=set.begin() ; i!=set.end(); i++){
        std::string setfilt = (*i);
        if ( setfilt[0]=='/' )
            setfilt = setfilt.substr(1);
        if ( filter.substr(0,setfilt.size()) == setfilt ){
            return true;
        }
    }
    return false;
}


void Dbg::update_filter()
{
    if (inSet( _exclude_set, _cur_class))
        _filter_blocking=true;
    else if (inSet( _include_set, _cur_class))
        _filter_blocking=false;
    else
        _filter_blocking = ! _default_include;
}

Dbg& Dbg::operator()(std::string oClass)
{
    _lock.lock();
    _cur_class = oClass;
    update_filter();
    _lock.unlock();
    return *this;
}


static void remove_starting_with( std::set< std::string > &set, std::string filter )
{
    std::set< std::string >::iterator i;
    if (filter.size()<=0)
        return;

    if (filter[0]=='/')
        filter = filter.substr(1);

    if (filter[filter.size()-1]=='/')
        filter = filter.substr(0, filter.size()-1);

    for (i=set.begin() ; i!=set.end(); i++){
        std::string tmp = (*i);
        if ( tmp[0]=='/' )
            tmp = tmp.substr(1);
        if ( tmp.substr( 0, filter.size() ) == filter ){
            set.erase(i);
            i=set.begin();
        }
        if (i==set.end())
            break;
    }
}

void Dbg::include(std::string s)
{
    s = trim(s);
    _lock.lock();
    _enabled=true;
    if (s == "" || s == "/")
    {
        _default_include=true;
        _include_set.clear();
        _exclude_set.clear();
    }
    else
    {
        _include_set.insert(s);
        remove_starting_with(_exclude_set, s);
    }
    update_filter();
    _lock.unlock();
}

void Dbg::exclude(std::string s)
{
    s = trim(s);
    _lock.lock();
    if ( s == "" || s == "/" )
    {
        _default_include=false;
        _include_set.clear();
        _exclude_set.clear();
    }
    else
    {
        _exclude_set.insert(s);
        remove_starting_with(_include_set, s);
    }
    update_filter();
    _lock.unlock();
}
