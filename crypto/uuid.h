#ifndef UUID_H
#define UUID_H

#include "sobject.h"

class Uuid : public SObject
{
public:
    ~Uuid();
    static Uuid* create();

    int operator=(const Uuid &u);
    std::string to_string();

private:
    Uuid(const void *priv);
    void *m_priv;
};

#endif // UUID_H
