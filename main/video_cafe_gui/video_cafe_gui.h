#ifndef VIDEO_CAFE_GUI_H
#define VIDEO_CAFE_GUI_H

#include "command_string.h"
#include "my_semaphore.h"

#include "sip_configuration.h"
#include "video_display.h"
#include "gui.h"
#include "opengl_window.h"

#include <string>

class Video_Cafe_Gui : public Gui
{
public:
    Video_Cafe_Gui(bool fullscreen, std::string mcu);

    void connect();

    std::string get_mem_object_type() const {return "VCafeGui";}

    virtual void handle_command(const Command_String&);
    virtual Command_String handle_command_resp(std::string subsystem, const Command_String&);

    virtual void set_sip_configuration(SRef<Sip_Configuration *>sipphoneconfig);
    virtual void set_contact_db(SRef<ContactDb *>) { }
    virtual bool config_dialog( SRef<Sip_Configuration *> conf );

    virtual void display_error_message(std::string msg);

    void start();
    virtual void run();

    void wait_quit();

    virtual void set_callback(SRef<Command_Receiver*> callback);

    void join();

    SRef<OpenGl_Window*> get_window() { return window; }
private:
    SRef<OpenGl_Window*> window;

    std::string mcu;

    bool start_fullscreen;

    std::string current_caller;
    SRef<Sip_Configuration *> config;
    SRef<Semaphore *> sem_sip_ready;
    Thread *thread;
    SRef<Semaphore *> quit_sem;
};

#endif // VIDEO_CAFE_GUI_H
