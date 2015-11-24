#include "key_agreement_rsar.h"
#include "mikey_message.h"

Key_Agreement_RSAR::Key_Agreement_RSAR( SRef<Certificate_Chain *> cert, SRef<Certificate_Set *> certificateset )
    :Key_Agreement_PKE(cert, certificateset)
{
}

Key_Agreement_RSAR::~Key_Agreement_RSAR()
{
}

Mikey_Message* Key_Agreement_RSAR::create_message()
{
    return Mikey_Message::create( this );
}
