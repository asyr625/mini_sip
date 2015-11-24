#include "command_string.h"
#include "exception.h"

Command_String::Command_String()
{
}


Command_String::Command_String(const std::string destination_id,
        const std::string operation,
        const std::string parameter,
        const std::string parameter2,
        const std::string parameter3)
{
    _keys["destination_id"] = destination_id;
    _keys["op"] = operation;
    _keys["param"] = parameter;
    _keys["param2"] = parameter2;
    _keys["param3"] = parameter3;
}

Command_String::Command_String(const Command_String &c)
{
    this->_keys = c._keys;
}

std::string Command_String::get_destination_id() const
{
    return get("destination_id");
}
void Command_String::set_destination_id(std::string id)
{
    _keys["destination_id"] = id;
}

std::string Command_String::get_op() const
{
    return get("op");
}
void Command_String::set_op(std::string op)
{
    _keys["op"] = op;
}

std::string Command_String::get_param() const
{
    return get("param");
}
void Command_String::set_param(std::string param)
{
    _keys["param"] = param;
}

std::string Command_String::get_param2() const
{
    return get("param2");
}
void Command_String::set_param2(std::string param2)
{
    _keys["param2"] = param2;
}

std::string Command_String::get_param3() const
{
    return get("param3");
}

void Command_String::set_param3(std::string param3)
{
    _keys["param3"] = param3;
}

std::string Command_String::get_mem_object_type() const
{
    return "CommandString";
}

std::string Command_String::get_string() const
{
    std::string ret;
    std::map<std::string,std::string>::const_iterator it = _keys.begin();

    ret = "op=" + get_op() + "; ";

    for( it = _keys.begin(); it != _keys.end(); ++it )
    {
        if( it->first != "op" && it->second != "" )
            ret += it->first + "=" + it->second + "; ";
    }
    return ret;
}

std::string &Command_String::operator[](std::string key)
{
    return _keys[key];
}

std::string Command_String::operator[](std::string key) const
{
    if ( _keys.count(key) <= 0 )
        return "";
    std::map<std::string, std::string>::const_iterator i = _keys.find(key);
    return (*i).second;
}

std::string Command_String::get(const std::string &key, const std::string &dfltval) const
{
    std::map<std::string,std::string>::const_iterator it = _keys.find(key);
    if( it == _keys.end() )
    {
        if ( dfltval == LIBUTIL_DEFAULT )
            throw Exception(("Key not found ("+key+")").c_str());
        else
            return dfltval;
    }
    return it->second;
}
