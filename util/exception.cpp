#include "exception.h"
#include "util_defines.h"

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#endif

using namespace std;

#define MAX_STACK_TRACE_DEPTH 30

//The code for getting the stack trace
//is the same as in the second constructor.
//It could be in a private method, but
//then that will be in the trace as well...
Exception::Exception():exception()
{
#ifdef HAVE_EXECINFO_H
    stack = new void*[MAX_STACK_TRACE_DEPTH];
    if (stack)
    {
        stack_depth = backtrace(stack, MAX_STACK_TRACE_DEPTH);
    }else{
        stack_depth = 0;
    }
#else
    stack_depth = -1;
    stack = NULL;
#endif
}

Exception::Exception(const Exception &e)
    : msg(e.msg),
      stack_depth(e.stack_depth)
{
#ifdef HAVE_EXECINFO_H
    stack = new void*[MAX_STACK_TRACE_DEPTH];
    memcpy(stack, e.stack, MAX_STACK_TRACE_DEPTH*sizeof(void*));
#else
    stack = NULL;
    stack_depth = -1;
#endif
}

/**
 * We use "backtrace" in libc to get find out what the
 * stack looks like.
 *
 * We store the return value from "backtrace", but only
 * transform it into names when "stackTrace()" is called.
 *
 * See:
 *  http://www.gnu.org/software/libc/manual/html_node/Backtraces.html
 */
Exception::Exception(char const* m):exception()
{
    msg = string(m);
#ifdef HAVE_EXECINFO_H
    stack = new void*[MAX_STACK_TRACE_DEPTH];
    if (stack){
        stack_depth = backtrace(stack, MAX_STACK_TRACE_DEPTH);
    }else{
        stack_depth = 0;
    }
#else
    stack_depth = -1;
    stack = NULL;
#endif
}

/**
 * Same as Exception(char*) except that it takes a string instead.
*/
Exception::Exception(const std::string &m):exception()
{
    msg = m;
#ifdef HAVE_EXECINFO_H
    stack = new void*[MAX_STACK_TRACE_DEPTH];
    if (stack){
        stack_depth = backtrace(stack, MAX_STACK_TRACE_DEPTH);
    }else{
        stack_depth = 0;
    }
#else
    stack_depth = -1;
    stack = NULL;
#endif
}


Exception::~Exception() throw()
{
    if (stack)
        delete[] stack;
    stack=NULL;
}

const char* Exception::what()const throw()
{
    return msg.c_str();
}

string Exception::stack_trace() const
{
    string ret;
#ifdef HAVE_EXECINFO_H
    if (stack && stack_depth>0)
    {
        char **strings = backtrace_symbols(stack,stack_depth);
        for (int i=1; i<stack_depth; i++)
        {
            ret+=string(strings[i])+"\n";
        }
        free (strings);
    }else{
        ret = "(stack trace failed)";
    }
#else
    ret="";
#endif
    return ret;
}

string get_stack_trace_string()
{
#ifdef HAVE_EXECINFO_H
    string ret;
    void **stack = new void*[MAX_STACK_TRACE_DEPTH];
    if (stack)
    {
        int n= backtrace(stack, MAX_STACK_TRACE_DEPTH);
        char **strings = backtrace_symbols(stack, n);
        for (int i=0; i<n; i++){
            ret+="  " + string(strings[i])+"\n";
        }
        free (strings);
    }else{
        ret= "(failed to allocate memory for stack trace)";
    }
    return ret;
#else
    return "(stack trace not enabled in libmutil Exception::get_stack_trace_string)";
#endif
}
