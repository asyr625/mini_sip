#include "library.h"

#include "my_assert.h"

#include<iostream>

// Use ltdl symbols defined in ltdl.c
#define LT_SCOPE extern
#include<ltdl.h>

using namespace std;

int Library::ref_count = 0;

Library::Library(const string &path_)
    : path(path_)
{

    if( ref_count == 0 )
        lt_dlinit();

    ref_count++;

    handle = lt_dlopenext(path.c_str());
#ifdef DEBUG_OUTPUT
    if( !handle ){
        cerr << "Library opening \""<< path << "\": " << lt_dlerror() << endl;
    }
#endif
}

Library::~Library()
{
    if(handle)
    {
        lt_dlclose((lt_dlhandle)handle);
        handle=NULL;
    }

    ref_count--;
    if( ref_count == 0 )
        lt_dlexit();
}

void *Library::get_function_ptr(string name)
{
    void * ptr = lt_dlsym((lt_dlhandle)handle, name.c_str());
    return ptr;
}

SRef<Library *> Library::open(const string &path)
{
    SRef<Library *> ret = new Library(path);
    if(ret->handle)
    {
        const lt_dlinfo *info;

        info = lt_dlgetinfo( (lt_dlhandle)ret->handle );
        if( info && info->filename )
        {
            ret->path = info->filename;
        }

        return ret;
    }
    ret = NULL;
    return ret;
}

const string &Library::get_path()
{
    return path;
}
