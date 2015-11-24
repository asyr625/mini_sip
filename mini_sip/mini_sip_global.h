#ifndef MINI_SIP_GLOBAL_H
#define MINI_SIP_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(MINI_SIP_LIBRARY)
#  define MINI_SIPSHARED_EXPORT Q_DECL_EXPORT
#else
#  define MINI_SIPSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // MINI_SIP_GLOBAL_H
