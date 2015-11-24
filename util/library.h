#ifndef LIBRARY_H
#define LIBRARY_H

#include "sobject.h"

class Library : public SObject
{
public:
    static SRef<Library *> open(const std::string &path);
    virtual ~Library();
    void *get_function_ptr(std::string name);

    const std::string &get_path();

    std::string get_mem_object_type() const { return "Library"; }
protected:
    Library(const std::string &path);
private:
    void *handle;
    std::string path;
    static int ref_count;
};

#endif // LIBRARY_H
