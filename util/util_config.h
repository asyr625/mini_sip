#ifndef UTIL_CONFIG_H
#define UTIL_CONFIG_H

#ifdef _MSC_VER
# ifndef LIBUTIL_EXPORTS
#  define LIBUTIL_IMPORTS
# endif
#endif

#if defined(WIN32) && defined(LIBUTIL_EXPORTS)
# define LIBUTIL_API __declspec(dllexport)
#elif defined(WIN32) && defined(LIBUTIL_IMPORTS)
# define LIBUTIL_API __declspec(dllimport)
#else
# define LIBUTIL_API
#endif

#if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
#define LIBUTIL_DEPRECATED  __attribute__((__deprecated__))
#else
#define LIBUTIL_DEPRECATED
#endif /* __GNUC__ */

#endif // UTIL_CONFIG_H
