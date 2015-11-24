#include "video_cafe_gui.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "media_command_string.h"
#include "sobject.h"
#include "string_utils.h"
#include "sip_command_string.h"
#include "default_dialog_handler.h"
#include "video_display.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>


using namespace std;
Video_Cafe_Gui::Video_Cafe_Gui(bool fullscreen, std::string _mcu)
    : mcu(_mcu)
{
    start_fullscreen = fullscreen;
    thread = NULL;
    quit_sem = new Semaphore();
    window = OpenGl_Window::get_window(fullscreen);
}


void Video_Cafe_Gui::connect()
{
    Command_String cmdstr( "", Sip_Command_String::invite, mcu);
    Command_String resp = send_command_resp("sip",cmdstr);
}

void Video_Cafe_Gui::handle_command(const Command_String&cmd)
{
    cerr << "VCafeGui::handleCommand: Got "<<cmd.get_string() << endl;
}

Command_String Video_Cafe_Gui::handle_command_resp(std::string subsystem, const Command_String&cmd)
{
    if (cmd.get_op()=="invite")
    {
        return send_command_resp("sip", cmd);
    }
    my_assert(false);
}

void Video_Cafe_Gui::set_sip_configuration(SRef<Sip_Configuration *>sipphoneconfig)
{
    config = sipphoneconfig;
}

bool Video_Cafe_Gui::config_dialog( SRef<Sip_Configuration *> /*conf*/ )
{
    cout << "ERROR: VCafeGui::configDialog is not implemented"<< endl;
    return false;
}

void Video_Cafe_Gui::display_error_message(std::string msg)
{
}

void Video_Cafe_Gui::start()
{
    thread = new Thread(this, Thread::Normal_Priority);
}

void join()
{
}

void Video_Cafe_Gui::run()
{
#ifdef DEBUG_OUTPUT
    set_thread_name("Video_Cafe_Gui::run");
#endif
    Video_Display_Registry::get_instance();

    //Un-block any thread waiting for us to quit
    quit_sem->inc();
}

void Video_Cafe_Gui::wait_quit()
{
    quit_sem->dec();
}

void Video_Cafe_Gui::set_callback(SRef<Command_Receiver*> callback)
{
    Gui::set_callback(callback);
    SRef<Semaphore *> localSem = sem_sip_ready;
    if( localSem )
    {
        localSem->inc();
    }
}
