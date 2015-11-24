#ifndef VIDEO_CAFE_OUTPUT_H
#define VIDEO_CAFE_OUTPUT_H

#include "sobject.h"
#include "opengl_display.h"
#include "opengl_window.h"

#include<list>

/**
 Video is output is a single OpenGL window. The layout is initially a grid.
 The user can make a video bigger by clicking it. If clicking it when it
 is "full screen", then layout is changed back to a grid.

 Modes:

 +----------------+    +----------------+     +----------------+     +----------------+    +-----------------+
 |  A  .  B  .  C |    |         .      |     |           .    |     |              . |    |               o |
 |................|    |         .      |     |           .    |     |              . |    |               o |
 |  D  .  E  .  F | -> |         .      | ->  |           .    | ->  |              . | -> |               o | ->back
 |................|    |         .      |     |           .    |     |              . |    |               o |
 |  G  .  H  .  I |    |         .      |     |           .    |     |              . |    |               o |
 +----------------+    +................+     +----------------+     +----------------+    +-----------------+

 Start:
   1.  grid
   2.  (1280/1920) percent of width (do 1280/1920 percent instead?)
   3.  75 percent
   4.  90 percent
   5. 100 percent
 Local video shown when mouse moves.
*/

class Icon_Button
{
public:
    Icon_Button(std::string texture)
        : max_alpha(1.0f), texture_name(texture), /*bounds( 0.0F, 0.0F, 0.0F, 0.0F),*/ icon(NULL), n90rot(0)
    {
    }

    void set_position(float x1, float y1, float x2, float y2, Animation_Type animation, uint64_t ms, uint64_t cur_time)
    {
        bounds.animate(x1,y1,x2,y2, animation, ms, cur_time);
    }

    void set_max_alpha(float alpha)
    {
        max_alpha = alpha;
    }

    void draw(OpenGl_Window* window, float xtransl, float z, uint64_t cur_time)
    {
        float a = max_alpha;
        if (alpha)
        {
            a = alpha->get_val(cur_time);
            if (a<0.0001)
                return;
        }

        if (!icon)
        {
            icon = window->load_image(texture_name);
            my_assert(glGetError()==GL_NO_ERROR);
        }
        window->draw_texture(icon,
                             bounds.get_x1(cur_time) - xtransl,
                             bounds.get_y1(cur_time),
                             bounds.get_x2(cur_time) - xtransl,
                             bounds.get_y2(cur_time),
                             z, a,n90rot);
    }

    Bounds& get_bounds()
    {
        return bounds;
    }

    void rotate90(int n90)
    {
        n90rot=n90;
    }

    void fade_in()
    {
        if (alpha)
        {
            alpha = alpha->animate(250, alpha->get_val(), max_alpha, ANIMATE_STARTSTOP);
        }else{
            alpha = new Animate(250,0.0,max_alpha, ANIMATE_STARTSTOP);
        }
        alpha->start();
    }

    void fade_out()
    {
        if (alpha)
        {
            alpha = alpha->animate(250, alpha->get_val(), 0.0, ANIMATE_LINEAR);
        }
        else
        {
            alpha = new Animate(0.0);
        }
        alpha->start();
    }

    bool is_shown()
    {
        return alpha && alpha->get_val() >= 0.01;
    }
private:
    float max_alpha;
    SRef<Animate*> alpha;

    std::string texture_name;
    Bounds bounds;
    struct mgl_gfx * icon;
    int n90rot;
};

class Video_Cafe_Display: public OpenGl_Display
{
public:
    Video_Cafe_Display(std::string _callId, uint32_t _ssrc, bool _fullscreen);
    virtual std::string get_mem_object_type() const {return "VCafeDisplay";}

    virtual void draw(uint64_t cur_time);

    bool is_disabled() { return disabled; }

    void set_position(float x1, float y1, float x2, float y2, Animation_Type animation=ANIMATE_CONSTANT, uint64_t anim_ms=0, uint64_t cur_time=0);
    Bounds& get_bounds() { return bounds; }
    void set_draw_on_top(bool onTop) { draw_on_top = onTop; }
    bool get_draw_on_top() { return draw_on_top; }
    void fade_in();
    void fade_out();
    void show_pan_tilt_zoom();
    void hide_pan_tilt_zoom();

    bool consume_mouse_click(float glx, float gly, int button);

    void calc_displayed_window_size();
    void save_actual_displayed_window_size();
    virtual int get_displayed_width();
    virtual int get_displayed_height();
    bool get_supports_pan_tilt_zoom() { return pan_tilt_zoom_supported; }

    /**
         * @return true of layout should be updated
        */
    bool maintenance(uint64_t timeNow);
private:
    bool disabled;
    bool draw_on_top;

    bool pan_tilt_zoom_supported;
    Icon_Button pan_left_button;
    Icon_Button pan_right_button;
    Icon_Button tilt_up_button;
    Icon_Button tilt_down_button;

    int displayed_width, displayed_height;
    int previous_width, previous_height;

    uint64_t last_rtcp_app_update;
};

class Video_Window_Callback: public Window_Callback
{
public:
    Video_Window_Callback(OpenGl_Window* window);
    enum Layout_Mode {LAYOUT_GRID, LAYOUT_ZOOM1, LAYOUT_ZOOM2, LAYOUT_ZOOM3, LAYOUT_ZOOM4};
    /**
         * @param sx Screen x coordinate of new mouse position
         * @param sy Screen y coordinate of new mouse position
         * @param glx OpenGl x coordinate of new mouse position
         * @param gly OpenGl y coordinate of new mouse position
        */
    virtual bool mouse_move(int sx, int sy, float glx, float gly);
    virtual bool mouse_down(int sx, int sy, float glx, float gly, int button);
    virtual bool mouse_up(int sx, int sy, float glx, float gly, int button);
    virtual bool mouse_click(int sx, int sy, float glx, float gly, int button);
    virtual bool mouse_double_click(int sx, int sy, float glx, float gly, int button);
    virtual bool mouse_drag_move(int from_sx, int from_sy, float from_glx, float from_gly,  int cur_sx, int cur_sy, float cur_glx, float cur_gly, int button, int delta_sx, int delta_sy, float delta_glx, float delta_gly);
    virtual bool mouse_drag_release(int from_sx, int from_sy, float from_glx, float from_gly,  int cur_sx, int cur_sy, float cur_glx, float cur_gly, int button);


    virtual bool supports_touch() { return false; }
    virtual void touch(STouch &mt){}

    /**
         * Called when
         *  . A new display is added or removed
         *  . The screen has been resized (including from/to fullscreen)
        */
    virtual void update_layout(float x1, float y1, float x2, float y2, uint64_t cur_time, Animation_Type anim);
    void update_layout(uint64_t cur_time, Animation_Type anim);
    virtual void draw(uint64_t timeNow);

    virtual void add_display(SRef<OpenGl_Display*> d);
    virtual void remove_display(SRef<OpenGl_Display*> d);
    SRef<Video_Cafe_Display*> get_display_at(float glx, float gly);

    void draw_logo(uint64_t now);

private:
    void maintenance(uint64_t timeNow);

    OpenGl_Window* window;
    int drag_status;
    SRef<Video_Cafe_Display*> drag_cur_local_display;
    float drag_in_displayX,drag_in_displayY;

    Bounds bounds;
    Mutex display_list_lock;
    std::list<SRef<Video_Cafe_Display*> > videos;
    SRef<Video_Cafe_Display*> selected_stream;
    Layout_Mode layout_mode;
    int layout_animation_ms;
    uint64_t last_time_mouse_move;
    float last_mouseglX;
    float last_mouseglY;
    uint64_t fade_out_timer;
    bool display_local;

    Icon_Button* eit_ict_labs_button;
    uint64_t time_gui_start;
    bool show_splash;

    uint64_t last_dead_video_scan;

    Bounds logo_bounds;
};


class Video_Cafe_Display_Plugin: public Video_Display_Plugin
{
public:
    Video_Cafe_Display_Plugin( SRef<Library *> lib )
        : Video_Display_Plugin(lib)
    {
    }

    ~Video_Cafe_Display_Plugin(){}

    virtual std::string get_mem_object_type() const { return "VCafeDisplayPlugin"; }

    virtual std::string get_name() const { return "internal"; }

    virtual uint32_t get_version() const { return 0x00000001; }

    virtual std::string get_description() const { return "VCafe GUI video display"; }

    virtual SRef<Video_Display *> create( bool fullscreen, std::string callId, uint32_t ssrc ) const
    {
        return new Video_Cafe_Display(callId, ssrc, fullscreen );
    }
};

#endif // VIDEO_CAFE_OUTPUT_H
