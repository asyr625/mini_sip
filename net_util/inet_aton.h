#ifndef INET_ATON_H
#define INET_ATON_H

#ifdef WIN32
# include<winsock2.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// borrowed from tcpdump
int
inet_aton(const char *cp, struct in_addr *addr);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // INET_ATON_H
