#ifndef OPENGL_WINDOW_H
#define OPENGL_WINDOW_H

#include <GL/glu.h>
#include "mutex.h"
#include "my_semaphore.h"
#include "thread.h"
#include "opengl_display.h"

#include "animate.h"
#include "text.h"
#include "statistics_text_plane.h"

#include <list>

class SDL_Surface;
class Text;

#define DRAW_Z -20.0F

struct mgl_gfx
{
    GLuint texture;
    float wu;       //width usage of texture (0..1). How much of texture is rendered
    float hu;       //height usage
    float aratio;   // width/height
    int tex_dim;
#if 0
    Animate* x1;
    Animate* y1;
    Animate* x2;
    Animate* y2;
    Animate* alpha;
    Animate* rotate;
#endif
    bool is_selected;
    char* name;
    char* call_id;
    char* video_name;
};

typedef struct rect_f
{
    float x1,y1,x2,y2;
} rect_f;

typedef struct rect_a
{
    Animate* x1;
    Animate* y1;
    Animate* x2;
    Animate* y2;
} rect_a;


///A finger
class Touch
{
public:
    Touch(int x, int y, long touchId);
    std::string get_debug_string();
    int touchId; ///Used to differentiate between different fingers.
    ///A new ID is used for each drag.
    int x,y;
    //int pressure;
};

class STouch
{
public:
    std::string get_debug_string();
    std::vector<Touch> fingers;
};


class Window_Callback : public virtual SObject
{
public:
    virtual bool mouse_move(int sx, int sy, float glx, float gly) = 0;
    virtual bool mouse_down(int sx, int sy, float glx, float gly, int button) = 0;
    virtual bool mouse_up(int sx, int sy, float glx, float gly, int button) = 0;
    virtual bool mouse_click(int sx, int sy, float glx, float gly, int button) = 0;
    virtual bool mouse_double_click(int sx, int sy, float glx, float gly, int button) = 0;
    virtual bool mouse_drag_move(int from_sx, int from_sy, float from_glx, float from_gly,
                                 int cur_sx, int cur_sy, float cur_glx, float cur_gly,
                                 int button, int delta_sx, int delta_sy, float delta_glx, float delta_gly) = 0;

    virtual bool mouse_drag_release(int from_sx, int from_sy, float from_glx, float from_gly,
                                    int cur_sx, int cur_sy, float cur_glx, float cur_gly, int button) = 0;

    virtual bool supports_touch() = 0;
    virtual void touch(STouch &mt) = 0;

    virtual void update_layout(float x1, float y1, float x2, float y2, uint64_t cur_time, Animation_Type anim) = 0;
    virtual void draw(uint64_t curTime) = 0;
    virtual void add_display(SRef<OpenGl_Display*> d) = 0;
    virtual void remove_display(SRef<OpenGl_Display*> d) = 0;
};


class OpenGl_Ui : public Window_Callback
{
public:
    OpenGl_Ui(OpenGl_Window* glwin):window(glwin), enabled(false)
    {
    }

    virtual void init() = 0;
    virtual void stop() = 0;

    virtual void draw_background() = 0;

    virtual void draw_app() = 0;
    virtual void draw_widget() = 0;
    virtual void draw_icon() = 0;
    virtual void draw_video_decoration(OpenGl_Display* display) = 0;
    virtual void post_draw() = 0;

    virtual bool consumekey(SDL_KeyboardEvent& key) = 0;
    virtual bool mouse_wheel_up() = 0;
    virtual bool mouse_wheel_down() = 0;

    virtual bool has_app() = 0;	//returns true if plugins wants area reserved for it to draw in
    virtual bool has_widget() = 0;
    virtual bool has_icon() = 0;
    virtual bool has_video_decoration() = 0;
    virtual bool has_layout_manager() = 0;

    virtual bool app_visible() = 0;
    virtual bool widget_visible() = 0;

    virtual void set_app_visible(bool v) = 0;
    virtual void set_widget_visible(bool v) = 0;

    virtual void set_app_bounds(SRef<Animate*> x1, SRef<Animate*> y1,
                                SRef<Animate*> x2, SRef<Animate*> y2,
                                SRef<Animate*> alpha)
    {
        lock.lock();
        appBounds.set(x1, y1, x2, y2);
        appAlpha = alpha;
        lock.unlock();
    }


    virtual void set_widget_bounds(SRef<Animate*> x1, SRef<Animate*> y1,
                                   SRef<Animate*> x2, SRef<Animate*> y2,
                                   SRef<Animate*> alpha)
    {
        lock.lock();
        widgetBounds.set(x1, y1, x2, y2);
        widgetAlpha = alpha;
        lock.unlock();
    }

    virtual void set_icon_bounds(SRef<Animate*> x1, SRef<Animate*> y1,
                                 SRef<Animate*> x2, SRef<Animate*> y2,
                                 SRef<Animate*> alpha)
    {
        lock.lock();
        iconBounds.set(x1, y1, x2, y2);
        iconAlpha = alpha;
        lock.unlock();
    }

    virtual bool get_default_enabled() = 0;
    virtual void set_enabled(bool isEnabled)
    {
        lock.lock(); enabled = isEnabled; lock.unlock();
    }
    virtual bool get_enabled() const {return enabled;}

    bool isFocused;

    Bounds appBounds;
    Bounds widgetBounds;
    Bounds iconBounds;
    SRef<Animate*> appAlpha;
    SRef<Animate*> widgetAlpha;
    SRef<Animate*> iconAlpha;

protected:
    OpenGl_Window* window;
    Mutex lock;
    bool enabled;
};


class OpenGl_Ui_Plugin : public SPlugin
{
public:
    virtual SRef<OpenGl_Ui*> new_instance(OpenGl_Window* window) = 0;
    bool seen;
};

class OpenGl_Ui_Registry: public SPlugin_Registry, public SSingleton<OpenGl_Ui_Registry>
{
public:
    virtual std::string get_plugin_type(){ return "openglui"; }

protected:
    OpenGl_Ui_Registry();
private:
    friend class SSingleton<OpenGl_Ui_Registry>;
};


class Session_Registry;
class OpenGl_Window : public Runnable, public Command_Receiver
{
public:
    OpenGl_Window(int width, int height, bool fullscreen);
    virtual ~OpenGl_Window();

    void set_minisip(Mini_Sip*m);
    Mini_Sip* get_minisip();

    void handle_command(std::string subsystem, const Command_String& cmd);
    Command_String handle_command_resp(std::string subsystem, const Command_String& cmd);

    void set_default_callback(SRef<Window_Callback*> _callback)
    {
        default_callback = _callback;
        if (!callback)
        {
            callback = default_callback;
            callback->update_layout(windowX0, windowY0, -windowX0, -windowY0, cur_time_ms, ANIMATE_STARTSTOP);
        }
    }

    void get_window_bounds(GLdouble &x1, GLdouble &y1, GLdouble &x2, GLdouble &y2)
    {
        x1 = windowX0;
        x2 = -windowX0;
        y1 = windowY0;
        y2 = -windowY0;
    }

    Rect get_window_bounds()
    {
        Rect r;
        r.x1 = windowX0;
        r.x2 = -windowX0;
        r.y1 = windowY0;
        r.y2 = -windowY0;
        return r;
    }

    int get_texture_max_size() {return t_max_size;}

    void run();
    void start();
    void stop();
    void wait_quit();
    void layout_ui_plugins();

    static SRef<OpenGl_Window*> get_window(bool fullscreen);

    void add_display(SRef<OpenGl_Display*> d)
    {
        lock.lock();
        displays.push_back(d);
        lock.unlock();
        callback->add_display(d);
        std::vector<SRef<OpenGl_Ui*> >::iterator i;
        for (i=ui_apps.begin(); i!=ui_apps.end(); i++)
            (*i)->add_display(d);
        callback->update_layout(windowX0, windowY0, -windowX0, -windowY0, cur_time_ms, ANIMATE_STARTSTOP);
    }

    void remove_display(SRef<OpenGl_Display*> d)
    {
        lock.lock();
        displays.remove(d);
        lock.unlock();
        if(callback)
        {
            callback->remove_display(d);
            callback->update_layout(windowX0, windowY0, -windowX0, -windowY0, cur_time_ms, ANIMATE_STARTSTOP);
        }
    }

    struct mgl_gfx* load_texture(std::string name, int dim, int ncomp=4);
    struct mgl_gfx* load_image(std::string name);


    void draw_texture(struct mgl_gfx* t, float x1, float y1, float x2, float y2, float z, float alpha, int nrot, float tx1=0.0f, float ty1=0.0f, float tx2=1.0f, float ty2=1.0f);

    void draw_texture(struct mgl_gfx* t, float x1, float y1, float x2, float y2, float z, float alpha, /*float tx1, float ty1, float tx2, float ty2,*/ float radius, int radius_nsteps, bool softedge);

    void draw_rect(float x1, float y1, float x2, float y2, float z, uint32_t bgcol=0x000000, float bgtransp=1.0, float cornerRadius=0.0f, uint32_t bordercol=0xffffff, bool fill=true, bool drawBorder=false, float borderwidth=0.0f, int cornerNSteps=8, float bordertransp=1.0);

    SRef<Session_Registry*> get_session_registry();

    int get_window_width();
    int get_window_height();
    double get_gl_window_width();
    double get_gl_window_height();

    Text* get_text() {return text;}

    std::list<SRef<OpenGl_Display*> > get_displays()
    {
        lock.lock();
        std::list<SRef<OpenGl_Display*> > ret = displays;
        lock.unlock();
        return ret;
    }

    void touch_input(STouch& mt, bool screenCoord);

    uint64_t user_interaction_timer();
    bool layout_app_icons(bool visible, bool animate);
    void hide_apps();

    std::vector<SRef<OpenGl_Ui*> > get_ui_apps();

    bool is_fullscreen();

    /** Converts from screen coordinate (inside window) to OpenGl window coordinate */
    void screen2Gl(int sx, int sy, float&glx, float&gly);

private:
    void load_ui_plugins();
    static SRef<OpenGl_Window*> global_window_obj;
    Mini_Sip* minisip;
    SRef<Window_Callback*> callback;
    SRef<Window_Callback*> default_callback;

    std::list<SRef<OpenGl_Display*> > displays;

    std::vector<SRef<OpenGl_Ui*> > ui_apps;

    bool first_draw;

    void draw_surface();
    void draw_statistics();
    void sdl_quit();

    void toggle_fullscreen();
    void window_resized(int w, int h);

    void init();
    void init_surface();
    void find_screen_coords();
    void init_sdl_image();


    bool start_fullscreen;
    Mutex lock;
    SRef<Thread*> thread;
    int run_count;
    SRef<Semaphore*> start_wait_sem;
    bool do_stop;


    bool initialized;
    SDL_Surface* gdraw_surface;
    SDL_Window*  sdl_window;
    SDL_GLContext glcontext;
    int sdl_flags;
    int bpp;
    GLdouble windowX0;
    GLdouble windowY0;

    int windowed_width;
    int windowed_height;
    int native_width;
    int native_height;
    int cur_width;
    int cur_height;
    float screen_aratio;
    int t_max_size;

    Text *text;

    uint64_t cur_time_ms; //the run look keeps this up to day, so no code should have to call mtime() or similar
    int double_click_interval;
    int click_move_limit;
    int drag_move_limit;

    Statistics_Text_Plane statistics_plane;
    GLuint statistics_texture_name;
    bool statistics_drawing_enabled;
    uint64_t last_time_statistics_generated_ms;
    SRef<Session_Registry*> session_registry;
    uint64_t lastTimeUserInteraction_ms; //"mtime()" when there was last some user interaction. Can be used to show/hide UI elements for example

    bool appIconsVisible;

    int lastTouchId;
    int mouseEmulFromSx,mouseEmulFromSy;
    int mouseEmulLastSx,mouseEmulLastSy;
    float mouseEmulFromGlx, mouseEmulFromGly;
    float mouseEmulLastGlx,mouseEmulLastGly;
};

#endif // OPENGL_WINDOW_H
