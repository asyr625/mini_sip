#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <string>
#include "my_types.h"

#ifdef WIN32
    //do not alert in mingw32 ... i don't know the equivalent pragma message ...
#	ifndef __MINGW32__
#		pragma message ("Using libmutil::Timestamp.h in windows system ... useless ...")
#	endif
#endif


#ifdef WIN32
#else
#	include<stdint.h>
#	include<sys/time.h>
#endif


#include<time.h>
#include<fstream>
#include<string>
#include<sstream>
#include<ctime>

#define FILE_NAME "/tmp/minisip_ts"
#define INVITE_START 	0
#define INVITE_END   	1
#define MIKEY_START  	2
#define MIKEY_END    	3
#define RINGING      	4
#define PACKET_IN    	5
#define PACKET_OUT   	6
#define TLS_START    	7
#define TLS_END		8
#define DH_PRECOMPUTE_START 	9
#define DH_PRECOMPUTE_END	10
#define MIKEY_CREATE_START	11
#define MIKEY_CREATE_END	12
#define RAND_START		13
#define RAND_END		14
#define SIGN_START		15
#define SIGN_END		16
#define AUTH_START		17
#define AUTH_END		18
#define MIKEY_PARSE_START	19
#define MIKEY_PARSE_END		20
#define TGK_START		21
#define TGK_END			22
#define USER_ACCEPT		23

#define TMP	     24

typedef clock_t SystemTime;

class Timestamp
{
public:
    Timestamp();
    Timestamp(const Timestamp&);
    ~Timestamp();

    void save( uint32_t id );
    void save(std::string descr);
    void print();
    void print(std::string fileName);

private:
    uint32_t index;

    struct timezone * tz;

    struct timeval * values;
    int32_t * ids;

    int auto_id;
    std::string *strings;

    double start_time;
    double stop_time;
    std::string filename;
};

#endif // TIMESTAMP_H
