#include "animate.h"

#include "my_time.h"
#include "my_error.h"
#include "my_assert.h"

#include <iostream>
#include <math.h>
#include <sstream>

using namespace std;

#define PI 3.1415926535

Animate::Animate(float fixedval, bool autoStart)
{
    startval = endval = fixedval;
    start_time = 0;
    end_time = 0;

    duration = -1;

    type = ANIMATE_CONSTANT;
    continuous = true;
    delta = 0;

    if (autoStart)
        start();
}

Animate::Animate(int duration_ms, float _startval, float _endval, const int _type, bool autoStart)
{
    my_assert(duration_ms > 0);
    start_time = 0;
    end_time = 0;

    duration = duration_ms;

    startval = _startval;
    endval = _endval;
    type = _type;
    if (type == ANIMATE_SINUS)
        continuous = true;
    else
        continuous = false;
    delta = 0.0F;

    if (autoStart)
        start();
}

Animate::Animate(float _startval, float increase_per_s, bool autoStart)
{
    start_time = 0;
    end_time = 0;

    duration = -1;

    startval = _startval;
    endval = -1;
    type = ANIMATE_LINEAR;
    continuous = true;
    delta = increase_per_s;
    if (autoStart)
        start();
}

void Animate::start(uint64_t time )
{
    if (time == 0)
        start_time = time_ms();
    else
        start_time = time;
    if (!continuous)
        end_time = start_time + duration;
}

uint64_t Animate::get_start_time()
{
    return start_time;
}

float Animate::get_val(uint64_t time, int nsteps)
{
    uint64_t stime = start_time;
    uint64_t now;
    if ( time == 0)
        now = time_ms();
    else
        now = time;

    if (stime == 0)
        stime = now;

    uint64_t ms = now - stime;

    if (!continuous && nsteps>0)
    {
        int modulo = (duration) / nsteps;
        ms = ms + modulo / 2;
        ms -= ms % modulo;
    }

    float p, sinscale;
    switch(type)
    {
    case ANIMATE_NONE:
    case ANIMATE_CONSTANT:
        return endval;
    case ANIMATE_LINEAR:
        if (continuous){
            return startval + (float)ms/1000.0F * delta;
        }else{
            if (ms>=duration)
                return endval;
            p = (float)ms/(float)duration;
            return startval + p*(endval-startval);
        }
        break;
    case ANIMATE_STARTSTOP:
    {
        if (ms>=duration)
            return endval;
        p = (float)ms/(float)duration;
        p = p*PI;
        sinscale = (sin(p-PI/2.0F)+1.0F)/2.0F; // in 0..1 when p=0..pi range
        return startval + sinscale*(endval-startval);
    }
    case ANIMATE_EXPONENTIAL:
    {
        if (ms>=duration)
            return endval;
        p = (float)ms/(float)duration;
        return startval + p*p*(endval-startval);
    }
    case ANIMATE_INVEXPONENTIAL:
    {
        if (ms>=duration)
            return endval;
        p = (float)ms/(float)duration;
        return startval + sqrt(p)*(endval-startval);
    }

    case ANIMATE_SINUS:
    {
        ms = ms % duration;
        p = (float)ms/(float)duration;
        float ret= startval + ((-cos(p*2.0*PI)+1.0)/2)*(endval-startval);
        return ret ;
    }
    default:
        my_error("Unimplemented animation type");
        break;
    };
    my_error("Animate: but in Animate::get_val - could not calculate animation");
    return startval;
}

bool Animate::ended()
{
    if (continuous)
        return false;

    uint64_t now = time_ms();
    if ( now >= end_time)
        return true;
    else
        return false;
}

uint64_t Animate::time_ms()
{
    return my_time();
}


Bounds::Bounds()
{
    x1 = new Animate(0.0);
    y1 = new Animate(0.0);
    x2 = new Animate(0.0);
    y2 = new Animate(0.0);
    x1->start(0);
    y1->start(0);
    x2->start(0);
    y2->start(0);
}

bool Bounds::contains(float x, float y, uint64_t t)
{
    return x >= get_x1(t) && x <= get_x2(t) && y >= get_y1(t) && y <= get_y2(t);
}

void Bounds::get_relative_position(float glx, float gly, float& out_x, float& out_y)
{
    Rect r = get_rect();
    out_x = (glx-r.x1)/(r.x2-r.x1);
    out_y = (gly-r.y1)/(r.y2-r.y1);
}

void Bounds::animate(float _x1, float _y1, float _x2, float _y2, int type, uint64_t ms, uint64_t now)
{
    my_assert( ms > 0 );
    x1 = x1->animate(ms, x1->get_val(now), _x1, type);
    y1 = y1->animate(ms, y1->get_val(now), _y1, type);
    x2 = x2->animate(ms, x2->get_val(now), _x2, type);
    y2 = y2->animate(ms, y2->get_val(now), _y2, type);
    x1->start(now);
    y1->start(now);
    x2->start(now);
    y2->start(now);
}

void Bounds::animate_relative(float dx, float dy, int type, uint64_t ms, uint64_t now)
{
    my_assert( ms > 0 );
    float curx1 = x1->get_val(now);
    float curx2 = x2->get_val(now);
    float cury1 = y1->get_val(now);
    float cury2 = y2->get_val(now);
    x1=x1->animate(ms, curx1, curx1 + dx, type);
    y1=y1->animate(ms, cury1, cury1 + dy, type);
    x2=x2->animate(ms, curx2, curx2 + dx, type);
    y2=y2->animate(ms, cury2, cury2 + dy, type);
    x1->start(now);
    y1->start(now);
    x2->start(now);
    y2->start(now);
}

void Bounds::animate(float factor, int type, uint64_t ms, uint64_t now, float centerx, float centery)
{
    float _x1 = x1->get_val(now);
    float _x2 = x2->get_val(now);
    float xnewsize = (_x2-_x1)*factor;
    float xchange = xnewsize-(_x2-_x1);
    my_assert( ms > 0 );

    x1 = x1->animate(ms, _x1, _x1 - xchange * centerx, type);
    x2 = x2->animate(ms, _x2, _x2 + xchange * (1.0f - centerx), type);

    float _y1 = y1->get_val(now);
    float _y2 = y2->get_val(now);
    float ynewsize = (_y2-_y1)*factor;
    float ychange = ynewsize-(_y2-_y1);

    y1=y1->animate(ms, _y1, _y1 - ychange * centery, type);
    y2=y2->animate(ms, _y2, _y2 + ychange * (1.0f - centery), type);
    x1->start(now);
    y1->start(now);
    x2->start(now);
    y2->start(now);
}

void Bounds::set(SRef<Animate*> _x1, SRef<Animate*> _y1, SRef<Animate*> _x2, SRef<Animate*> _y2)
{
    x1 = _x1;
    y1 = _y1;
    x2 = _x2;
    y2 = _y2;
}

Rect Bounds::get_rect(uint64_t t)
{
    Rect r;
    if (!t)
        t = my_time();
    if (!x1)
    {
        r.x1=r.y1=r.x2=r.y2=0.0f;
    }
    else
    {
        r.x1 = get_x1(t);
        r.y1 = get_y1(t);
        r.x2 = get_x2(t);
        r.y2 = get_y2(t);
    }
    return r;
}

Rect Bounds::get_rect_end_val()
{
    Rect r;
    if (!x1)
    {
        r.x1=r.y1=r.x2=r.y2=0.0f;
    }
    else
    {
        r.x1 = get_end_x1();
        r.y1 = get_end_y1();
        r.x2 = get_end_x2();
        r.y2 = get_end_y2();
    }
    return r;
}

std::string Bounds::get_string()
{
    uint64_t now = my_time();
    std::ostringstream out;

    out<<"x1="<<x1->get_val(now)<<"->"<<x1->get_end_val()<<" y1="<<y1->get_val(now)<<"->"<<y1->get_end_val()<<" x2="<<x2->get_val(now)<<"->"<<x2->get_end_val()<<" y2="<<y2->get_val(now)<<"->"<<y2->get_end_val();

    return out.str();
}

/**
 @param align -1=left, 0=center, 1=right
*/
static void rect_to_coord(float &x1, float&y1, float &x2, float &y2,
                          float lx1, float ly1, float lx2, float ly2, float aratio, int halign=0, int valign=0)
{
    float l_aratio = (lx2 - lx1) / (ly2 - ly1);
    if (aratio > l_aratio) // fill full width, center vertically
    {
        float h = (lx2-lx1)/aratio;
        float lh = ly2-ly1;
        x1 = lx1;
        y1 = ly1 + ((lh-h)/2.0);
        x2 = lx2;
        y2 = y1 + h;
        if (halign != 0)
        {
            if (halign<0){
            }else{
            }
        }
    }
    else  //fill full height
    {
        y1=ly1;
        float w=(ly2-ly1)*aratio;
        float lw=lx2-lx1;
        x1=lx1+(lw-w)/2.0;
        y2=ly2;
        x2=x1+w;
        if (valign!=0)
        {
            if (valign<0){
            }else{
            }
        }
    }
}

Rect Bounds::get_rect_aspect_ratio(float aratio, uint64_t t, int halign, int valign)
{
    Rect r=get_rect(t);
    Rect r2;
    rect_to_coord( r2.x1, r2.y1, r2.x2, r2.y2,
                   r.x1,  r.y1,  r.x2,  r.y2,
                   aratio, halign, valign );
    return r2;
}
