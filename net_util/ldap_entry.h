#ifndef LDAP_ENTRY_H
#define LDAP_ENTRY_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <map>

#include "sobject.h"
#include "ldap_exception.h"

class Ldap_Entry_Binary_Value : public SObject
{
public:
    Ldap_Entry_Binary_Value(char* v, int l) : length(l)
    {
        value = new char[length];
        memcpy(value, v, length);
    }
    ~Ldap_Entry_Binary_Value()
    {
        delete[] value;
    }
    char* value;
    int length;
};

class Ldap_Entry_Binary_Pair_Value : public SObject
{
public:
    Ldap_Entry_Binary_Pair_Value(SRef<Ldap_Entry_Binary_Value*> first, SRef<Ldap_Entry_Binary_Value*> second)
    {
        this->first = first;
        this->second = second;
    }
    SRef<Ldap_Entry_Binary_Value*> first;
    SRef<Ldap_Entry_Binary_Value*> second;
};

class Ldap_Entry : public SObject
{
public:
    Ldap_Entry(void* ld, void* entry);
    std::string get_attr_value_string(std::string attr) throw (Ldap_Attribute_Not_Found_Exception);
    std::vector< SRef<Ldap_Entry_Binary_Value*> > get_attr_values_binary(std::string attr) throw (Ldap_Attribute_Not_Found_Exception);
    std::vector< SRef<Ldap_Entry_Binary_Value*> > get_attr_values_binary_pairs(std::string attr) throw (Ldap_Attribute_Not_Found_Exception);

    std::vector<std::string> get_attr_values_strings(std::string attr) throw (Ldap_Attribute_Not_Found_Exception);

    std::vector<std::string> get_attr_names();

    bool has_attribute(std::string attr);
private:
    std::map<std::string, std::vector<SRef<Ldap_Entry_Binary_Value*> > > values_binary;
    std::map<std::string, std::vector<std::string> > values_strings;
};

#endif // LDAP_ENTRY_H
