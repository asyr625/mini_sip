#ifndef SOBJECT_H
#define SOBJECT_H

#include <string>

#include "my_assert.h"
#include "mini_list.h"

class Mutex;
class SObject
{
public:
    SObject();

    SObject(const SObject& );
    virtual ~SObject();

    void operator= (const SObject&);

    int dec_ref_count() const;

    void inc_ref_count() const;

    int get_ref_count() const;

    virtual std::string get_mem_object_type() const;

public:
    mutable int ref_count;
    Mutex*  ref_lock;
};

template<class OPType>
class SRef
{
public:
    inline SRef(const OPType& optr = NULL);

    inline SRef(const SRef<OPType>& r);
    inline virtual ~SRef();

    inline SRef<OPType>& operator=(const OPType& o);

    inline SRef<OPType>& operator=(const SRef<OPType>& r);

    inline bool operator ==(const SRef<OPType>& r) const;

    inline bool operator <(const SRef<OPType>& r) const;

    inline bool is_null() const;

    inline operator bool() const;

    inline OPType operator->() const;

    inline OPType operator*() const;
protected:
    inline bool init();
    inline bool increase();

    inline bool decrease();

    inline OPType get_pointer() const;

    inline void set_pointer(const OPType& o);

private:
    OPType  _objp;
};
template<class OPType>
OPType SRef<OPType>::get_pointer() const
{
    return _objp;
}

template<class OPType>
void SRef<OPType>::set_pointer(const OPType& o)
{
    if (o != NULL)
    {
        if( dynamic_cast<const SObject*>(o) != NULL )
        {
            _objp = o;
        }
        else
        {
            _objp = NULL;
        }
    }
    else
    {
        _objp = NULL;
    }
}


template<class OPType>
bool SRef<OPType>::init()
{
    bool ret = false;
    if ( _objp != NULL )
    {
        ret = increase();
    }
    return ret;
}

template<class OPType>
bool SRef<OPType>::increase()
{
    bool ret = false;
    if ( _objp != NULL ) {
        _objp->inc_ref_count();
        ret = true;
    }
    return ret;
}

template<class OPType>
bool SRef<OPType>::decrease()
{
    bool ret = false;

    if ( _objp != NULL ) {
        int rc = _objp->dec_ref_count();
        if (rc <= 0)
        {
            if ( rc < 0 )
            {
#ifndef _MSC_VER
#ifdef DEBUG_OUTPUT
                std::cerr << "SRef::~SRef: WARNING: deleteing object with negative reference count ("
                          << rc
                          << ") - created without reference?" << std::endl;
#endif
#endif
            }
            delete get_pointer();
            set_pointer(NULL);
            ret = true;
        }
    }
    return ret;
}
template<class OPType>
SRef<OPType>::SRef(const OPType& optr)
{
    set_pointer(optr);
    init();
}
template<class OPType>
SRef<OPType>::SRef(const SRef<OPType>& r)
{
    set_pointer( r.get_pointer() );
    increase();
}
template<class OPType>
SRef<OPType>::~SRef()
{
    decrease();
    set_pointer( NULL );
}
template<class OPType>
SRef<OPType>& SRef<OPType>::operator=(const OPType& o)
{
    if(_objp != o)
    {
        decrease();
        set_pointer( o );
        increase();
    }
    return *this;
}

template<class OPType>
SRef<OPType>& SRef<OPType>::operator=(const SRef<OPType>& r)
{
    OPType ptr = r.get_pointer();
    if(_objp != ptr)
    {
        decrease();
        set_pointer( ptr );
        increase();
    }
    return *this;
}

template<class OPType>
bool SRef<OPType>::operator ==(const SRef<OPType>& r) const
{
    return get_pointer() == r.get_pointer();
}

template<class OPType>
bool SRef<OPType>::operator <(const SRef<OPType>& r) const
{
    return get_pointer() < r.get_pointer();
}

template<class OPType>
bool SRef<OPType>::is_null() const
{
    return get_pointer()==NULL;
}

template<class OPType>
SRef<OPType>::operator bool() const
{
    return !is_null();
}
template<class OPType>
OPType SRef<OPType>::operator->() const
{
    OPType ret = get_pointer();
    if( ret == NULL )
    {
#ifdef DEBUG_OUTPUT
        std::cerr << "SRef::operator-> : ERROR: trying to access a null pointer (";
        std::cerr << typeid(OPType).name() << ").";
        std::cerr << std::endl;
        std::cerr << "Stack trace: "<< get_stack_trace_string() << std::endl;
#endif
        my_assert( ret != NULL );
    }
    return ret;
}

template<class OPType>
OPType SRef<OPType>::operator*() const
{
    OPType ret;
    ret = get_pointer();
    if( ret == NULL )
    {
        #ifdef DEBUG_OUTPUT
        std::cerr << "SRef::operator* : Warning: accessing a null pointer (" << typeid(OPType).name() << ")." << std::endl;
        std::cerr << "Stack trace: "<< get_stack_trace_string() << std::endl;
        #endif
    }
    return ret;
}

bool set_debug_output(bool on);

bool get_debug_output_enabled();

int get_mem_object_count();

Mini_List<std::string> get_mem_object_names();

Mini_List<std::string> get_mem_object_names_summary();

#endif // SOBJECT_H
