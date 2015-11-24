#ifndef SSINGLETON_H
#define SSINGLETON_H
#include "sobject.h"

template<class T>
class SSingleton
{
public:
    static SRef<T*> get_instance()
    {
        if( !instance )
            instance = new T();
        return instance;
    }
protected:
    SSingleton() {}
    virtual ~SSingleton() {}

private:
    SSingleton(const SSingleton&);
    SSingleton& operator=(const SSingleton&);
    static SRef<T*> instance;
};

template<class T>
SRef<T *> SSingleton<T>::instance;

#endif // SSINGLETON_H
