#ifndef MY_ASSERT_H
#define MY_ASSERT_H

#include "util_config.h"

LIBUTIL_API void assert_failed(const char *expr, const char *file, const char *baseFile, int line);

#ifdef NDEBUG
#define my_assert(exp)
#else
/**
 * my_assert works the same way as assert except that
 * if libutil has support for stack traces, then
 * the current state of the stack is output.
 * Note that the stack trace will contain calls to
 * "assert_failed" and "get_stack_trace_string".
 */

#ifdef _MSC_VER
#include<assert.h>
#define my_assert(exp) assert(exp)
#else
#ifdef __BASE_FILE__
#define my_assert(exp) if (exp) ; else assert_failed(#exp, __FILE__, __BASE_FILE__, __LINE__)
#else
#define my_assert(exp) if (exp) ; else assert_failed(#exp, __FILE__, __FILE__, __LINE__)
#endif
#endif
#endif

#endif // MY_ASSERT_H
