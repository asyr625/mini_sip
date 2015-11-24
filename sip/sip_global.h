#ifndef SIP_GLOBAL_H
#define SIP_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SIP_LIBRARY)
#  define SIPSHARED_EXPORT Q_DECL_EXPORT
#else
#  define SIPSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // SIP_GLOBAL_H
