#include "istring.h"
#include "my_assert.h"
#include <string.h>

#include<iostream>

using namespace std;

String_Atom::String_Atom(char *b, int len)
    : buf(b),
      n(len)
{
}

String_Atom::String_Atom(std::string s)
{
#ifdef _MSC_VER
    buf = _strdup(s.c_str());
#else
    buf = strdup(s.c_str());
#endif
    n = (int)s.length();
}

String_Atom::String_Atom(const String_Atom &a)
    :n(a.n)
{
    buf = new char[n];
    memcpy(buf,a.buf,n);
}

String_Atom::~String_Atom()
{
    my_assert(buf);
    delete []buf;
    buf = NULL;
    n = 0;
}

std::string String_Atom::get_mem_object_type() const
{
    return "String_Atom";
}

char *String_Atom::get_buf() const
{
    return buf;
}

int String_Atom::get_length() const
{
    return n;
}


IString::IString(SRef<String_Atom*> a)
    : atom(a),
      start(0)
{
    n = a->get_length();
}

IString::IString(SRef<String_Atom*> a, int startIndex, int length)
    : atom(a),
      start(startIndex),
      n(length)
{
    my_assert( startIndex+length <= a->get_length() );
}

IString::IString(const IString &s)
{
    atom = s.atom;
    start = s.start;
    n = s.n;
}

IString::~IString()
{
}

std::string IString::get_mem_object_type() const
{
    return "IString";
}


SRef<IString*> IString::trim()
{
    int newstart, newn;
    int oldend = start+n;
    newstart = start;
    newn = n;

    int bufsize = atom->get_length();
    char *buf = atom->get_buf();

    while ((buf[newstart]==' ' || buf[newstart]=='\n' || buf[newstart]=='\t' || buf[newstart]=='\r') && newstart<oldend && newstart<bufsize)
    {
        newstart++;
        newn--;
    }

    while ( (buf[newstart+newn-1]==' ' || buf[newstart+newn-1]=='\n' || buf[newstart+newn-1]=='\t' || buf[newstart+newn-1]=='\r') && newn>0 )
    {
        newn--;
    }

    SRef<IString*> ret = new IString(atom, newstart,newn);
    return ret;
}

SRef<IString*> IString::substr(int i)
{
    my_assert(i<=n); //if i==n the result will be an empty string
    SRef<IString*> ret = new IString(atom, start+i,n-i);
    return ret;
}

SRef<IString*> IString::substr(int i, int newn)
{
    SRef<IString*> ret = new IString(atom, start+i ,newn);
    return ret;
}

struct strptr IString::get_string_pointer() const
{
    struct strptr p;
    p.str_start = atom->get_buf()+start;
    p.n = n;
    return p;
}

std::string IString::cpp_str()
{
    std::string ret(atom->get_buf()+start, n);
    return ret;
}
