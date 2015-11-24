#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "sobject.h"
#include "exception.h"

class Semaphore_Exception : public Exception
{
public:
    Semaphore_Exception(std::string what);
};

class Semaphore : public SObject
{
public:
    Semaphore();
    ~Semaphore();

    std::string get_mem_object_type() const { return "Semaphore"; }

    void inc();

    void dec();

    bool dec_try(const bool &block_until_available=true);

private:
    void *handle_ptr;
};

#endif // SEMAPHORE_H
