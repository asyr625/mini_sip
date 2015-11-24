#ifndef ISTRING_H
#define ISTRING_H

#include "sobject.h"
#include "my_types.h"

struct strptr
{
    void *str_start;
    uint32_t n;
};

class String_Atom : public SObject
{
public:
    String_Atom(char *buf, int n);

    String_Atom(const String_Atom &);

    String_Atom(std::string);
    ~String_Atom();

    std::string get_mem_object_type() const;

    char* get_buf() const;
    int get_length() const;
private:
    char *buf;
    int n;
};

class IString : public SObject
{
public:
    IString(SRef<String_Atom*> a);
    IString(SRef<String_Atom*> a, int startIndex, int length);
    IString(const IString &s);

    ~IString();

    std::string get_mem_object_type() const;
    std::string cpp_str();

    struct strptr get_string_pointer() const;

    int get_length(){return n;}
    char* get_buffer(){return atom->get_buf()+start;}
    SRef<IString*> trim();

    SRef<IString*> substr(int i);

    SRef<IString*> substr(int i, int n);
private:
    SRef<String_Atom*> atom;
    int start;
    int n;
};

#endif // ISTRING_H
