
#include "my_assert.h"
#include "exception.h"

#include<string.h>
#include<iostream>
#include<stdio.h>
#include<stdlib.h>

void assert_failed(const char *expr, const char *file, const char *baseFile, int line)
{
    if (!strcmp(file, baseFile))
    {
        fprintf(stderr,"massert(%s) failed in file %s, line %d\n", expr, file, line);
    }
    else
    {
        fprintf(stderr,"massert(%s) failed in file %s (included from %s), line %d\n",
                expr, file, baseFile, line);
    }
    std::string stackTrace = get_stack_trace_string();
    if (stackTrace.size()>0)
    {
        std::cerr << "massert stack trace:"<<std::endl;
        std::cerr << stackTrace<<std::endl;
    }
    exit(1);
}
