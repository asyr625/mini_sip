#ifndef ANIMATE_H
#define ANIMATE_H

#include "sobject.h"

enum Animation_Type
{
    ANIMATE_NONE = 0,
    ANIMATE_CONSTANT = 1,
    ANIMATE_LINEAR,
    ANIMATE_EXPONENTIAL,
    ANIMATE_INVEXPONENTIAL,
    ANIMATE_STARTSTOP,
    ANIMATE_SINUS
};

// ANIMATE_EXPONENTIAL:
// y=x^2
//  |         .
//  |        .
//  |      .
//  |    .
//  |.
//  +-----------

// ANIMATE_STARTSTOP:
// Accelerating at start of interval, and slowing to halt at end
// Based on the sinus curve.
//   |           . .
//   |        .
//   |      .
//   |     .
//   |    .
//   |. .
//   +----------------

class Animate : public SObject
{
public:
    Animate(float fixedval, bool autoStart=true);
    Animate(int duration_ms, float startval, float endval, const int _type, bool autoStart=false);
    Animate(float startval, float increase_per_s, bool autoStart=false);

    virtual void start(uint64_t time = 0);

    uint64_t get_start_time();

    virtual float get_val(uint64_t time=0, int nsteps=0);

    virtual float get_end_val() { return endval; }

    virtual bool ended();

    virtual int get_duration() { return duration; }

    virtual int get_type() { return type; }

    static SRef<Animate*> animate(uint64_t ms, float v1, float v2, int type)
    {
        if ( type == ANIMATE_CONSTANT )
            return new Animate(v2);
        else
            return new Animate(ms, v1, v2, type);
    }

private:
    uint64_t time_ms();

    float startval;
    int type;

    bool continuous;
    float delta;
    //for non-continuous animations
    int duration;
    uint64_t start_time;
    uint64_t end_time;
    float endval;
};

enum rectanglearea
{
    TRIANGLE_TOP,
    TRIANGLE_BOTTOM,
    TRIANGLE_LEFT,
    TRIANGLE_RIGHT
};

class Rect
{
public:
    float x1,y1,x2,y2;

    float h() { return y2-y1; }
    float w() { return x2-x1; }

    rectanglearea get_triangle_area(float x, float y)
    {
        float topd = y2 - y;
        float bottomd = y - y1;
        float leftd  = x - x1;
        float rightd = x2 - x;

        if (topd <= bottomd && topd <= leftd && topd <= rightd)
            return TRIANGLE_TOP;
        else if (bottomd <= topd && bottomd <= leftd && bottomd <= rightd)
            return TRIANGLE_BOTTOM;
        else if (leftd <= topd && leftd <= bottomd && leftd <= rightd)
            return TRIANGLE_LEFT;
        else
            return TRIANGLE_RIGHT;
    }
};

class Bounds
{
public:
    Bounds();
    bool contains(float x, float y, uint64_t t=0);

    float get_x1(uint64_t t) { return x1->get_val(t); }
    float get_end_x1() { return x1->get_end_val(); }
    float get_y1(uint64_t t) { return y1->get_val(t); }
    float get_end_y1() { return y1->get_end_val(); }
    float get_x2(uint64_t t) { return x2->get_val(t); }
    float get_end_x2() { return x2->get_end_val(); }
    float get_y2(uint64_t t) { return y2->get_val(t); }
    float get_end_y2() { return y2->get_end_val(); }

    void get_relative_position(float glx, float gly, float& out_x, float& out_y);

    void animate(float _x1, float _y1, float _x2, float _y2, int type, uint64_t ms, uint64_t now);
    void animate_relative(float dx, float dy, int type, uint64_t ms, uint64_t now);

    void animate(float factor, int type, uint64_t ms, uint64_t now, float centerx=0.5f, float centery=0.5f);

    void set(SRef<Animate*> x1, SRef<Animate*> y1, SRef<Animate*> x2, SRef<Animate*> _y2);

    Rect get_rect(uint64_t t=0);
    Rect get_rect_end_val();

    std::string get_string();
    Rect get_rect_aspect_ratio(float aratio, uint64_t t=0, int halign=0, int valign=0);

private:
    SRef<Animate*> x1;
    SRef<Animate*> y1;
    SRef<Animate*> x2;
    SRef<Animate*> y2;
};

#endif // ANIMATE_H
