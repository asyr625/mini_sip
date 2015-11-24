#include "video_cafe_output.h"

#include "my_time.h"
#include "mini_sip.h"
#include <math.h>
#include <fstream>

using namespace std;

#define SPLASH_SIZE 0.5
#define SPLASH_TIME_MS 2500

#define LAYOUT_DEFAULT_MS 500 		//controls how fast layout changes are animates
#define FADEOUT_TIMER_DEFAULT 1000000000000LL      //controls how long until local video is faded out - 40 years (i.e. disable)

#define LOCAL_VIDEO_SIZE 0.175 //percentage of height - controls how big local video is displayed
#define LOGO_SIZE_PERCENTAGE 0.075

#define CLICKANIM ANIMATE_STARTSTOP//controls what type of animation is used when clicking a video stream

#define RTCP_APP_VIEW_UPDATE_MS 10000

Video_Window_Callback::Video_Window_Callback(OpenGl_Window* win)
    : window(win),
      drag_status(0),
      layout_mode(LAYOUT_GRID),
      layout_animation_ms(LAYOUT_DEFAULT_MS),
      last_time_mouse_move(0),
      last_mouseglX(-1.0F),
      last_mouseglY(-1.0F),
      fade_out_timer(FADEOUT_TIMER_DEFAULT),
      display_local(true),
      //eit_ict_labs_button(NULL),
      show_splash(true),
      last_dead_video_scan(0)
{
}

bool Video_Window_Callback::mouse_move(int sx, int sy, float glx, float gly)
{
    last_mouseglX = glx;
    last_mouseglY = gly;
    last_time_mouse_move = my_time();
    if (!display_local)
    {
        display_local = true;

        display_list_lock.lock();
        list<SRef<Video_Cafe_Display*> >::iterator i;
        for (i=videos.begin(); i!=videos.end(); i++)
        {
            if ( (*i)->get_is_local_video())
                (*i)->fade_in();
#if 0
            if ( (*i)->get_supports_pan_tilt_zoom())
                (*i)->show_pan_tilt_zoom();
#endif
        }
        display_list_lock.unlock();
    }
    return true;
}

bool Video_Window_Callback::mouse_down(int sx, int sy, float glx, float gly, int button)
{
    return false;
}

bool Video_Window_Callback::mouse_up(int sx, int sy, float glx, float gly, int button)
{
    return false;
}

bool Video_Window_Callback::mouse_click(int sx, int sy, float glx, float gly, int button)
{
    list<SRef<Video_Cafe_Display*> >::iterator i;
    display_list_lock.lock();
    int nNonLocalVideo = 0;
    for (i=videos.begin(); i!=videos.end(); i++)
    {
        if ( !(*i)->is_disabled() && (*i)->get_is_local_video()==false )
            nNonLocalVideo++;
    }

    bool handled = false;
    for (i=videos.begin(); !handled && i!=videos.end(); i++)
    {
        if ( !(*i)->is_disabled() )
            handled = (*i)->consume_mouse_click( glx, gly, button);
    }

    if (!handled && nNonLocalVideo>1)
    {
        for (i=videos.begin(); i!=videos.end(); i++)
        {
            if ( !(*i)->is_disabled() && (*i)->get_is_local_video()==false &&  (*i)->get_bounds().contains(glx,gly) )
            {
                bool same = false;
                if (selected_stream)
                {
                    selected_stream->set_draw_on_top(false);
                    same =  **i == *selected_stream;
                }
                selected_stream = *i;
                selected_stream->set_draw_on_top(true);
                if (same)
                {
                    switch (layout_mode)
                    {
                    case LAYOUT_GRID:  layout_mode = LAYOUT_ZOOM1; break;
                    case LAYOUT_ZOOM1: layout_mode = LAYOUT_ZOOM2; break;
                    case LAYOUT_ZOOM2: layout_mode = LAYOUT_ZOOM3; break;
                    case LAYOUT_ZOOM3: layout_mode = LAYOUT_ZOOM4; break;
                    case LAYOUT_ZOOM4: layout_mode = LAYOUT_GRID;  break;
                    }

                }else{
                    if (layout_mode == LAYOUT_GRID)
                        layout_mode = LAYOUT_ZOOM1;
                }
                update_layout(0, CLICKANIM);
                break;
            }
        }
    }
    display_list_lock.unlock();
    return true;
}

bool Video_Window_Callback::mouse_double_click(int sx, int sy, float glx, float gly, int button)
{
    return false;
}

void Video_Window_Callback::update_layout(float x1, float y1, float x2, float y2, uint64_t cur_time, Animation_Type anim)
{
    bounds.animate(x1,y1,x2,y2, anim, layout_animation_ms, cur_time);
    display_list_lock.lock();
    update_layout(cur_time, anim);
    display_list_lock.unlock();
}

void Video_Window_Callback::update_layout(uint64_t cur_time, Animation_Type anim)
{
    int nVideo = 0;
    int nLocal = 0;

    list<SRef<Video_Cafe_Display*> >::iterator i;
    for (i=videos.begin(); i!=videos.end(); i++)
    {
        if (!(*i)->is_disabled())
        {
            if ((*i)->get_call_id()=="localVideoDisplay")
            {
                nLocal++;
                (*i)->set_is_local_video(true);
            } else
                nVideo++;
        }
    }

    if (nVideo < 2)
        layout_mode = LAYOUT_GRID;

    float x1 = bounds.get_end_x1();
    float y1 = bounds.get_end_y1();
    float x2 = bounds.get_end_x2();
    float y2 = bounds.get_end_y2();

    float w = x2 - x1;
    logo_bounds.animate(x2-w*LOGO_SIZE_PERCENTAGE,  y2-w*LOGO_SIZE_PERCENTAGE, x2, y2, ANIMATE_CONSTANT, 100, cur_time);

    bool preferHorisontal = true;
    switch (layout_mode)
    {
    case LAYOUT_ZOOM1:
    case LAYOUT_ZOOM2:
    case LAYOUT_ZOOM3:
    case LAYOUT_ZOOM4:
        if (selected_stream && !selected_stream->is_disabled())
        {
            float selectedWidth = 1.0;//percentage of width, ZOOM4
            if (layout_mode == LAYOUT_ZOOM1)
                selectedWidth = 1280.0f/1920.0f;
            if (layout_mode == LAYOUT_ZOOM2)
                selectedWidth = 0.75f;
            if (layout_mode == LAYOUT_ZOOM3)
                selectedWidth = 0.9f;

            //layout selected video
            float vidX2 = x1 + (x2-x1) * selectedWidth;
            selected_stream->set_position(x1,y1, vidX2, y2, anim, layout_animation_ms, cur_time);

            if (nVideo>1)
            {
                //layout non-selected videos in right part of screen
                float vidHeight = (y2-y1) / (nVideo-1);

                int vidpos = 0;
                for (i=videos.begin(); i!=videos.end(); i++)
                {
                    if ( !(*i)->is_disabled() && selected_stream &&  **i != *selected_stream && (*i)->get_is_local_video()==false )
                    {
                        (*i)->set_position(vidX2, y1 + vidHeight * vidpos,
                                           x2,    y1+vidHeight * (vidpos+1), anim, layout_animation_ms, cur_time );
                        vidpos++;
                    }
                }
            }
            break;
        }

        //fall through to grid layout if no "selectedStream" or if selected stream is disabled.
    case LAYOUT_GRID:
    {
        int nr = 1;
        int nc = 1;
        while (nr * nc < nVideo) //find how many columns and rows to layout in
        {
            if (preferHorisontal){
                if (nr<nc) nr++; else nc++;
            }else{
                if (nc<nr) nc++; else nr++;
            }
        }
        int vidpos = 0;
        float gridSizeX = (x2-x1) / nc;
        float gridSizeY = (y2-y1) / nr;
        for (i=videos.begin(); i!=videos.end(); i++)
        {
            if ( !(*i)->is_disabled() && !(*i)->get_is_local_video() )
            {
                int gx = vidpos % nc;
                int gy = vidpos / nc;
                (*i)->set_position( x1+  gx   *gridSizeX, y1+ gy   *gridSizeY,
                                    x1+ (gx+1)*gridSizeX, y1+(gy+1)*gridSizeY,
                                    anim, layout_animation_ms, cur_time );
                vidpos++;
            }
        }
    }
    } //end switch



    int localPos = 0;
    float ly2 = y1 + LOCAL_VIDEO_SIZE*(y2-y1);
    //	float lw = (x2-x1)/nLocal;
    float lw = LOCAL_VIDEO_SIZE*(y2-y1)*16.0/9.0;

    float startx = x1+(x2-x1)/2.0 - lw*nLocal/2.0;

    for (i=videos.begin(); i!=videos.end(); i++)
    {
        if ( !(*i)->is_disabled() && (*i)->get_is_local_video() )
        {
            if ( (*i)->get_bounds().get_rect().h()<=0.001)
            {
                (*i)->set_position( startx+localPos*lw,    y1,
                                    startx+(localPos+1)*lw, ly2, anim, layout_animation_ms, cur_time );
                (*i)->set_draw_on_top(true);
                localPos++;
            }
        }
    }
}

bool Video_Window_Callback::mouse_drag_move(int from_sx, int from_sy, float from_glx, float from_gly,
                                            int cur_sx, int cur_sy, float cur_glx, float cur_gly,
                                            int button, int delta_sx, int delta_sy, float delta_glx, float delta_gly)
{
    if (drag_status == 0)
    {
        SRef<Video_Cafe_Display*> vd = get_display_at( from_glx, from_gly );
        if (vd && vd->get_is_local_video())
        {
            drag_cur_local_display = vd;
            drag_status = 1;

            drag_cur_local_display->get_bounds().get_relative_position(from_glx,from_gly, drag_in_displayX, drag_in_displayY);
            cerr<<"EEEE: relative position: "<<drag_in_displayX<<","<<drag_in_displayY<<endl;

#if 0
TODO: move are on first drag move here
                    Rect r = window->getWindowBounds();
            rectanglearea a = r.getTriangleArea(cur_glx, cur_gly);

            if (a==TRIANGLE_TOP || a==TRIANGLE_BOTTOM)
                dragCurLocalDisplay->get_bounds().animateRelative(delta_glx,0, ANIMATE_CONSTANT,1,0);
            else
                dragCurLocalDisplay->get_bounds().animateRelative(0,delta_gly, ANIMATE_CONSTANT,1,0);
#endif
            //	vd->get_bounds().animateRelative(delta_glx, delta_gly, ANIMATE_CONSTANT, 1, 0);
        }else{
        }

    }else{

        if (drag_cur_local_display)
        {
            Rect wr = window->get_window_bounds();
            rectanglearea a = wr.get_triangle_area(cur_glx, cur_gly);

            Rect vr = drag_cur_local_display->get_bounds().get_rect_end_val();

            float adjust = 0; //used to limit dragging so that it is never done outside of window

            if (a == TRIANGLE_TOP || a == TRIANGLE_BOTTOM)
            {
                if ( cur_glx - vr.w()*drag_in_displayX < wr.x1 )
                    adjust = cur_glx-vr.w()*drag_in_displayX - wr.x1;
                if ( cur_glx+vr.w()*(1-drag_in_displayX)> wr.x2 )
                    adjust =  cur_glx+vr.w()*(1-drag_in_displayX) - wr.x2;
            }

            if (a == TRIANGLE_TOP)
            {
                drag_cur_local_display->get_bounds().animate( cur_glx-vr.w()*drag_in_displayX-adjust, wr.y2-vr.h(), cur_glx+vr.w()*(1-drag_in_displayX)-adjust, wr.y2, ANIMATE_CONSTANT,1,0);
            }
            if (a == TRIANGLE_BOTTOM)
            {
                drag_cur_local_display->get_bounds().animate( cur_glx-vr.w()*drag_in_displayX-adjust, wr.y1, cur_glx+vr.w()*(1-drag_in_displayX)-adjust, wr.y1+vr.h(), ANIMATE_CONSTANT,1,0);
            }

            if (a == TRIANGLE_LEFT || a == TRIANGLE_RIGHT)
            {
                if ( cur_gly-vr.h()*drag_in_displayY < wr.y1 )
                    adjust = cur_gly-vr.h()*drag_in_displayY - wr.y1;
                if ( cur_gly+vr.h()*(1-drag_in_displayY) > wr.y2 )
                    adjust = cur_gly+vr.h()*(1-drag_in_displayY)-wr.y2;
            }

            if (a == TRIANGLE_LEFT)
            {
                drag_cur_local_display->get_bounds().animate( wr.x1, cur_gly-vr.h()*drag_in_displayY-adjust,  wr.x1+vr.w(), cur_gly+vr.h()*(1-drag_in_displayY)-adjust, ANIMATE_CONSTANT,1,0);
            }

            if (a==TRIANGLE_RIGHT)
            {
                drag_cur_local_display->get_bounds().animate( wr.x2-vr.w(), cur_gly-vr.h()*drag_in_displayY-adjust,  wr.x2, cur_gly+vr.h()*(1-drag_in_displayY)-adjust, ANIMATE_CONSTANT,1,0);
            }
            //	if (a==TRIANGLE_TOP || a==TRIANGLE_BOTTOM)
            //		drag_cur_local_display->get_bounds().animate_relative(delta_glx,0, ANIMATE_CONSTANT,1,0);
            //	else
            //		drag_cur_local_display->get_bounds().animate_relative(0,delta_gly, ANIMATE_CONSTANT,1,0);

        }
        /*
           SRef<VCafeDisplay*> vd = getDisplayAt(cur_glx-delta_glx, cur_gly-delta_gly);
           massert(vd);
           vd->get_bounds().animateRelative(delta_glx, delta_gly, ANIMATE_CONSTANT, 1, 0);
        */
    }
    return false;
}

bool Video_Window_Callback::mouse_drag_release(int from_sx, int from_sy, float from_glx, float from_gly,  int cur_sx, int cur_sy, float cur_glx, float cur_gly, int button)
{
    drag_status = 0;
#if 0
    lastx = -1;
    lasty = -1;
    return true;
#endif
    drag_cur_local_display = NULL;
    return false;
}

void Video_Window_Callback::add_display(SRef<OpenGl_Display*> d)
{
#ifdef DEBUG_OUTPUT
    my_assert( dynamic_cast<Video_Cafe_Display*>(*d) );
#endif
    int alphapercentage = window->get_minisip()->get_config()->_video_transparency_percentage;
    float alpha = (float)alphapercentage / 100.0;
    if (alpha >= 0.01 && alpha <= 1.00001)
        d->set_alpha(alpha);
    display_list_lock.lock();
    videos.push_back((Video_Cafe_Display*)*d);
    display_list_lock.unlock();
}

void Video_Window_Callback::remove_display(SRef<OpenGl_Display*> d)
{
    display_list_lock.lock();
    videos.remove((Video_Cafe_Display*)*d);
    display_list_lock.unlock();
}

SRef<Video_Cafe_Display*> Video_Window_Callback::get_display_at(float glx, float gly)
{
    std::list<SRef<Video_Cafe_Display*> >::iterator i;
    display_list_lock.lock();

    for (i=videos.begin(); i!=videos.end(); i++)
    {
        cerr <<"Checking "<<glx<<","<<gly<<" against "<< (*i)->get_bounds().get_string()<<endl;
        if ( (*i)->get_bounds().contains(glx, gly) )
        {
            SRef<Video_Cafe_Display*> vd = *i;
            display_list_lock.unlock();
            cerr<<"FOUND"<<endl;
            return vd;
        }
    }
    display_list_lock.unlock();

    cerr<<"NOT FOUND"<<endl;
    return NULL;
}

string nowstr()
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    string nowmin = itoa(tm.tm_min);
    if (tm.tm_min<10)
        nowmin = "0"+nowmin;

    string nowh = itoa(tm.tm_hour);

    if (tm.tm_sec%2==0){
        return nowh+":"+nowmin;
    }else{
        return nowh+" "+nowmin;
    }
}

const float DEG2RAD = 3.14159/180;

void drawCircle(float x, float y, float radius)
{
    glDisable(GL_TEXTURE_2D);
    glColor4f(0,0,1,1);
    glBegin(GL_TRIANGLE_FAN);
    for (int i=0; i < 360; i++)
    {
        float degInRad = i*DEG2RAD;
        glVertex3f(x+cos(degInRad)*radius, y+sin(degInRad)*radius,0);
    }
    glEnd();
    glEnable(GL_TEXTURE_2D);
}

void Video_Window_Callback::draw_logo(uint64_t now)
{
    static const SDL_Color white = {255,255,255};
    static const SDL_Color black = {0,0,0};
    static const SDL_Color blue = {0,0,255};
    Bounds& b = logo_bounds;
    uint64_t now = my_time();
    float h = b.get_y2(now) - b.get_y1(now);
    float offset = h * 0.33;

    //glEnable(GL_BLEND);
    glColor4f(1,1,1,0.8);
    drawCircle(b.getX1(now)+h*0.5+0.1, b.getY1(now)+h*0.5+0.1, h*0.825);
    glColor4f(1,1,1,0.8);

    window->get_text()->draw3D(b.get_x1(now),b.get_y1(now)+h*0.5,0,b.get_x2(now),b.get_y2(now),0, "Video", 48, white, black, TEXT_ALIGN_CENTER);
    window->get_text()->draw3D(b.get_x1(now),b.get_y1(now)+0.1,0,b.get_x2(now),b.get_y2(now)-h*0.5+0.25,0, "Caf3", 48, white, black, TEXT_ALIGN_CENTER);
    //	glDisable(GL_BLEND);
    window->get_text()->draw3D(b.get_x1(now),b.get_y1(now)-offset+0.20,0,b.get_x2(now),b.get_y1(now)+0.15,0, nowstr(), 24, white, black, TEXT_ALIGN_CENTER);
}

void Video_Window_Callback::draw(uint64_t timeNow)
{
#if 0
    if (!eit_ict_labs_button)
    {
        eit_ict_labs_button = new Icon_Button("eitictlabs_logo");
        eit_ict_labs_button->set_max_alpha(0.75);
        update_layout(timeNow, ANIMATE_CONSTANT);
        eit_ict_labs_button->fade_in();
        time_gui_start = timeNow;
    }
#endif
    display_list_lock.lock();
    if (timeNow - last_time_mouse_move > fade_out_timer)
    {
        list<SRef<Video_Cafe_Display*> >::iterator i;
        for (i=videos.begin(); i!=videos.end(); i++)
        {
            if ((*i)->is_disabled())
                continue;
            if ( display_local && (*i)->get_is_local_video() )
            {
                if ((*i)->get_bounds().contains( lastMouseglX, lastMouseglY ))
                {
                    last_time_mouse_move = timeNow; // don't hide local video if mouse is hovering
                }else{
                    display_local = false;
                    (*i)->fade_out();
                }
            }
#if 0
            if ( (*i)->get_supports_pan_tilt_zoom())
            {
                (*i)->hide_pan_tilt_zoom();
            }
#endif
        }
    }

    my_assert(glGetError()==GL_NO_ERROR);
    //	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0, 0.0f, DRAW_Z );

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    vector<SRef<OpenGl_Ui*> > uiApps = window->get_ui_apps();

    list<SRef<Video_Cafe_Display*> >::iterator i;
#if 0
    for (i=videos.begin(); i!=videos.end(); i++)
    {
        if ((*i)->is_disabled())
            continue;
        (*i)->calc_displayed_window_size();
        (*i)->save_actual_displayed_window_size();
    }
#endif
    for (i=videos.begin(); i!=videos.end(); i++)
    {
        if ((*i)->is_disabled())
            continue;
        if (!(*i)->get_draw_on_top())
        {
            (*i)->draw(timeNow);

            for (auto j=uiApps.begin(); j!=uiApps.end(); j++)
                if ( (*j)->has_video_decoration() )
                    (*j)->draw_video_decoration(**i);
        }
    }

    for (i=videos.begin(); i!=videos.end(); i++)
    {
        if ((*i)->is_disabled())
            continue;
        if ((*i)->get_draw_on_top() && (*i)->get_is_local_video()==false)
        {
            (*i)->draw(timeNow);
            for (auto j=uiApps.begin(); j!=uiApps.end(); j++)
                if ( (*j)->has_video_decoration() )
                    (*j)->draw_video_decoration(**i);
        }
    }
    for (i=videos.begin(); i!=videos.end(); i++)
    {
        if ((*i)->is_disabled())
            continue;
        if ((*i)->get_draw_on_top() && (*i)->get_is_local_video() )
        {
            (*i)->draw(timeNow);
            for (auto j=uiApps.begin(); j!=uiApps.end(); j++)
                if ( (*j)->has_video_decoration() )
                    (*j)->draw_video_decoration(**i);
        }
    }

#if 0
    if (eit_ict_labs_button->is_shown())
    {
        eit_ict_labs_button->draw(window, 0.0,0.0, timeNow);
        if ( show_splash && timeNow-time_gui_start > SPLASH_TIME_MS)
        {
            eit_ict_labs_button->fade_out();
            eit_ict_labs_button->get_bounds().animate(8,ANIMATE_EXPONENTIAL,500, timeNow);
            show_splash=false;
        }
    }
#endif
    display_list_lock.unlock();

    if (timeNow-last_dead_video_scan > 1000)
    {
        last_dead_video_scan = timeNow;
        maintenance(timeNow);
    }
    draw_logo( timeNow );
}

void Video_Window_Callback::maintenance(uint64_t timeNow)
{
    bool doLayout=false;
    display_list_lock.lock();
    list<SRef<Video_Cafe_Display*> >::iterator i;
    for (i=videos.begin(); i!=videos.end(); i++)
    {
        doLayout |= (*i)->maintenance(timeNow);
    }
    display_list_lock.unlock();
    if (doLayout)
        update_layout(timeNow, ANIMATE_CONSTANT);
}

Video_Cafe_Display::Video_Cafe_Display(std::string _callId, uint32_t _ssrc, bool _fullscreen)
    : OpenGl_Display(_callId, _ssrc, _fullscreen/*, config*/),
      draw_on_top(false),
      pan_tilt_zoom_supported(false),
      pan_left_button("rightarrow"),
      pan_right_button("rightarrow"),
      tilt_up_button("rightarrow"),
      tilt_down_button("rightarrow"),
      previous_width(0),
      previous_height(0),
      last_rtcp_app_update(0)
{
    disabled = false;
    tilt_down_button.rotate90(1);
    pan_left_button.rotate90(2);
    tilt_up_button.rotate90(3);

}

void Video_Cafe_Display::draw(uint64_t cur_time)
{
    bool softedge = true;
    OpenGl_Display::draw( cur_time, softedge);

    /* Periodical RTCP APP View */
    //cerr<<hex<<ssrc<<dec<<" curtime="<<cur_time<<" last_rtcp_app_update="<<last_rtcp_app_update<<endl;
    if(cur_time-last_rtcp_app_update > RTCP_APP_VIEW_UPDATE_MS)
    {
        last_rtcp_app_update = cur_time;
        calc_displayed_window_size();
        displayed_size_change( displayed_width, displayed_height );
    }
#if 0
    if (panTiltZoomSupported){
        pan_left_button.draw(window, 0.0,0.0, cur_time);
        pan_right_button.draw(window,0.0,0.0, cur_time);
        tilt_up_button.draw(window, 0.0,0.0, cur_time);
        tilt_down_button.draw(window, 0.0,0.0, cur_time);
    }
#endif
}

void Video_Cafe_Display::set_position(float x1, float y1, float x2, float y2, Animation_Type animation, uint64_t anim_ms, uint64_t cur_time)
{
    bounds.animate(x1,y1,x2,y2, animation, ms, cur_time);

    float bw = (x2-x1)/16.0;
    float midy = y1+(y2-y1)/2.0;
    float midx = x1+(x2-x1)/2.0;

#if 0
    pan_left_button.set_position(  x1,          midy-bw/2.0, x1+bw,       midy+bw/2.0, animation, ms, cur_time);
    pan_right_button.set_position( x2-bw,       midy-bw/2.0, x2,          midy+bw/2.0, animation, ms, cur_time);
    tilt_up_button.set_position(   midx-bw/2.0, y2-bw,       midx+bw/2.0, y2,          animation, ms, cur_time);
    tilt_down_button.set_position( midx-bw/2.0, y1,          midx+bw/2.0, y1+bw,       animation, ms, cur_time);
#endif

    calc_displayed_window_size();

    if (previous_width != displayed_width || previous_height != displayed_height){//FIXME: uninitialized variable
//		last_rtcp_app_update = cur_time;
        displayed_size_change( displayed_width, displayed_height );
    }
}

void Video_Cafe_Display::fade_in()
{
    if (alpha)
    {
        alpha = alpha->animate(250, alpha->get_val(), 1.0, ANIMATE_STARTSTOP);
    }
    else
    {
        alpha = new Animate(250,0.0,1.0, ANIMATE_STARTSTOP);
    }
    alpha->start();
}

void Video_Cafe_Display::fade_out()
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

void Video_Cafe_Display::show_pan_tilt_zoom()
{
    pan_left_button.fade_in();
    pan_right_button.fade_in();
    tilt_up_button.fade_in();
    tilt_down_button.fade_in();
}

void Video_Cafe_Display::hide_pan_tilt_zoom()
{
    pan_left_button.fade_out();
    pan_right_button.fade_out();
    tilt_up_button.fade_out();
    tilt_down_button.fade_out();
}

bool Video_Cafe_Display::consume_mouse_click(float glx, float gly, int button)
{
#if 0
    if (!pan_tilt_zoom_supported)
        return false;
    if (!pan_left_button.is_shown())
        return false;

    if (pan_left_button.get_bounds().contains(glx,gly)){
        camera_ctrl(-64, 0, 0, 500);
        return true;
    }

    if (pan_right_button.get_bounds().contains(glx,gly)){
        camera_ctrl(64, 0, 0, 500);
        return true;
    }
    if (tilt_up_button.get_bounds().contains(glx,gly)){
        camera_ctrl(0,64, 0, 500);
        return true;
    }
    if (tilt_down_button.get_bounds().contains(glx,gly)){
        camera_ctrl(0,-64, 0, 500);
        return true;
    }
#endif
    return false;
}

void Video_Cafe_Display::calc_displayed_window_size()
{
    float x1,x2,y1,y2;

    float aratio = gfx->aratio;

    float lx1 = bounds.get_end_x1();
    float lx2 = bounds.get_end_x2();
    float ly1 = bounds.get_end_y1();
    float ly2 = bounds.get_end_y2();
    rect_to_coord(x1,y1,x2,y2, lx1,ly1,lx2,ly2, aratio);

    /* Dislpayed video size in Pixels  */
    displayed_height = (y2-y1) * get_pix_gl_ratio_height();
    displayed_width = (x2-x1) * get_pix_gl_ratio_width();
}

void Video_Cafe_Display::save_actual_displayed_window_size()
{
    previous_width = displayed_width;
    previous_height = displayed_height;
}

int Video_Cafe_Display::get_displayed_width()
{
    return displayed_width;
}

int Video_Cafe_Display::get_displayed_height()
{
    return displayed_height;
}

bool Video_Cafe_Display::maintenance(uint64_t timeNow)
{
    bool oldval = disabled;
    if (timeNow > time_last_receive && timeNow - time_last_receive > 2200)
    {
        disabled=true;
    }
    else
    {
        disabled=false;
    }
    return oldval != disabled;
}
