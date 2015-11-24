#include "log_entry.h"

SRef<Log_Entry_Handler *> Log_Entry::handler;

void Log_Entry::handle()
{
    if( handler )
    {
        handler->handle( this );
    }
}
