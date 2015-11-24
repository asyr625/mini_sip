#ifndef COND_VAR_H
#define COND_VAR_H

#include "sobject.h"

class Mutex;

class Cond_Var : public SObject
{
public:
    Cond_Var();
    ~Cond_Var();

    void broadcast();
    std::string get_mem_object_type() const;

    static void wait(Cond_Var &variable, bool &deleted, const unsigned int &timeout_ms = 0);
    static void wait(Cond_Var &variable, Mutex &mutex, bool &deleted, const unsigned int &timeout_ms = 0);

private:
    void *internal_struct;
#ifndef _WIN32_WCE
    Mutex *condvar_mutex;
    Mutex *current_mutex;
    bool deleted;
    int waiting_threads_count;
#endif
    void wait( Mutex &mutex, bool &deleted, const unsigned int &timeout_ms = 0);
};

#endif // COND_VAR_H
