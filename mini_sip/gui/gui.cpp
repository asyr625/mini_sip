#include "gui.h"

Gui::~Gui()
{
}

void Gui::set_callback(SRef<Command_Receiver*> cb)
{
    this->callback = cb;
}

SRef<Command_Receiver*> Gui::get_callback()
{
    return callback;
}
