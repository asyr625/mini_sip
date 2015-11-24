#ifndef MIKEY_GLOBAL_H
#define MIKEY_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(MIKEY_LIBRARY)
#  define MIKEYSHARED_EXPORT Q_DECL_EXPORT
#else
#  define MIKEYSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // MIKEY_GLOBAL_H
