#include "uuid.h"
#include "uuid_t.h"
using namespace std;

#define UUID_STRLEN 36

#define uuid ((uuid_t*)m_priv)

Uuid::Uuid(const void *priv)
{
    m_priv = new uuid_t;

    *uuid = *((uuid_t*)priv);
}

Uuid::~Uuid()
{
    delete uuid;
}

Uuid* Uuid::create()
{
    uuid_t u;

    uuid_create(&u);
    return new Uuid(&u);
}

int Uuid::operator=(const Uuid &u)
{
    return uuid_compare(uuid, ((uuid_t*)u.m_priv));
}

std::string Uuid::to_string()
{
    char buf[UUID_STRLEN+1]="";

    return uuid_to_str(uuid, buf, sizeof(buf));
}
