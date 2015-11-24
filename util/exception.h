#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <string>
#include <exception>

class Exception : public std::exception
{
public:
    Exception();
    Exception(const char* what);
    Exception(const std::string& what);
    Exception(const Exception&);

    ~Exception() throw ();

    virtual const char* what() const throw();

    std::string stack_trace() const;

protected:
    std::string msg;
private:
    void** stack;
    int stack_depth;
};

std::string get_stack_trace_string();

#endif // EXCEPTION_H
