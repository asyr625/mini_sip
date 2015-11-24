#include "string_utils.h"
#include "my_types.h"

#include<string>
#include<iostream>
#include<stdio.h>

#ifdef _MSC_VER
static int nocaseequal(char c1, char c2)
{
    if ( ((c1>='A') && (c1<='Z')) )
    {
        return (c1==c2) || (c1 == (c2 - ('a'-'A')));
    }
    if ( (c1>='a') && (c1<='z') )
    {
        return (c1==c2) || (c1 == (c2 + ('a'-'A')));
    }
    return c1==c2;
}

//Fixes by Philippe Torrelli - Thanks
static int strncasecmp(const char *s1, const char *s2, int n)
{
    int i;
    for ( i=0; s1[i]!=0 && s2[i]!=0 && (n==-1 || (n!=-1 && i<n) ); i++)
    {
        if ( !nocaseequal(s1[i],s2[i]) )
        {
            if (s1[i]<s2[i]){
                return -1;
            }else{
                return 1;
            }
        }
    }

    if(i != n && (s1[i] != s2[i] ))
    {
        if (s1[i] == '\0' ){
            return -1;
        }else{
            return 1;
        }
    }
    return 0;
}


static int strcasecmp(const char *s1, const char *s2)
{
    return strncasecmp(s1,s2,-1);
}

#else
#include <strings.h>
#endif

using namespace std;

string quote(const string &str)
{
    if (str.length()>1 && str[0] == '"' && str[str.length() - 1] == '"')
        return str;
    else
        return "\"" + str + "\"";
}

string unquote(string str)
{
    if( str.length() > 1 && str[0] == '"' && str[str.length() - 1] == '"' )
        return str.substr(1, str.length() - 2);
    else
        return str;
}


std::vector<string> split(const string &s, bool do_trim, char delim, bool includeEmpty)
{
    std::vector<string> ret;

    if (s.size()==0)
        return ret;

    unsigned i=0;
    do{
        string line;
        while (!(i>(s.length()-1)) && s[i]!=delim)
            line+=s[i++];
        if (do_trim)
            line=trim(line);
        if (line.length()>0 || includeEmpty)
            ret.push_back(line);
        i++;
    }while (!(i>=s.length()));

    if ( s.size()>0 && s[s.length()-1]==delim && includeEmpty )
        ret.push_back("");

    return ret;
}

std::vector<string> split_lines(const string &s, bool do_trim)
{
    return split(s, do_trim, '\n',false);
}

bool is_ws(char c)
{
    return (c==' ' || c=='\t' || c=='\n' || c=='\r');
}

string up_case(const string &s)
{
    size_t n=s.size();
    string ret(n,' ');
    string::const_iterator i=s.begin();
    string::iterator j=ret.begin();
    for ( ; i!=s.end(); i++, j++)
        *j = up_case(*i);
    return ret;
}

int up_case(char c)
{
    if ((c>='a') && (c<='z'))
        return c - ('a'-'A');
    else
        return c;
}


int str_case_cmp(const char *s1, const char* s2)
{
    return strcasecmp(s1,s2);
}

int strNCaseCmp(const char *s1, const char* s2, int n)
{
    return strncasecmp(s1,s2,n);
}

string trim(const string &line)
{
    size_t spacesFront = 0, spacesEnd = 0;
    int32_t idx;

    idx = 0;
    while( idx < (int)line.size() && isspace(line[idx]) )
    {
        spacesFront++;
        idx++;
    }

    idx = (int)line.size() - 1 ;
    while( idx >= 0 && isspace(line[idx]) )
    {
        spacesEnd++;
        idx--;
    }
    return line.substr( spacesFront, line.size() - ( spacesFront + spacesEnd ) );
}

string bin_to_hex( const unsigned char * data, int length )
{

    string result = "";
    char hex_number[3];

    for( int i = 0; i < length; i++ ){
#ifdef _MSC_VER
        _snprintf_s( hex_number, 3, 2, "%02x", *(data+i) );
#else
        snprintf( hex_number, 3, "%02x", *(data+i) );
#endif
        result += hex_number;
    }
    return result;
}


string itoa(int64_t i)
{
    char buf[30]; //should be enough for a 64 bit integer ...
    for (int j=0; j<16; j++)
        buf[j]=0;
#ifdef _MSC_VER
    _snprintf_s( buf, 30, 29, "%lld", i);
#else
    snprintf(buf, 30, "%lld",i);
#endif
    return string(buf);
}


template <class charT, class traits, class Alloc>
int str_case_cmp( const std::basic_string<charT, traits, Alloc>& s1,
        const std::basic_string<charT, traits, Alloc>& s2,
        const std::locale& loc )
{
    typedef typename std::basic_string<charT, traits, Alloc>::const_iterator iterator;

    const std::ctype<charT>& facet = std::use_facet<std::ctype<charT> >(loc);

    if( s1.size() < s2.size() ){
        return 1;
    }
    else if( s1.size() > s2.size() ){
        return -1;
    }

    iterator i,j;
    iterator last = s1.end();

    for (i = s1.begin(), j = s2.begin(); i != last; ++i, ++j)
    {
        const charT c1 = *i;
        const charT c2 = *j;
        if ( !(c1 == c2) &&
             !(facet.toupper(c1) == facet.toupper(c2)) ){
            return (int(c1 < c2) - int(c2 < c1));
        }
    }
    return 0;
}

//template int strCaseCmp<char, char_traits<char>, allocator<char> >(const string&, const string&, const locale&);

bool string_ends_with(const std::string & haystack, const std::string & needle)
{
    if (haystack.length() >= needle.length()) {
        return (haystack.substr(haystack.length() - needle.length(), needle.length()) == needle);
    } else
        return false;
}

bool replace(std::string& str, const std::string& from, const std::string& to)
{
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

void replace_all(std::string& str, const std::string& from, const std::string& to)
{
    size_t startpos=0;
    while (1){
        size_t foundpos;
        if ((foundpos=str.find(from,startpos))==string::npos)
            return;
        str.replace(foundpos, from.length(), to);
        startpos = foundpos + to.length();
    }
}
