#include "opengl_window.h"

#include<math.h>
#include "my_time.h"

#include <sys/time.h>

#include "SDL.h"
#include "SDL_syswm.h"
#include "SDL_opengl.h"
#include "SDL_image.h"
#include "SDL_video.h"

#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glxext.h>

#include <iostream>
#include <fstream>

#include "session_registry.h"
#include "string_utils.h"
#include "mini_sip.h"

using namespace std;

#define DOUBLECLICK_INTERVAL_DEFAULT 250 //in milli-seconds
#define CLICK_MOVE_LIMIT_DEFAULT 20 //in pixels in either x or y axis
#define DRAG_MOVE_LIMIT_DEFAULT 10 //pixels

//size of plugin UI icons in percentage of window width
#define APP_ICON_SIZE 0.10
#define APP_ICON_ANIMATION_MS 250
#define APP_ICON_HIDE_MS 100000000//5000
#define APP_ANIMATION_TYPE ANIMATE_LINEAR
#define APP_WINDOW_ANIMATION_MS 100

SRef<OpenGl_Window*> OpenGl_Window::global_window_obj = NULL;

Touch::Touch(int _x, int _y, long id)
    : x(_x) , y(_y), touchId(id)
{
}

string Touch::get_debug_string()
{
    return "x="+itoa(x)+" y="+itoa(y)+" id="+itoa(touchId);
}

string STouch::get_debug_string()
{
    string ret;
    for (int i=0; i<fingers.size(); i++)
        ret += "finger_"+itoa(i+1)+"{"+fingers[i].get_debug_string()+"} ";
    return ret;
}


OpenGl_Window::OpenGl_Window(int w, int h, bool fullscreen):
    minisip(NULL),
    first_draw(true),
    run_count(0),
    gdraw_surface(NULL),
    sdl_window(NULL),
    bpp(0),
    native_height(0),
    native_width(0),
    screen_aratio(1.0f),
    text(NULL),
    cur_time_ms(0),
    double_click_interval(DOUBLECLICK_INTERVAL_DEFAULT),
    click_move_limit(CLICK_MOVE_LIMIT_DEFAULT),
    drag_move_limit(DRAG_MOVE_LIMIT_DEFAULT),
    statistics_texture_name(0),
    session_registry(NULL),
    statistics_drawing_enabled(false),
    last_time_statistics_generated_ms(0),
    lastTimeUserInteraction_ms(0),
    appIconsVisible(false),
    lastTouchId(0)
{
    windowed_width = w;
    windowed_height = h;
    start_fullscreen = fullscreen;
    do_stop = false;
    initialized = false;
    OpenGl_Ui_Registry::get_instance()->add_listener(this);
}

OpenGl_Window::~OpenGl_Window()
{
    if(statistics_texture_name)
        glDeleteTextures(1, &statistics_texture_name);
}

void OpenGl_Window::set_minisip(Mini_Sip*m)
{
    minisip=m;
    my_assert(minisip);
    session_registry = minisip->get_subsystem_media()->get_session_registry();
}

Mini_Sip* OpenGl_Window::get_minisip()
{
    return minisip;
}

void OpenGl_Window::handle_command(std::string subsystem, const Command_String& cmd)
{
    if (cmd.get_op()=="newplugin")
    {
        load_ui_plugins();
    }
}

Command_String OpenGl_Window::handle_command_resp(std::string subsystem, const Command_String& cmd)
{
#ifdef DEBUG_OUTPUT
    cerr<<"WARNING: OpenGlWindow::handleCommandResp called - BUG?"<<endl;
#endif
}

SRef<OpenGl_Window*> OpenGl_Window::get_window(bool fullscreen)
{
    if (!global_window_obj)
    {
        global_window_obj = new OpenGl_Window(640,  360, fullscreen);
    }
    return global_window_obj;
}


void OpenGl_Window::init_sdl_image()
{
    int flags = IMG_INIT_JPG|IMG_INIT_PNG;
    int initted = IMG_Init(flags);
    if((initted & flags) != flags)
    {
        printf("IMG_Init: Failed to init required jpg and png support!\n");
        printf("IMG_Init: %s\n", IMG_GetError());
        my_assert(0);
    }
}

static inline int nextPowerOfTwo(int num)
{
    for (int i = 0x10000000; i > 0; i >>= 1)
    {
        if (num == i)
            return num;
        if (num & i)
            return (i << 1);
    }
    return 1;
}

void extendImage(int nColor, int inw, int inh, uint8_t* indata, int outw, int outh, uint8_t* outdata)
{
    int rlenin=nColor*inw;
    int rlenout=nColor*outw;
    for (int row=0; row<inh; row++)
    {
        memcpy(&outdata[row*rlenout], &indata[row*rlenin], rlenin );
        memset(&outdata[row*rlenout+inw*nColor], 0, rlenout-rlenin);
    }
    for (int row=inh; row<outh; row++)
        memset(&outdata[row*rlenout], 0, outw*nColor);
}

struct mgl_gfx* OpenGl_Window::load_texture(std::string name, int dim, int ncomp)
{
    string path = MINISIP_DATADIR + string("/") + name + ".raw";
    int len = dim*dim*4;
    byte_t *tmp = new byte_t[len+16];
    ifstream inf;
    inf.open(path.c_str(), ios::binary);
    if (inf.fail())
    {
        cerr<<"ERROR: could not open "<<path<<endl;
        return NULL;
    }

    inf.read((char*)tmp,len);
    struct mgl_gfx* t = new struct mgl_gfx;
    memset(t, 0, sizeof(struct mgl_gfx));
    t->is_selected = true;
    t->hu = 1.0;
    t->wu = 1.0;
    t->tex_dim=dim;

    glEnable(GL_TEXTURE_2D);
    glGenTextures( 1, (GLuint*)&(t->texture));
    if (glGetError()!=GL_NO_ERROR)
    {
        return NULL;
    }

    my_assert(t->texture > 0);
    glBindTexture( GL_TEXTURE_2D, t->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (ncomp==4)
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, dim, dim, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
    else
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, dim, dim, GL_RGB, GL_UNSIGNED_BYTE, tmp);
    my_assert(glGetError()==GL_NO_ERROR);
    t->name=strdup(name.c_str());
    delete[] tmp;
    if (glGetError()!=GL_NO_ERROR)
    {
        return NULL;
    }
    return t;
}

struct mgl_gfx* OpenGl_Window::load_image(std::string name)
{
    struct mgl_gfx* ret = new struct mgl_gfx;
    ret->texture = 0;
    float returnvalue = 1.0f;
    SDL_Surface *image;
    GLenum texture_format = 0;

    try{
        string path = name;
        if (name[0]!='/')
            path= MINISIP_DATADIR + string("/") + name;

        image = IMG_Load( path.c_str() );
        if (image)
        {
            ret->aratio = (float)image->w / image->h;

            unsigned char nOfColors = image->format->BytesPerPixel ;
            if (nOfColors== 4)     // contains an alpha channel
            {
                if (image->format->Rmask == 0x000000ff)
                {
                    texture_format = GL_RGBA;
                }else{
                    texture_format = GL_BGRA;
                }

            } else if (nOfColors == 3)     // no alpha channel
            {
                if (image->format->Rmask == 0x000000ff){
                    texture_format = GL_RGB;
                }else{
                    texture_format = GL_BGR;
                }
            } else {
                printf("warning: the image is not truecolor..  this will probably break num color =%u\n",nOfColors);
                // this error should not go unhandled
                texture_format = GL_RGB;
            }

            glGenTextures( 1, &(ret->texture) );
            glBindTexture( GL_TEXTURE_2D, ret->texture );
            // Set the texture's stretching properties

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );


            int sWidth  = nextPowerOfTwo( image->w );
            int sHeight = nextPowerOfTwo( image->h );
            if (sWidth > sHeight) sHeight = sWidth;
            if (sHeight > sWidth) sWidth = sHeight;
            ret->tex_dim = sHeight;
            void *imgdata = image->pixels;

            /* scale texture image to 2^m by 2^n if necessary */
            if ( sWidth == image->w && sHeight == image->h) {
            } else {

                imgdata = (GLubyte *)malloc( sHeight*sWidth*4*sizeof( GLubyte ) );
                extendImage(nOfColors, image->w, image->h, (uint8_t*)image->pixels, sWidth, sHeight, (uint8_t*)imgdata );
            }

            ret->wu = (float)image->w / sWidth;
            ret->hu = (float)image->h / sHeight;

            my_assert(glGetError()==GL_NO_ERROR);
            gluBuild2DMipmaps(GL_TEXTURE_2D, texture_format, sWidth, sHeight, texture_format, GL_UNSIGNED_BYTE, imgdata);
            my_assert(glGetError()==GL_NO_ERROR);

            if (image->pixels != imgdata)
                free(imgdata);
            SDL_FreeSurface( image);
        }
        else
        {
            if(image)SDL_FreeSurface( image);
            printf("SDL could not load image : %s\n", SDL_GetError());
            return NULL;
        }
    }
    catch(exception& e)
    {
        cout<<"Load image error! "<<  " "<<e.what() <<endl;
        if(image)SDL_FreeSurface( image);
        return NULL;
    }
    return ret;
}


static void drawLineLoop(float* x, float* y, float* z, int n, int width, uint32_t color, float transparency)
{
    glLineWidth(width);
    glColor4ub((color>>16)&0xFF,(color>>8)&0xFF,color&0xFF,transparency*255);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glBegin(GL_LINE_STRIP);
    for (int i=0; i<n; i++)
    {
        glVertex3f(x[i],y[i],z[i]);
    }
    glVertex3f(x[0],y[0],z[0]);
    glEnd();
}


void OpenGl_Window::draw_texture(struct mgl_gfx* t, float x1, float y1, float x2, float y2,
                                 float z, float alpha, int nrot, float tx1, float ty1, float tx2, float ty2)
{
    my_assert(t);
    float x[4];
    x[0] = tx1;
    x[1] = t->wu*tx2;
    x[2] = t->wu*tx2;
    x[3] = tx1;

    float y[4];
    y[0] = t->hu*ty2;
    y[1] = t->hu*ty2;
    y[2] = ty1;
    y[3] = ty1;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);

    my_assert(t->texture>0);

    glColor4f(1.0, 1.0, 1.0, alpha);
    glBindTexture( GL_TEXTURE_2D, t->texture);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glBegin( GL_QUADS );

    glTexCoord2f( x[nrot%4], y[nrot%4] );
    glVertex3f(  x1, y1, 0.0f );

    glTexCoord2f( x[(nrot+1)%4], y[(nrot+1)%4] );
    glVertex3f( x2, y1, 0.0f );

    glTexCoord2f( x[(nrot+2)%4] , y[(nrot+2)%4] );
    glVertex3f( x2, y2, 0.0f );

    glTexCoord2f( x[(nrot+3)%4], y[(nrot+3)%4] );
    glVertex3f( x1, y2, 0.0f );
    glEnd();

    glDisable(GL_TEXTURE_2D);
}



void OpenGl_Window::draw_texture(struct mgl_gfx* t,
                               float sx1, float sy1, float sx2, float sy2,
                               float z,
                               float alpha,
                               //	float tx1, float ty1, float tx2, float ty2,
                               float radiusPercentage, int radius_nsteps,
                               bool softedge)
{
    my_assert(t);

    bool border = true;

    if (radius_nsteps > 64)
        radius_nsteps = 64;
    float xarr[64*4+1];
    float yarr[64*4+1];
    float zarr[64*4+1];
    float xarr2[64*4+1];
    float yarr2[64*4+1];
    float zarr2[64*4+1];

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    my_assert(t->texture>0);

    glBindTexture(GL_TEXTURE_2D, t->texture);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);


    double rScreen = radiusPercentage * (sx2-sx1);
    double rTexture = radiusPercentage * t->wu; //BUG: FIXME: account for tx values
    int nsteps = radius_nsteps;
    double stepDegree = 90.0/(double)(nsteps-1.0);
    int i;

    //texture drawing is done clockwise from top left corner (x=0, y=max-radius)
    //screen drawing is done couner clockwise from bottom left corner

#define PI 3.1415926
    //calculate the four corner points offset by the radius
    int nv = 4*radius_nsteps;
    double anglerad = 0;

    //offset coordinates
    double scornerx[4];
    scornerx[0] = sx1 + rScreen;
    scornerx[1] = sx2 - rScreen;
    scornerx[2] = sx2 - rScreen;
    scornerx[3] = sx1 + rScreen;

    double scornery[4];
    scornery[0] = sy1 + rScreen;
    scornery[1] = sy1 + rScreen;
    scornery[2] = sy2 - rScreen;
    scornery[3] = sy2 - rScreen;

    //outer coordinates
    double socornerx[4];
    socornerx[0] = sx1;
    socornerx[1] = sx2;
    socornerx[2] = sx2;
    socornerx[3] = sx1;

    double socornery[4];
    socornery[0] = sy1;
    socornery[1] = sy1;
    socornery[2] = sy2;
    socornery[3] = sy2;

    //offset coordinates
    double tcornerx[4];
    tcornerx[0] = 0.0 + rTexture;
    tcornerx[1] = t->wu-rTexture;
    tcornerx[2] = t->wu-rTexture;
    tcornerx[3] = 0.0 + rTexture;

    double tcornery[4];
    tcornery[0] = t->hu-rTexture;
    tcornery[1] = t->hu-rTexture;
    tcornery[2] = 0.0 + rTexture;
    tcornery[3] = 0.0 + rTexture;

    //outercoordinates
    double tocornerx[4];
    tocornerx[0] = 0.0;
    tocornerx[1] = t->wu;
    tocornerx[2] = t->wu;
    tocornerx[3] = 0.0;

    double tocornery[4];
    tocornery[0] = t->hu;
    tocornery[1] = t->hu;
    tocornery[2] = 0.0;
    tocornery[3] = 0.0;



    double anglestart[4];
    anglestart[0] = PI*(180)/180.0;
    anglestart[1] = PI*(270)/180.0;
    anglestart[2] = PI*(0)/180.0;
    anglestart[3] = PI*(90)/180.0;

    int arri=0;

    //Draw the four corners with rounded edges
    for (int c=0; c<4; c++)
    {
        glBegin(GL_TRIANGLE_FAN);
        glColor4f(1.0f,1.0f,1.0f,alpha);
        glTexCoord2f( tcornerx[c], tcornery[c] );
        glVertex3f( scornerx[c], scornery[c], 0.0f );
        anglerad=anglestart[c];
        if (softedge)
            glColor4f(1.0f,1.0f,1.0f,0);
        for (i=0; i<nsteps; i++)
        {
            double tx = tcornerx[c] + cosf(-anglerad)*rTexture;
            double ty = tcornery[c] + sinf(-anglerad)*rTexture;
            double sx = scornerx[c] + cosf(anglerad)*rScreen;
            double sy = scornery[c] + sinf(anglerad)*rScreen;
            glTexCoord2f( tx, ty );
            glVertex3f( sx, sy, 0.0f );

            xarr[arri]=sx;
            yarr[arri]=sy;
            zarr[arri]=z;

            xarr2[arri]=scornerx[c] + cosf(anglerad)*rScreen*0.5;
            yarr2[arri]=scornery[c] + sinf(anglerad)*rScreen*0.5;
            zarr2[arri]=z;

            arri++;
            anglerad = anglerad + PI*(stepDegree)/180.0;
        }
        glEnd();
    }

    glColor4f(1.0f,1.0f,1.0f,alpha);
    glBegin( GL_QUADS );
    for (int c=0; c<4; c++)
    {
        glTexCoord2f( tcornerx[c], tcornery[c] );
        glVertex3f(  scornerx[c], scornery[c], 0.0f );
    }
    glEnd();


    //bottom
    glBegin( GL_QUADS );
    if (softedge)
        glColor4f(1.0f,1.0f,1.0f,0);
    glTexCoord2f( tcornerx[0], tocornery[0] );
    glVertex3f(  scornerx[0], socornery[0], 0.0f );

    //glColor4f(1.0f,1.0f,1.0f,0);
    glTexCoord2f( tcornerx[1], tocornery[1] );
    glVertex3f(  scornerx[1], socornery[1], 0.0f );

    glColor4f(1.0f,1.0f,1.0f,alpha);
    glTexCoord2f( tcornerx[1], tcornery[1] );
    glVertex3f( scornerx[1], scornery[1], 0.0f );

    //glColor4f(1.0f,1.0f,1.0f,alpha);
    glTexCoord2f( tcornerx[0], tcornery[0] );
    glVertex3f( scornerx[0], scornery[0], 0.0f );
    glEnd();


    //top
    glBegin( GL_QUADS );
    if (softedge)
        glColor4f(1.0f,1.0f,1.0f,0);
    glTexCoord2f( tcornerx[3], tocornery[3] );
    glVertex3f(  scornerx[3], socornery[3], 0.0f );

    //glColor4f(1.0f,1.0f,1.0f,0);
    glTexCoord2f( tcornerx[2], tocornery[2] );
    glVertex3f(  scornerx[2], socornery[2], 0.0f );

    glColor4f(1.0f,1.0f,1.0f,alpha);
    glTexCoord2f( tcornerx[2], tcornery[2] );
    glVertex3f( scornerx[2], scornery[2], 0.0f );

    //glColor4f(1.0f,1.0f,1.0f,alpha);
    glTexCoord2f( tcornerx[3], tcornery[3] );
    glVertex3f( scornerx[3], scornery[3], 0.0f );
    glEnd();

    //left
    glBegin( GL_QUADS );
    if (softedge)
        glColor4f(1.0f,1.0f,1.0f,0);
    glTexCoord2f( tocornerx[0], tcornery[0] );
    glVertex3f(  socornerx[0], scornery[0], 0.0f );

    //glColor4f(1.0f,1.0f,1.0f,0);
    glTexCoord2f( tocornerx[3], tcornery[3] );
    glVertex3f(  socornerx[3], scornery[3], 0.0f );

    glColor4f(1.0f,1.0f,1.0f,alpha);
    glTexCoord2f( tcornerx[3], tcornery[3] );
    glVertex3f( scornerx[3], scornery[3], 0.0f );

    //glColor4f(1.0f,1.0f,1.0f,alpha);
    glTexCoord2f( tcornerx[0], tcornery[0] );
    glVertex3f( scornerx[0], scornery[0], 0.0f );
    glEnd();


    //right
    glBegin( GL_QUADS );
    if (softedge)
        glColor4f(1.0f,1.0f,1.0f,0);
    glTexCoord2f( tocornerx[1], tcornery[1] );
    glVertex3f(  socornerx[1], scornery[1], 0.0f );

    //glColor4f(1.0f,1.0f,1.0f,0);
    glTexCoord2f( tocornerx[2], tcornery[2] );
    glVertex3f(  socornerx[2], scornery[2], 0.0f );

    glColor4f(1.0f,1.0f,1.0f,alpha);
    glTexCoord2f( tcornerx[2], tcornery[2] );
    glVertex3f( scornerx[2], scornery[2], 0.0f );

    //glColor4f(1.0f,1.0f,1.0f,alpha);
    glTexCoord2f( tcornerx[1], tcornery[1] );
    glVertex3f( scornerx[1], scornery[1], 0.0f );
    glEnd();

    if (/*border*/ true)
    {
        //draw border around the video here?
        int borderwidth = 2;
        int bordercolor = 0xa0a0a0;
        float bordertransparency = 0.5;
        glDisable(GL_TEXTURE_2D);
        drawLineLoop(xarr2,yarr2,zarr2, nv, borderwidth, bordercolor, bordertransparency);
        glEnable(GL_TEXTURE_2D);
    }

#if 0


    //bottom
    glBegin( GL_QUADS );

    glColor4f(1.0, 1.0, 1.0, 0);
    glTexCoord2f( tx1, ty2 );
    glVertex3f(  sx1, sy1, 0.0f );

    glColor4f(1.0, 1.0, 1.0, 0);
    glTexCoord2f( tx2, ty2 );
    glVertex3f( sx2, sy1, 0.0f );

    glColor4f(1.0, 1.0, 1.0, alpha);
    glTexCoord2f( tx2-txo , ty2-tyo );
    glVertex3f( sx2-sxo, sy1+syo, 0.0f );

    glColor4f(1.0, 1.0, 1.0, alpha);
    glTexCoord2f( tx1+txo, ty2-tyo );
    glVertex3f( sx1+sxo, sy1+syo, 0.0f );
    glEnd();


    //right
    glBegin( GL_QUADS );

    glColor4f(1.0, 1.0, 1.0, 0);
    glTexCoord2f( tx2-txo, ty2-tyo );
    glVertex3f(  sx2, sy1, 0.0f );

    glColor4f(1.0, 1.0, 1.0, 0);
    glTexCoord2f( tx2, ty1 );
    glVertex3f( sx2, sy2, 0.0f );

    glColor4f(1.0, 1.0, 1.0, alpha);
    glTexCoord2f( tx2-txo , ty1+tyo );
    glVertex3f( sx2-sxo, sy2-syo, 0.0f );

    glColor4f(1.0, 1.0, 1.0, alpha);
    glTexCoord2f( tx2-txo, ty2-tyo );
    glVertex3f( sx2-sxo, sy1+syo, 0.0f );
    glEnd();


    //middle part
    glBegin( GL_QUADS );

    glColor4f(1.0, 1.0, 1.0, alpha);
    glTexCoord2f( tx1+txo, ty2+tyo );
    glVertex3f(  sx1+sxo, sy1+syo, 0.0f );

    glColor4f(1.0, 1.0, 1.0, alpha);
    glTexCoord2f( tx2-txo, ty2-tyo );
    glVertex3f( sx2-sxo, sy1+syo, 0.0f );

    glColor4f(1.0, 1.0, 1.0, alpha);
    glTexCoord2f( tx2-txo , ty1+tyo );
    glVertex3f( sx2-sxo, sy2-syo, 0.0f );

    glColor4f(1.0, 1.0, 1.0, alpha);
    glTexCoord2f( tx1+txo, ty1+tyo );
    glVertex3f( sx1+sxo, sy2-syo, 0.0f );
    glEnd();

#endif
    glDisable(GL_TEXTURE_2D);
}


void drawPolygon(float *x, float* y, float *z, int n, uint32_t color, float transparency)
{
    glBegin(GL_POLYGON);
    glColor4ub((color>>16)&0xFF,(color>>8)&0xFF,color&0xFF,transparency*255);
    for (int i = 0; i < n; i++)
    {
        glVertex3f(x[i],y[i],z[i]);
    }
    glEnd();
}

void OpenGl_Window::draw_rect(float x1, float y1, float x2, float y2, float z,
                              uint32_t bgcolor, float bgtransparency, float cornerRadius, uint32_t bordercolor, bool fill,
                              bool drawBorder, float borderwidth, int cornerNSteps, float bordertransparency)
{
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (cornerNSteps > 64)
        cornerNSteps = 64;
    float xarr[64*4+1];
    float yarr[64*4+1];
    float zarr[64*4+1];
    //	float xarr2[64*4+1];
    //	float yarr2[64*4+1];
    //	float zarr2[64*4+1];

    int nv;

    if (cornerRadius < 0.00001)
    {
        nv=4;
        xarr[0]=x1;
        xarr[1]=x2;
        xarr[2]=x2;
        xarr[3]=x1;

        yarr[0]=y1;
        yarr[1]=y1;
        yarr[2]=y2;
        yarr[3]=y2;

        zarr[0]=z;
        zarr[1]=z;
        zarr[2]=z;
        zarr[3]=z;

    }else{
        nv = 4*cornerNSteps;
        float stepDegree = 90.0/(float)(cornerNSteps-1.0);
        double anglerad = 0;

        //calculate the four points that are in the center defining the corner circles
        double scornerx[4];
        scornerx[0] = x1+cornerRadius;
        scornerx[1] = x2-cornerRadius;
        scornerx[2] = x2-cornerRadius;
        scornerx[3] = x1+cornerRadius;

        double scornery[4];
        scornery[0] = y1+cornerRadius;
        scornery[1] = y1+cornerRadius;
        scornery[2] = y2-cornerRadius;
        scornery[3] = y2-cornerRadius;

        int arri=0;

        double anglestart[4];
        anglestart[0]=M_PI*(180)/180.0;
        anglestart[1]=M_PI*(270)/180.0;
        anglestart[2]=M_PI*(0)/180.0;
        anglestart[3]=M_PI*(90)/180.0;
        for (int corner=0; corner<4; corner++)
        {
            anglerad=anglestart[corner];
            for (int i=0; i<cornerNSteps; i++)
            {
                float sx = scornerx[corner] + cosf(anglerad)*cornerRadius;
                float sy = scornery[corner] + sinf(anglerad)*cornerRadius;
                xarr[arri]=sx;
                yarr[arri]=sy;
                zarr[arri]=z;
                //				xarr2[arri]=scornerx[corner] + cosf(anglerad)*cornerRadius*0.5;
                //				yarr2[arri]=scornery[corner] + sinf(anglerad)*cornerRadius*0.5;
                //				zarr2[arri]=z;

                arri++;
                anglerad = anglerad + M_PI*(stepDegree)/180.0;
            }
        }
    }

    if (fill)
        drawPolygon(xarr,yarr,zarr, nv, bgcolor, bgtransparency);
    if (drawBorder)
        drawLineLoop(xarr,yarr,zarr, nv, borderwidth, bordercolor, bordertransparency);

    glEnable(GL_TEXTURE_2D);

}

void OpenGl_Window::draw_surface()
{
    bool doSwap = false;
    vector<SRef<OpenGl_Ui*> >::iterator iter;

#if 0
    glEnable(GL_BLEND);
    for (iter = ui_apps.begin(); iter!=ui_apps.end(); iter++)
    {
        (*iter)->draw_background();
        doSwap = true;
    }
    glDisable(GL_BLEND);
#endif
#if 1
    for (iter = ui_apps.begin(); iter!=ui_apps.end(); iter++)
    {
        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
        (*iter)->draw_background();
        doSwap = true;
        glDisable(GL_BLEND);
    }
#endif

    if (callback)
    {
        if (first_draw)
        {
            first_draw = false;
            callback->update_layout(windowX0, windowY0, -windowX0, -windowY0, cur_time_ms, ANIMATE_NONE);
            layout_app_icons(true, false);
        }
        callback->draw(cur_time_ms);

        if(statistics_drawing_enabled)
            draw_statistics();

        //glPopMatrix();
        int err;
        if ( (err=glGetError())!=GL_NO_ERROR)
        {
            cerr <<"WARNING0: glGetError returned an error code: "<<err<<endl;
        }
        doSwap = true;
    }
    for (iter = ui_apps.begin(); iter!=ui_apps.end(); iter++)
    {
        if ( (*iter)->get_enabled() && (*iter)->has_app()  )
        {
            glEnable(GL_BLEND);
            (*iter)->draw_app();
            doSwap = true;
            glDisable(GL_BLEND);
        }
    }
    for (iter = ui_apps.begin(); iter!=ui_apps.end(); iter++)
    {
        if ( (*iter)->get_enabled() && (*iter)->has_widget()  )
        {
            (*iter)->draw_widget();
            doSwap = true;
        }
    }
    for (iter = ui_apps.begin(); iter!=ui_apps.end(); iter++)
    {
        if ( (*iter)->get_enabled() && (*iter)->has_icon()  )
        {
            glEnable(GL_BLEND);
            (*iter)->draw_icon();
            doSwap = true;
            glDisable(GL_BLEND);
        }
    }
    for (iter = ui_apps.begin(); iter!=ui_apps.end(); iter++)
    {
        if ( (*iter)->get_enabled() )
        {
            (*iter)->post_draw();
        }
    }

#if 0
    for(int i=0; i<(int)ui_apps.size();i++)
    {
        ui_apps[UIdrawingPriority[i]]->draw();
        doSwap=true;
    }
#endif

    if (doSwap)
    {
        //SDL_GL_SwapBuffers();
        SDL_GL_SwapWindow(sdl_window);
    }

    //Icon hide/show logic
    if (appIconsVisible && user_interaction_timer()>APP_ICON_HIDE_MS)
    {
        appIconsVisible=false;
        layout_app_icons(false,true);
    }
    else if (!appIconsVisible && user_interaction_timer()<APP_ICON_HIDE_MS)
    {
        appIconsVisible=true;
        layout_app_icons(true,true);
    }
}

void OpenGl_Window::draw_statistics()
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if(cur_time_ms >= last_time_statistics_generated_ms + 1000)
    {
        if (!session_registry)
        {
            my_err <<"WARNING: can not draw statistics - no session registry set!"<<endl;
            return;
        }
        statistics_plane.generate(session_registry->get_all_session());
        last_time_statistics_generated_ms = cur_time_ms;
    }

    const SDL_Surface *surface = statistics_plane.get_surface();
    if(statistics_texture_name)
        glDeleteTextures(1, &statistics_texture_name);
    glGenTextures(1, &statistics_texture_name);
    glBindTexture(GL_TEXTURE_2D, statistics_texture_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, surface->format->BytesPerPixel, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    float statisticsSurfaceAspectRatio = float(surface->w) / surface->h,
            screen_aratio = windowX0 / windowY0,
            statisticsPlaneWidthModifier = 1.f,
            statisticsPlaneHeightModifier = 1.f;
    if(statisticsSurfaceAspectRatio > screen_aratio)
        statisticsPlaneHeightModifier = screen_aratio / statisticsSurfaceAspectRatio;
    else
        statisticsPlaneWidthModifier = statisticsSurfaceAspectRatio / screen_aratio;

    glColor4f(1.f, 1.f, 1.f, 1.f);
    glBegin( GL_QUADS );

    glTexCoord2f( 0, 0 );
    glVertex2f( windowX0, -windowY0);

    glTexCoord2f( 1, 0 );
    glVertex2f( windowX0 * (1 - 2 * statisticsPlaneWidthModifier), -windowY0);

    glTexCoord2f( 1, 1 );
    glVertex2f( windowX0 * (1 - 2 * statisticsPlaneWidthModifier), windowY0 * (2 * statisticsPlaneHeightModifier - 1));

    glTexCoord2f( 0, 1 );
    glVertex2f( windowX0, windowY0 * (2 * statisticsPlaneHeightModifier - 1));

    glEnd();
}

void OpenGl_Window::layout_ui_plugins()
{
    vector<SRef<OpenGl_Ui*> >::iterator i;
    int nIcons = 0;
    int nApps = 0;
    int nWidgets = 0;
    int nAppsVisible = 0;
    int nWidgetsVisible = 0;
    uint64_t now = my_time();

    float width = APP_ICON_SIZE * fabsf(2.0f*windowX0);
    float curIconY2 = -windowY0 + width/2.0;
    float curIconY1 = curIconY2 - width;

    for (i = ui_apps.begin(); i!=ui_apps.end(); i++)
    {
        my_assert(*i);
        if (! (*i)->get_enabled())
            continue;
        if ((*i)->has_icon())
            nIcons++;
        if ((*i)->has_app())
        {
            nApps++;
            if ((*i)->app_visible())
                nAppsVisible++;
        }

        if ((*i)->has_widget())
        {
            nWidgets++;
            if ((*i)->widget_visible())
                nWidgetsVisible++;
        }
    }

    for (i = ui_apps.begin(); i!=ui_apps.end(); i++)
    {
        SRef<OpenGl_Ui*> ui=*i;
        if (! ui->get_enabled())
        {
            continue;
        }

        //		if ((*i)->hasIcon()){
        //			(*i)->setIconBounds( new Animate(windowX0),new Animate(curIconY1),new Animate(windowX0+width),new Animate(curIconY2), new Animate(1.0));
        //			curIconY2=curIconY2-width;
        //			curIconY1=curIconY1-width;
        //		}
        if (ui->has_app())
        {
            float x1,y1,x2,y2,alpha;
            if (ui->app_visible())
            {
                x1 = windowX0; //TODO: support multiple maximized apps
                y1 = windowY0;
                x2 = -windowX0;
                y2 = -windowY0;
                alpha = 1.0f;
            }
            else
            {
                x1 = x2 = windowX0;
                y1 = y2 = ui->iconBounds.get_y1(now);
                alpha = 0.0f;
            }
            float prevx1 = ui->appBounds.get_x1(now);
            float prevy1 = ui->appBounds.get_y1(now);
            float prevx2 = ui->appBounds.get_x2(now);
            float prevy2 = ui->appBounds.get_y2(now);
            float prevalpha = ui->appAlpha->get_val(now);
            ui->set_app_bounds( new Animate(APP_WINDOW_ANIMATION_MS, prevx1, x1, APP_ANIMATION_TYPE, true),
                                new Animate(APP_WINDOW_ANIMATION_MS, prevy1, y1, APP_ANIMATION_TYPE, true),
                                new Animate(APP_WINDOW_ANIMATION_MS, prevx2, x2, APP_ANIMATION_TYPE, true),
                                new Animate(APP_WINDOW_ANIMATION_MS, prevy2, y2, APP_ANIMATION_TYPE, true),
                                new Animate(APP_WINDOW_ANIMATION_MS, prevalpha, alpha, APP_ANIMATION_TYPE, true));
        }
    }
    layout_app_icons(appIconsVisible, true);
}

void OpenGl_Window::load_ui_plugins()
{
    my_dbg << "OpenGl_Window::load_ui_plugins() running" << endl;
    SRef<SPlugin_Registry*> reg = *OpenGl_Ui_Registry::get_instance();
    SPlugin_Registry::const_iterator iter;
    bool doLayout = false;
    for( iter = reg->begin(); iter !=  reg->end(); iter++ )
    {
        OpenGl_Ui_Plugin* uip = dynamic_cast<OpenGl_Ui_Plugin*>(* *iter);
        if (uip)
        {
            if (uip->seen){
            }else{
                uip->seen = true;
                SRef<OpenGl_Ui*> ui = uip->new_instance(this);
                my_dbg<< "OpenGlWindow::load_ui_plugins(): created plugin "<<uip->get_name()<<endl;
                ui->init();
                if (ui->has_icon())
                    ui->set_icon_bounds( new Animate(windowX0),new Animate(-1),new Animate(windowX0+2),new Animate(1), new Animate(0.0));
                if (ui->has_app())
                    ui->set_app_bounds( new Animate(windowX0),new Animate(0),new Animate(windowX0),new Animate(0), new Animate(0.0f));
                if (ui->has_layout_manager()){
                    callback = *ui;
                }

                ui_apps.push_back( ui );
                doLayout = true;
            }
        }
    }
    if (doLayout)
        layout_ui_plugins();
}


uint64_t OpenGl_Window::user_interaction_timer()
{
    uint64_t now = my_time();
    return now - lastTimeUserInteraction_ms;
}


void OpenGl_Window::run()
{
#ifdef DEBUG_OUTPUT
    set_thread_name("OpenGl_Window::run");
#endif
    if (!initialized)
        init();
    cur_time_ms = my_time(); //we should set the current time before signalling that we are done to avoid race condition
    //where a cur_time_ms zero value is used
    if (start_wait_sem)
    {
        start_wait_sem->inc();
    }
    if ((int)ui_apps.size()==0)
    {
        load_ui_plugins();
    }

    SDL_Event event={};
    uint64_t lastMouseDownTime = 0;
    int lastMouseDownX = -100;
    int lastMouseDownY = -100;
    float lastMouseMoveGLX = 0;
    float lastMouseMoveGLY = 0;
    int lastMouseMoveSX = 0;
    int lastMouseMoveSY = 0;

    uint64_t lastClickTime = 0;
    int lastClickX = -100;
    int lastClickY = -100;
    bool buttonDown = false;
    int  buttonDownButton = 0;
    bool isDrag = false;
    SDL_MouseButtonEvent *mousebutton = NULL;

    while(!do_stop)
    {
        while( SDL_PollEvent( &event ))
        {
            cur_time_ms = my_time();
            switch( event.type )
            {
            case SDL_MOUSEBUTTONDOWN:
                lastTimeUserInteraction_ms = cur_time_ms;
                mousebutton = &event.button;

                //printf("PRESS: %d,%d\n",event.button.x, event.button.y);
            {
                float glx, gly;
                glx = windowX0 + ((double)event.motion.x/(double)cur_width)*(-windowX0-windowX0);
                gly = windowY0 + ((double)(cur_height-event.motion.y)/(double)cur_height)*(-windowY0-windowY0);

                bool handled = false;

                lastMouseDownTime = cur_time_ms;
                lastMouseDownX = event.motion.x;
                lastMouseDownY = event.motion.y;
                lastMouseMoveSX= lastMouseDownX;
                lastMouseMoveSY= lastMouseDownY;
                lastMouseMoveGLX = glx;
                lastMouseMoveGLY = gly;
                buttonDown = true;
                buttonDownButton = event.button.button;

                for(int i=0; !handled && i<(int)ui_apps.size();i++)
                {
                    if(mousebutton->button == SDL_MOUSEBUTTONUP)
                        handled = ui_apps[i]->mouse_wheel_up();
                    else if(mousebutton->button == SDL_MOUSEBUTTONDOWN)
                        handled = ui_apps[i]->mouse_wheel_down();
                }

                if (!handled && callback)
                    callback->mouse_down(event.motion.x, event.motion.y, glx, gly,event.button.button);

            }
                break;

            case SDL_MOUSEBUTTONUP:
            {
                lastTimeUserInteraction_ms = cur_time_ms;
                float glx, gly;
                glx = windowX0 + ((double)event.motion.x/(double)cur_width)*(-windowX0-windowX0);
                gly = windowY0 + ((double)(cur_height-event.motion.y)/(double)cur_height)*(-windowY0-windowY0);
                bool clickHandled = false;
                bool dragHandled = false;

                bool isDoubleClick = false;

                int deltaX = lastMouseDownX - event.motion.x;
                int deltaY = lastMouseDownY - event.motion.y;
                if (deltaX<0) deltaX = -deltaX;
                if (deltaY<0) deltaY = -deltaY;

                if (deltaX<click_move_limit && deltaY<click_move_limit)
                {
                    deltaX = lastClickX - event.motion.x;
                    deltaY = lastClickY - event.motion.y;
                    if (deltaX<0) deltaX = -deltaX;
                    if (deltaY<0) deltaY = -deltaY;

                    if (cur_time_ms-click_move_limit <= double_click_interval && deltaX<click_move_limit && deltaY<click_move_limit)
                    {
                        isDoubleClick = true;
                    }else
                    {
                        lastClickTime = cur_time_ms;
                        lastClickX = event.motion.x;
                        lastClickY = event.motion.y;

                    }
                }

                bool doUiLayout = false;
                for (auto i = ui_apps.begin(); i!=ui_apps.end(); i++)
                {
                    if ( (*i)->iconBounds.contains(glx,gly) )
                    {
                        if (!(*i)->app_visible())
                            hide_apps();
                        (*i)->set_app_visible( !(*i)->app_visible() );
                        clickHandled = true;
                        doUiLayout = true;
                    }
                }
                if (doUiLayout)
                    layout_ui_plugins();

                for(int i=0; !clickHandled && i<(int)ui_apps.size();i++)
                {
                    if (ui_apps[i]->app_visible())
                    {
                        if (isDoubleClick)
                            clickHandled = ui_apps[i]->mouse_double_click(event.motion.x, event.motion.y, glx, gly,event.button.button);
                        else
                            clickHandled = ui_apps[i]->mouse_click(event.motion.x, event.motion.y, glx, gly,event.button.button);
                        clickHandled = true;
                    }
                }

                if (!clickHandled && callback)
                {
                    callback->mouse_click(event.motion.x, event.motion.y, glx, gly,event.button.button);
                }

                if (isDrag)
                {
                    float fglx, fgly;
                    fglx = windowX0 + ((double)lastMouseDownX / (double)cur_width) * (-windowX0-windowX0);
                    fgly = windowY0 + ((double)(cur_height-lastMouseDownY) / (double)cur_height) * (-windowY0-windowY0);

                    for(int i=0; !dragHandled && i<(int)ui_apps.size();i++)
                    {
                        dragHandled=ui_apps[i]->mouse_drag_release(lastMouseDownX, lastMouseDownY,fglx,fgly, event.motion.x, event.motion.y, glx, gly,event.button.button);
                    }
                    if (!dragHandled && callback)
                        callback->mouse_drag_release(lastMouseDownX, lastMouseDownY,fglx,fgly, event.motion.x, event.motion.y, glx, gly,event.button.button);
                }
                buttonDown = false;
                isDrag = false;

#if 0

                if (deltaX<click_move_limit && deltaY<click_move_limit)
                {
                    deltaX = lastClickX-event.motion.x;
                    deltaY = lastClickY-event.motion.y;
                    if (deltaX<0) deltaX = -deltaX;
                    if (deltaY<0) deltaY = -deltaY;

                    if (cur_time_ms-lastClickTime <= double_click_interval && deltaX<click_move_limit && deltaY<click_move_limit)
                    {
                        if (callback)
                            callback->mouse_double_click(event.motion.x, event.motion.y, glx, gly,event.button.button);
                    }
                    else
                    {
                        bool IspassEvent = true;
                        bool doUiLayout = false;

                        for (auto i = ui_apps.begin(); i!=ui_apps.end(); i++)
                        {
                            if ( (*i)->iconBounds.contains(glx,gly) )
                            {
                                if (!(*i)->app_visible())
                                    hide_apps();
                                (*i)->set_app_visible( !(*i)->app_visible() );
                                IspassEvent = false;
                                doUiLayout = true;
                            }
                        }

                        if (doUiLayout)
                            layout_ui_plugins();

                        if (callback && IspassEvent)
                            callback->mouse_click(event.motion.x, event.motion.y, glx, gly,event.button.button);

                    }

                    lastClickTime = cur_time_ms;
                    lastClickX = event.motion.x;
                    lastClickY = event.motion.y;
                }
                if (isDrag)
                {
                    float fglx, fgly;
                    fglx = windowX0 + ((double)lastMouseDownX/(double)cur_width)*(-windowX0-windowX0);
                    fgly = windowY0 + ((double)(cur_height-lastMouseDownY)/(double)cur_height)*(-windowY0-windowY0);

                    if (callback)
                        callback->mouse_drag_release(lastMouseDownX, lastMouseDownY,fglx,fgly, event.motion.x, event.motion.y, glx, gly,event.button.button);
                    for(int i=0; i<(int)ui_apps.size();i++)
                    {
                        ui_apps[i]->mouse_drag_release(lastMouseDownX, lastMouseDownY,fglx,fgly, event.motion.x, event.motion.y, glx, gly,event.button.button);
                    }
                }
                buttonDown = false;
                isDrag = false;
#endif
            }
                break;

            case SDL_MOUSEMOTION:
            {
                lastTimeUserInteraction_ms = cur_time_ms;
                float glx, gly;
                glx =   windowX0 + ((double)event.motion.x/(double)cur_width)*(-windowX0-windowX0);
                gly =   windowY0 + ((double)(cur_height-event.motion.y)/(double)cur_height)*(-windowY0-windowY0);
                bool handled = false;

                for(int i=0; !handled && i<(int)ui_apps.size();i++)
                {
                    handled = ui_apps[i]->mouse_move(event.motion.x, event.motion.y, glx, gly);
                }

                if (!handled && callback)
                    callback->mouse_move(event.motion.x, event.motion.y, glx, gly);

                if (buttonDown)
                {
                    handled = false;
                    int deltaX = lastMouseDownX - event.motion.x;
                    int deltaY = lastMouseDownY - event.motion.y;

                    if (deltaX<0) deltaX = -deltaX;
                    if (deltaY<0) deltaY = -deltaY;
                    if (deltaX>drag_move_limit || deltaY>drag_move_limit)
                        isDrag = true;

                    if (isDrag && callback)
                    {
                        float fglx, fgly;
                        fglx = windowX0 + ((double)lastMouseDownX/(double)cur_width)*(-windowX0-windowX0);
                        fgly = windowY0 + ((double)(cur_height-lastMouseDownY)/(double)cur_height)*(-windowY0-windowY0);

                        for(int i=0; !handled && i<(int)ui_apps.size();i++)
                        {
                            handled=ui_apps[i]->mouse_drag_move(lastMouseDownX, lastMouseDownY,
                                                             fglx, fgly, event.motion.x, event.motion.y,
                                                             glx, gly,
                                                             buttonDownButton,
                                                             event.motion.x-lastMouseMoveSX, event.motion.y-lastMouseMoveSY,
                                                             glx-lastMouseMoveGLX, gly-lastMouseMoveGLY);
                        }

                        if (!handled && callback)
                            callback->mouse_drag_move(lastMouseDownX, lastMouseDownY,
                                                    fglx, fgly,
                                                    event.motion.x, event.motion.y,
                                                    glx, gly,
                                                    buttonDownButton,
                                                    event.motion.x-lastMouseMoveSX, event.motion.y-lastMouseMoveSY,
                                                    glx-lastMouseMoveGLX, gly-lastMouseMoveGLY);
                        lastMouseMoveSX = event.motion.x;
                        lastMouseMoveSY = event.motion.y;
                        lastMouseMoveGLX = glx;
                        lastMouseMoveGLY = gly;
                    }
                }
            }
                break;

            case SDL_QUIT:
                sdl_quit();
                return;
            case SDL_VIDEORESIZE:
                window_resized(event.resize.w, event.resize.h);
                break;

            case SDL_KEYDOWN:
                lastTimeUserInteraction_ms=cur_time_ms;

                //trap quit and fullscreen events. Forward everything else

                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    if (is_fullscreen())
                        toggle_fullscreen();
                }

                if (event.key.keysym.sym == SDLK_RETURN && event.key.keysym.mod & KMOD_ALT)
                {
                    toggle_fullscreen();
                }

                if (event.key.keysym.sym == SDLK_s && event.key.keysym.mod & KMOD_ALT)
                {
                    statistics_drawing_enabled = !statistics_drawing_enabled;
                }
                for(int i=0; i<(int)ui_apps.size();i++)
                {
                    ui_apps[i]->consumekey(event.key);
                }
                break;
            };
        }
        cur_time_ms = my_time();
        draw_surface();
    }
    statistics_plane.clear();
    TTF_Quit();
    SDL_Quit();
}


void OpenGl_Window::start()
{
    lock.lock();
    bool useSem = false;

    if (run_count <= 0)
    {
        start_wait_sem = new Semaphore();
        useSem = true;
        do_stop = false;
        if (run_count==0)
        {
            my_assert(!thread);
            thread = new Thread(this, Thread::Normal_Priority);
        }
    }
    run_count ++;
    lock.unlock();
    if (useSem)
        start_wait_sem->dec();
}


void OpenGl_Window::stop()  //NOTE: this method is used from external thread
{
    lock.lock();
    run_count--;
    if (run_count==0)
    {
        do_stop = true;
        for (int i=0; i<ui_apps.size(); i++)
            ui_apps[i]->stop();
        my_assert(thread);
        thread->join();
        thread = NULL;
    }
    initialized = false;
    lock.unlock();
}

void OpenGl_Window::wait_quit()
{
    if (thread)
        thread->join();
}


void OpenGl_Window::find_screen_coords()  //NOTE: must be called by internal thread
{
    GLdouble x = 0;
    GLdouble y = 0;
    GLdouble z = 0;

    GLdouble model[16];
    GLdouble proj[16];
    GLint view[4];
    glGetDoublev(GL_PROJECTION_MATRIX, &proj[0]);
    glGetDoublev(GL_MODELVIEW_MATRIX, &model[0]);
    glGetIntegerv(GL_VIEWPORT, &view[0]);


    GLdouble winx;
    GLdouble winy;
    GLdouble winz;
    GLdouble delta = -DRAW_Z;
    int i;
    for (i=0; i<64; i++)
    {
        int ret = gluProject(x,y,z, model,proj,view, &winx, &winy, &winz);

        if (winx < 0)
            x = x + delta;
        else
            x = x - delta;
        delta = delta / 2;
    }
    windowX0 = x;

    x = 0;
    delta = -DRAW_Z;

    for (i=0; i<64; i++)
    {
        int ret = gluProject(x,y,z, model,proj,view, &winx, &winy, &winz);

        if ( winy < 0 )
            y = y + delta;
        else
            y = y - delta;
        delta = delta / 2;
    }
    windowY0 = y;
    int err;
    if ( (err=glGetError())!=GL_NO_ERROR)
    {
        cerr <<"WARNING0: OpenGL returned error code:"<<err<<endl;
    }
}


void OpenGl_Window::sdl_quit() //NOTE: must be called by internal thread
{
    if (is_fullscreen() )
    {
        toggle_fullscreen();
    }
    SDL_Quit();
}

void OpenGl_Window::toggle_fullscreen() //NOTE: must be called by internal thread
{
    sdl_flags ^= SDL_WINDOW_FULLSCREEN;//SDL_FULLSCREEN;

    init_surface();

    callback->update_layout(windowX0, windowY0, -windowX0, -windowY0, cur_time_ms, ANIMATE_CONSTANT);
    layout_app_icons(appIconsVisible,false);
}

bool OpenGl_Window::is_fullscreen()
{
    if (sdl_flags & SDL_WINDOW_FULLSCREEN) //SDL_FULLSCREEN
        return true;
    else
        return false;
}


void OpenGl_Window::window_resized(int w, int h)  //NOTE: must be called by internal thread
{
    if (!is_fullscreen())
    {
        windowed_width = w;
        windowed_height = h;
    }

    screen_aratio=(float)w/(float)h;

    init_surface();

    callback->update_layout(windowX0, windowY0, -windowX0, -windowY0, cur_time_ms, ANIMATE_NONE);
    layout_app_icons(appIconsVisible, false);
    callback->draw(cur_time_ms);
    //glPopMatrix();

    //SDL_GL_SwapBuffers();
    SDL_GL_SwapWindow(sdl_window);
}

void OpenGl_Window::init()
{
    initialized = true;

    if( SDL_Init(SDL_INIT_VIDEO) < 0 )
    {
        fprintf(stderr,"Failed to initialize SDL Video!\n");
        exit(1);
    }

#if 1
    if(TTF_Init())
    {
        fprintf(stderr,"Failed to initialize SDL_TTF!\n");
        exit(1);
    }

    init_sdl_image();

    //SDL_EnableUNICODE(1);
    text = new Text("/usr/share/fonts/truetype/freefont/FreeSans.ttf");
#endif


    // tell system which funciton to process when exit() call is made
    //      atexit(SDL_Quit);

    // get optimal video settings
    const SDL_VideoInfo* vidinfo = SDL_GetVideoInfo();
    if(!vidinfo)
    {
        fprintf(stderr,"Coudn't get video information!\n%s\n", SDL_GetError());
        exit(1);
    }

    const SDL_RendererInfo ddd;
    if (!native_height){ //the size of the desktop is only available the first
        //time this function is called. The second time
        //it is the size of the current window.
        native_width = vidinfo->current_w;
        native_height = vidinfo->current_h;
    }
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,        5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,      5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,       5);
#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,      32);
#else
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,      16);
#endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,    1);


    /* We want to enable synchronizing swapping buffers to
     * vsynch to avoid tearing. SDL_GL_SWAP_CONTROL does
     * not (always?) work under linux, so we try to
     * use GL extensions.
     */

    //SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL,    1);
    SDL_GL_SetSwapInterval(1);


    SDL_putenv((char*)"__GL_SYNC_TO_VBLANK=1");

    sdl_flags = SDL_WINDOW_OPENGL | SDL_RESIZABLE | SDL_HWSURFACE;

    if (start_fullscreen)
        sdl_flags = sdl_flags | SDL_WINDOW_FULLSCREEN; //SDL_FULLSCREEN

    bpp = vidinfo->vfmt->BitsPerPixel; //attribute used by initSurface

    init_surface( );
}


void OpenGl_Window::init_surface()  //NOTE: must be called by internal thread
{
    int w,h;
    if (sdl_flags & SDL_WINDOW_FULLSCREEN) //SDL_FULLSCREEN
    {
        w = native_width;
        h = native_height;
    }else{
        w = windowed_width;
        h = windowed_height;
    }
    cur_width = w;
    cur_height = h;
#if 0
    gdraw_surface = SDL_SetVideoMode(w,h, bpp, sdl_flags | SDL_DOUBLEBUF);
    if( !gdraw_surface )
    {
        fprintf(stderr,"Couldn't set video mode!\n%s\n", SDL_GetError());
        exit(1);
    }
#endif
    sdl_window = SDL_CreateWindow("sdl_XX",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    glcontext = SDL_GL_CreateContext(sdl_window);

    my_assert(glGetError()==GL_NO_ERROR);
    GLint texSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
    t_max_size = texSize;

    my_assert(glGetError()==GL_NO_ERROR);
    //glShadeModel(GL_SMOOTH);
    glClearColor(0.0F,0.0F,0.0F,0);
    glViewport(0,0,cur_width,cur_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (GLfloat) cur_width/(GLfloat) cur_height, 1.0, 500.0);
    my_assert(glGetError()==GL_NO_ERROR);

    if (text)
        text->restart_gl();

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,        5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,      5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,       5);
#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,      32);
#else
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,      16);
#endif

    //SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL,    1);
    SDL_GL_SetSwapInterval(1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,    1);

    my_assert(glGetError()==GL_NO_ERROR);

    glDisable(GL_DEPTH_TEST);

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glPushMatrix();
    glTranslatef( 0, 0.0f, DRAW_Z);
    find_screen_coords();
    glPopMatrix();

    my_assert(glGetError()==GL_NO_ERROR);
}

SRef<Session_Registry*> OpenGl_Window::get_session_registry()
{
    return session_registry;
}

int OpenGl_Window::get_window_width()
{
    return cur_width;
}

int OpenGl_Window::get_window_height()
{
    return cur_height;
}

double OpenGl_Window::get_gl_window_width()
{
    return -2.0*windowX0;
}

double OpenGl_Window::get_gl_window_height()
{
    return -2.0*windowY0;
}

void OpenGl_Window::screen2Gl(int sx, int sy, float&glx, float&gly)
{
    glx =   windowX0 + ((float)sx/(double)cur_width)*(-windowX0-windowX0);
    gly =   windowY0 + ((float)(cur_height-sy)/(double)cur_height)*(-windowY0-windowY0);
}


void OpenGl_Window::touch_input(STouch& mt, bool screenCoord)
{
    bool doUiLayout = false;
    bool clickHandled = false;
    uint64_t now = cur_time_ms;

    if (screenCoord && !is_fullscreen())
    {
        cerr<<"Warning: Multitouch input only supported in full-screeen for this input (TODO: implement screen coordinate to window coordinate"<<endl;
        return;
    }

    if (callback->supports_touch())
    {
        callback->touch(mt);
    }
    else
    {
        if (mt.fingers.size()==0|| mt.fingers[0].touchId != lastTouchId)
        {
            for(int i=0;  i<(int)ui_apps.size();i++)
            {
                ui_apps[i]->mouse_drag_release(mouseEmulFromSx, mouseEmulFromSy,
                        mouseEmulFromGlx, mouseEmulFromGly, mouseEmulLastSx,
                        mouseEmulLastSy, mouseEmulLastGlx, mouseEmulLastGly,0);
            }
            callback->mouse_drag_release(mouseEmulFromSx, mouseEmulFromSy, mouseEmulFromGlx,
                    mouseEmulFromGly, mouseEmulLastSx, mouseEmulLastSy, mouseEmulLastGlx,
                    mouseEmulLastGly,0);
            lastTouchId = -1;
            if (mt.fingers.size()==0)
                return;
        }

        if (lastTouchId<0)
        {
            mouseEmulFromSx  = mt.fingers[0].x;
            mouseEmulFromSy  = mt.fingers[0].y;
            screen2Gl( mouseEmulFromSx, mouseEmulFromSy, mouseEmulFromGlx, mouseEmulFromGly);
        }

        int sx = mt.fingers[0].x;
        int sy = mt.fingers[0].y;
        float glx,gly;
        screen2Gl(sx,sy,glx,gly);

        if (mt.fingers[0].touchId!=lastTouchId)
        {
            mouseEmulLastSx = sx;
            mouseEmulLastSy = sy;
            mouseEmulLastGlx= glx;
            mouseEmulLastGly= gly;
        }

        for(int i=0;  i<(int)ui_apps.size();i++)
        {
            ui_apps[i]->mouse_drag_move( mouseEmulFromSx, mouseEmulFromSy, mouseEmulFromGlx, mouseEmulFromGly, sx,sy,glx,gly,0, sx-mouseEmulLastSx, sy-mouseEmulLastSy, glx-mouseEmulLastGlx, gly-mouseEmulLastGly);
        }
        callback->mouse_drag_move( mouseEmulFromSx, mouseEmulFromSy, mouseEmulFromGlx, mouseEmulFromGly, sx,sy,glx,gly,0, sx-mouseEmulLastSx, sy-mouseEmulLastSy, glx-mouseEmulLastGlx, gly-mouseEmulLastGly );
        lastTouchId = mt.fingers[0].touchId;

        mouseEmulLastSx = sx;
        mouseEmulLastSy = sy;
        mouseEmulLastGlx= glx;
        mouseEmulLastGly= gly;

        if (now - lastTimeUserInteraction_ms > 500)
        {
            /* We assume a click if same position and we do it once a second */
            cerr<<"Touch: Assuming Click: "<< user_interaction_timer() <<endl;
            for (auto i = ui_apps.begin(); i != ui_apps.end(); i++)
            {
                if ( (*i)->iconBounds.contains(glx, gly) )
                {
                    if (!(*i)->app_visible())
                        hide_apps();
                    (*i)->set_app_visible( !(*i)->app_visible() );
                    doUiLayout = true;
                    clickHandled = true;
                }
            }
            if (doUiLayout)
                layout_ui_plugins();
            /*Normal click */
            for(int i=0; !clickHandled && i<(int)ui_apps.size();i++)
            {
                if (ui_apps[i]->app_visible())
                {
                    clickHandled = ui_apps[i]->mouse_click(sx, sy, glx, gly, 0);
                    clickHandled = true;
                }
            }

            if (!clickHandled && callback)
            {
                callback->mouse_click(sx, sy, glx, gly, 0);
            }
            lastTimeUserInteraction_ms = my_time();;
        }
    }
}


bool OpenGl_Window::layout_app_icons(bool visible, bool animate)
{
    vector<SRef<OpenGl_Ui*> >::iterator i;
    uint64_t now = my_time();
    int iconi=0;
    for (i=ui_apps.begin(); i!=ui_apps.end(); i++)
    {
        if ((*i)->has_icon())
        {
            float iconSize = APP_ICON_SIZE*get_gl_window_height();
            float x1 = windowX0;
            if (!visible)
                x1-=iconSize;
            float x2 = x1 + iconSize;
            float y2 = -windowY0-iconi*iconSize-iconSize*0.5; //place icons directly below previous icon. Leave half an icon of space on top to not draw on top of window decoration
            float y1 = y2 - iconSize;

            int atype = animate?APP_ANIMATION_TYPE:ANIMATE_NONE;

            float prevx1 = (*i)->iconBounds.get_x1(now);
            float prevy1 = (*i)->iconBounds.get_y1(now);
            float prevx2 = (*i)->iconBounds.get_x2(now);
            float prevy2 = (*i)->iconBounds.get_y2(now);
            (*i)->set_icon_bounds(
                    new Animate(APP_ICON_ANIMATION_MS, prevx1, x1, atype, true),
                    new Animate(APP_ICON_ANIMATION_MS, prevy1, y1, atype, true),
                    new Animate(APP_ICON_ANIMATION_MS, prevx2, x2, atype, true),
                    new Animate(APP_ICON_ANIMATION_MS, prevy2, y2, atype, true),
                    new Animate(1.0)
                    );
            iconi++;
        }
    }
}

void OpenGl_Window::hide_apps()
{
    for (auto i = ui_apps.begin();i!=ui_apps.end(); i++)
    {
        if ((*i)->app_visible())
            (*i)->set_app_visible(false);
    }
}

std::vector<SRef<OpenGl_Ui*> > OpenGl_Window::get_ui_apps()
{
    return ui_apps;
}

OpenGl_Ui_Registry::OpenGl_Ui_Registry()
{
}
