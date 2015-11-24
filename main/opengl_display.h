#ifndef OPENGL_DISPLAY_H
#define OPENGL_DISPLAY_H

#include "video_display.h"
#include "animate.h"

void rect_to_coord(float &x1, float&y1, float &x2, float &y2, float lx1, float ly1, float lx2, float ly2, float aratio);

class OpenGl_Window;
struct mgl_gfx;

class OpenGl_Display : public Video_Display
{
public:
    OpenGl_Display(std::string _callId, uint32_t _ssrc, bool fullscreen);
    virtual void init(const uint32_t &width, const uint32_t &height);

    virtual void handle( const SRef<SImage *>& mimage );

    virtual void start();
    virtual void stop();

    struct mgl_gfx* get_texture(bool textureUpdate=false);

    virtual void set_is_local_video(bool isLocal);
    virtual bool get_is_local_video();

    virtual void set_video_name(std::string id);
    virtual void set_call_id(std::string id);

    bool is_hidden()
    {
        return hidden;
    }

    void set_hidden(bool h)
    {
        hidden = h;
    }

    uint64_t time_last_receive;
    Mutex data_lock;

    virtual int get_displayed_width() = 0;
    virtual int get_displayed_height() = 0;
    float get_pix_gl_ratio_width();
    float get_pix_gl_ratio_height();

    Bounds& get_bounds() { return bounds;}
    Rect get_video_area(uint64_t time=0);
    float get_alpha(uint64_t time=0) { if (alpha) return alpha->get_val(time); else return 1.0;}
    void set_alpha(float val) { alpha = new Animate(val);}

    OpenGl_Window* get_window() { return window; }

    virtual void draw(uint64_t cur_time, bool softedge=false);

protected:
    mgl_gfx *gfx;
    OpenGl_Window* window;

    Bounds bounds;
    SRef<Animate*> alpha;

private:
    bool is_local_video;
    bool hidden;
    bool show_zoom_icons;
    int color_nbytes;


    uint8_t *rgba;
    bool new_rgb_data;
    bool need_upload;

    uint32_t height;
    uint32_t width;
    bool fullscreen;

    float corner_radius_rercentage;
};

#endif // OPENGL_DISPLAY_H
