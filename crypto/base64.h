#ifndef BASE64_H
#define BASE64_H

#include<string>

std::string base64_encode( unsigned char *, int );
unsigned char * base64_decode( std::string, int * );
unsigned char * base64_decode( unsigned char *, int, int * );

#endif // BASE64_H
