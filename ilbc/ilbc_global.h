#ifndef ILBC_GLOBAL_H
#define ILBC_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ILBC_LIBRARY)
#  define ILBCSHARED_EXPORT Q_DECL_EXPORT
#else
#  define ILBCSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // ILBC_GLOBAL_H
