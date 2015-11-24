#ifndef NET_UTIL_GLOBAL_H
#define NET_UTIL_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(NET_UTIL_LIBRARY)
#  define NET_UTILSHARED_EXPORT Q_DECL_EXPORT
#else
#  define NET_UTILSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // NET_UTIL_GLOBAL_H
