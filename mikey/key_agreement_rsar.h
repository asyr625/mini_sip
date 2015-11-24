#ifndef KEY_AGREEMENT_RSAR_H
#define KEY_AGREEMENT_RSAR_H

#include "key_agreement_pke.h"

class Key_Agreement_RSAR : public Key_Agreement_PKE
{
public:
    Key_Agreement_RSAR( SRef<Certificate_Chain *> cert, SRef<Certificate_Set *> certificateset );
    ~Key_Agreement_RSAR();

    Mikey_Message* create_message();
};

#endif // KEY_AGREEMENT_RSAR_H
