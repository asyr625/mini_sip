#ifndef COMMAND_STRING_H
#define COMMAND_STRING_H

#include <map>
#include <string>

#include "sobject.h"

#define LIBUTIL_DEFAULT "LIBUTIL_DEFAULT_RVAL_7D1PPX9Q"

class Command_String
{
public:
    Command_String();

    Command_String(const std::string destination_id,
            const std::string operation,
            const std::string parameter="",
            const std::string parameter2="",
            const std::string parameter3="");

    Command_String(const Command_String &c);

    std::string get_destination_id() const;
    void set_destination_id(std::string id);

    std::string get_op() const;
    void set_op(std::string op);

    std::string get_param() const;
    void set_param(std::string param);

    std::string get_param2() const;
    void set_param2(std::string param2);

    std::string get_param3() const;
    void set_param3(std::string param3);

    std::string get_string() const;

    virtual std::string get_mem_object_type() const;

    std::string &operator[](std::string key);

    std::string operator[](std::string key) const;

    std::string get(const std::string &key, const std::string &dfltval=LIBUTIL_DEFAULT) const;
private:
    std::map<std::string, std::string> _keys;
};

#endif // COMMAND_STRING_H
