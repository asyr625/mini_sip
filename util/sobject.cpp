#include "sobject.h"

#include "mutex.h"
#include "dbg.h"
#include<string>

#include<typeinfo>

using namespace std;

#ifdef MDEBUG
#include "string_utils.h"
#include<list>
Mutex *globalLock = NULL;
Mini_List<SObject *> objs;
int ocount = 0;
bool outputOnDestructor=false;

Mutex &global()
{
    if (!globalLock)
        globalLock = new Mutex;
    return *globalLock;
}
#endif

SObject::SObject()
    : ref_count(0)
{
#ifdef MDEBUG
    global().lock();
    ocount++;
    objs.push_front(this);
    ref_lock = NULL;
    global().unlock();
#else
    ref_lock = new Mutex();
#endif
}


// The reference count should be zero since
// any references to the argument object
// are not referencing us.
SObject::SObject(const SObject &)
    :ref_count(0)
{
#ifdef MDEBUG
    global().lock();
    ocount++;
    objs.push_front(this);
    ref_lock = NULL;
    global().unlock();
#else
    ref_lock = new Mutex();	//We don't want to share the mutex
#endif
}

SObject::~SObject()
{
#ifdef MDEBUG
    global().lock();
    for (int i=0; i<objs.size(); i++)
    {
        if (this == objs[i])
        {
            objs.remove(i);
            ocount--;
            break;
        }
    }
    global().unlock();
#else
    my_assert(ref_lock);
    delete ref_lock;
    ref_lock = NULL;
#endif
}

void SObject::operator=(const SObject &)
{
    // we don't copy the mutex handle - whe one we already
    // have protects the reference counter we in this object.
    // We also don't copy the reference counter. The value
    // we already have is the correct number of references.
    //
    // Don't delete this method even if it is empty.
}


int SObject::dec_ref_count() const
{
    int refRet;
#ifdef MDEBUG
    global().lock();
#else
    ref_lock->lock();
#endif

    ref_count--;
    refRet = ref_count;

#ifdef MDEBUG
    global().unlock();
    if (refRet==0 && outputOnDestructor)
    {
        string output = "MO (--):"+get_mem_object_type()+ "; count=" + itoa(refRet) + "; ptr=" + itoa((uint64_t)this);
        my_dbg("memobject") << output << endl;
    }
#else
    ref_lock->unlock();
#endif
    return refRet;
}

void SObject::inc_ref_count() const
{
#ifdef MDEBUG
    global().lock();
#else
    ref_lock->lock();
#endif

    ref_count++;

#ifdef MDEBUG
    global().unlock();
    if (ref_count == 1 && outputOnDestructor )
    {
        string output = "MO (++):"+get_mem_object_type()+ "; count=" + itoa(ref_count);
        my_dbg("memobject") << output << endl;
    }
#else
    ref_lock->unlock();
#endif
}

int SObject::get_ref_count() const
{
    return ref_count;
}

string SObject::get_mem_object_type() const
{
#ifdef MDEBUG
    return (typeid(*this)).name();
#else
    return "(unknown)";
#endif
}

Mini_List<string> get_mem_object_names()
{
#ifdef MDEBUG
    Mini_List<string> ret;
    global().lock();
    for (int i=0; i< objs.size(); i++)
    {
        int count = objs[i]->get_ref_count();
        string countstr = count?itoa(count):"on stack";
        ret.push_front(objs[i]->get_mem_object_type()+"("+countstr+")" + "; ptr=" + itoa((uint64_t)objs[i]) );
    }
    global().unlock();
    return ret;
#else
    Mini_List<string> ret;
    return ret;
#endif
}

int get_mem_object_count()
{
#ifdef MDEBUG
    return ocount;
#else
    return -1;
#endif
}

bool set_debug_output(bool on)
{
#ifdef MDEBUG
    outputOnDestructor=on;
    return true;
#else
    return false;
#endif
}

bool get_debug_output_enabled()
{
#ifdef MDEBUG
    return outputOnDestructor;
#else
    return false;
#endif
}

Mini_List<string> get_mem_object_names_summary()
{
#ifdef MDEBUG
    Mini_List<string> ret;
    std::list<string> str;	// unique names
    std::list<int> count;   // count for corresponding name in str
    std::list<string>::iterator si;
    std::list<string>::iterator js;
    std::list<int>::iterator jc;
    global().lock();
    // get list of unique names, and count
    int i;
    for (i=0; i< objs.size(); i++)
    {
        string oname = objs[i]->get_mem_object_type();
        bool found=false;
        for (  jc=count.begin(), js=str.begin();  js!=str.end();  jc++, js++  )
        {
            if (*js==oname){
                (*jc)=*jc+1;
                found=true;
                break;
            }
        }
        if (!found)
        {
            str.push_back(oname);
            count.push_back(1);
        }
    }

    // Bubble sort to arrange names is ascending order
    bool done;
    do {
        done = true;
        std::list<int>::iterator jc_last;
        std::list<string>::iterator js_last;
        jc_last = count.begin();
        js_last = str.begin();
        js=str.begin();
        jc=count.begin();
        if (js!=str.end())
        { //start from second item
            js++;
            jc++;
        }

        for ( ; js!=str.end(); jc++, js++, jc_last++, js_last++)
        {
            if ( (*jc) < (*jc_last) )
            {
                int tmpc=*jc;
                *jc = *jc_last;
                *jc_last=tmpc;

                string tmps=*js;
                *js = *js_last;
                *js_last=tmps;
                done = false;

            }
        }
    }while (!done);

    for (jc=count.begin(), js=str.begin(); js!=str.end(); jc++, js++)
    {
        ret.push_back( *js + " " + itoa( *jc ) );
    }

    global().unlock();
    return ret;
#else
    Mini_List<string> ret;
    return ret;
#endif
}
