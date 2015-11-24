#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <vector>
#include <locale>
#include "my_types.h"

std::string quote(const std::string &str);

std::string unquote(std::string str);

std::string itoa(int64_t i);

std::vector<std::string> split(const std::string &s, bool do_trim=true, char delim='\n', bool includeEmpty=false);

std::vector<std::string> split_lines(const std::string &s, bool do_trim = true);

std::string up_case(const std::string &s);

int up_case(char c);

int str_case_cmp(const char *s1, const char* s2);

int strNCaseCmp(const char *s1, const char* s2, int n);

bool is_ws(char c);

std::string trim(const std::string &s);

std::string bin_to_hex( const unsigned char * data, int length );

template <class charT, class traits, class Alloc>
int str_case_cmp( const std::basic_string<charT, traits, Alloc>& s1,
                 const std::basic_string<charT, traits, Alloc>& s2,
                 const std::locale& loc );

bool string_ends_with(const std::string & haystack, const std::string & needle);

bool replace(std::string& str, const std::string& from, const std::string& to);

void replace_all(std::string& str, const std::string& from, const std::string& to);

#endif // STRING_UTILS_H
