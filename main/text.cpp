#include "text.h"

#include <math.h>
#include <valarray>
#include <vector>
#include <iostream>

#include <GL/glu.h>
#include "SDL.h"
#include "SDL_surface.h"

using namespace std;

static int nextpoweroftwo(const int &x)
{
    int result = (x & (x-1)) << 1;
    if(result == 0)
        return x;
    int temp = result & (result - 1);
    while(temp != 0) {
        result = temp;
        temp &= temp - 1;
    }
    return result;
    //    double logbase2 = log(x) / log(2);
    //    return round(pow(2,ceil(logbase2)));
}

static float colorDiff(const int &r1, const int &g1, const int &b1, const int &r2, const int &g2, const int &b2)
{
    float dr = r1-r2;
    float dg = g1-g2;
    float db = b1-b2;

    return sqrt(dr*dr + dg*dg + db*db);
}

static SDL_Surface* SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth)
{
    Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif
    return SDL_CreateRGBSurface(flags, width, height, depth, rmask, gmask, bmask, amask);
}


Text_Texture::Text_Texture(TTF_Font* font, const std::string &t, int size, SDL_Color fgc, SDL_Color bgc)
{
    fgColor = fgc;
    bgColor = bgc;
    text = t;
    fontsize = size;
    SDL_Surface *initial = NULL;
    SDL_Surface *intermediary = NULL;

    my_assert(font);

    initial = TTF_RenderUTF8_Blended(font, text.c_str(), fgc);

    textwidth = initial->w;
    textheight = initial->h;

    int w = nextpoweroftwo(initial->w);
    int h = nextpoweroftwo(initial->h);

    texturewidth = w;
    textureheight = h;
    intermediary = SDL_CreateRGBSurface(0, w, h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);

    my_assert(intermediary);

    for (int i=0; i<w*h; i++) //use FillRect instead?
    {
        ((unsigned char*)intermediary->pixels)[i*4+0]=fgColor.r;
        ((unsigned char*)intermediary->pixels)[i*4+1]=fgColor.g;
        ((unsigned char*)intermediary->pixels)[i*4+2]=fgColor.b;
        ((unsigned char*)intermediary->pixels)[i*4+3]=0;
    }

    uint8_t *dest=(uint8_t*)intermediary->pixels;
    for (int ey=0; ey<initial->h; ey++)
        for (int ex=0; ex<initial->w; ex++)
        {
            dest[ey*w*4 + ex*4 + 3] = (int)((unsigned char*)initial->pixels)[ey*initial->w*4 + ex*4 + 3];
        }

    glGenTextures(1, &texture_handle);
    my_assert(glGetError()==GL_NO_ERROR);
    glBindTexture(GL_TEXTURE_2D, texture_handle);
    my_assert(glGetError()==GL_NO_ERROR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, intermediary->pixels );
    my_assert(glGetError()==GL_NO_ERROR);

    glFinish();
    my_assert(glGetError()==GL_NO_ERROR);
    SDL_FreeSurface(initial);
    SDL_FreeSurface(intermediary);
}

Text_Texture::~Text_Texture()
{
    glDeleteTextures(1, (GLuint*)&texture_handle);
    my_assert(glGetError()==GL_NO_ERROR);
}

GLuint Text_Texture::get_texture()
{
    return texture_handle;
}

Text::Text(std::string fp)
{
    fontPath = fp;
}

Text::~Text()
{
}

static bool ceq(SDL_Color c1, SDL_Color c2)
{
    return c1.r==c2.r && c1.g==c2.g && c1.b==c2.b;
}

SRef<Text_Texture*> Text::get_texture_object(std::string text, int size, SDL_Color fgcolor, SDL_Color bgcolor)
{
    int x=0;
    std::list<SRef<Text_Texture*> >::iterator i;
    if (textures.size()>100)
        textures.clear();
    for (i=textures.begin(); i!=textures.end(); i++)
    {
        if ( (*i)->get_text()==text && (*i)->get_size()==size &&
             ceq( (*i)->get_fg_color(), fgcolor) && ceq( (*i)->get_bg_color(), bgcolor)  )
        {
            return *i;
        }
        x++;
    }
    get_font(size);

    SRef<Text_Texture*> t = new Text_Texture(get_font(size), text, size, fgcolor,bgcolor);
    textures.push_back(t);
    return t;
}

SRef<Text_Texture*> Text::get_texture_object(std::string text, int size, SDL_Color fgcolor, SDL_Color bgcolor,int style)
{
    int x=0;
    if (textures.size()>100)
        textures.clear();
    std::list<SRef<Text_Texture*> >::iterator i;
    for (i=textures.begin(); i!=textures.end(); i++){

        if ( (*i)->get_text()==text && (*i)->get_size()==size &&
             ceq( (*i)->get_fg_color(), fgcolor) && ceq( (*i)->get_bg_color(), bgcolor)  )
        {
            return *i;
        }
        x++;
    }
    //cout<<x<<endl;

    SRef<Text_Texture*> t = new Text_Texture(get_font(size,style), text, size, fgcolor,bgcolor);
    textures.push_back(t);
    return t;

}
int Text::get_texture(std::string text, int size, SDL_Color fgcolor, SDL_Color bgcolor)
{
    return get_texture_object(text,size, fgcolor, bgcolor)->get_texture();
}

void glEnable2D()
{
    int vPort[4];

    glGetIntegerv(GL_VIEWPORT, vPort);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(0, vPort[2], 0, vPort[3], -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    my_assert(glGetError()==GL_NO_ERROR);
}

void glDisable2D()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


static void getScreenCoord(GLdouble x, GLdouble y, GLdouble z, GLdouble* winx, GLdouble* winy, GLdouble* winz)
{

    GLdouble model[16];
    GLdouble proj[16];
    GLint view[4];

    glGetDoublev(GL_PROJECTION_MATRIX, &proj[0]);
    glGetDoublev(GL_MODELVIEW_MATRIX, &model[0]);
    glGetIntegerv(GL_VIEWPORT, &view[0]);

    GLdouble tmpz;

    gluProject(x,y,z, model, proj, view, winx,winy,&tmpz);
    my_assert(glGetError()==GL_NO_ERROR);
    if (winz)
        *winz=tmpz;
}


void Text::draw2D( int x, int y, std::string text, int size, SDL_Color fgc, SDL_Color bgc,int style )
{
    if (text.size()<=0)
        return;
    SRef<Text_Texture*> t =  get_texture_object(text, size, fgc, bgc,style);
    glEnable2D();
    my_assert(glGetError()==GL_NO_ERROR);
    glDisable(GL_DEPTH_TEST);

    //glBlendFunc(GL_ONE, GL_ONE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    glEnable(GL_TEXTURE_2D);

    int tex = t->get_texture();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex );

    glBindTexture(GL_TEXTURE_2D, tex );
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(x ,y);

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(x + t->get_texture_width(), y);

    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(x + t->get_texture_width(), y + t->get_texture_height());

    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(x, y + t->get_texture_height() );
    glEnd();

    glDisable2D();
}


void Text::draw2D( int x, int y, std::string text, int size, SDL_Color fgc, SDL_Color bgc )
{
    if (text.size()<=0)
        return;
    SRef<Text_Texture*> t =  get_texture_object(text, size, fgc, bgc);
    glEnable2D();
    my_assert(glGetError()==GL_NO_ERROR);
    glDisable(GL_DEPTH_TEST);
    //cout<<":  "<<text<<endl;
    //glBlendFunc(GL_ONE, GL_ONE);


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    glEnable(GL_TEXTURE_2D);

    int tex = t->get_texture();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex );

    glBindTexture(GL_TEXTURE_2D, tex );
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(x ,y);

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(x + t->get_texture_width(), y);

    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(x + t->get_texture_width(), y + t->get_texture_height());

    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(x, y + t->get_texture_height() );
    glEnd();

    glDisable2D();
}


void Text::draw3D( float x, float y, float z, float scale, std::string text, int size, SDL_Color fgc, SDL_Color bgc, Screen_Location* where )
{
    if (text.size()<=0)
        return;
    SRef<Text_Texture*> t =  get_texture_object(text, size, fgc, bgc);

    //glBlendFunc(GL_ONE, GL_ONE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    my_assert(glGetError()==GL_NO_ERROR);

    glEnable(GL_BLEND);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glEnable(GL_TEXTURE_2D);

    int tex = t->get_texture();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex );
    my_assert(glGetError()==GL_NO_ERROR);

    glBindTexture(GL_TEXTURE_2D, tex );
    my_assert(glGetError()==GL_NO_ERROR);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f);

    glVertex2f(x*scale ,y*scale);

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f((x + t->get_texture_width())*scale, y*scale);


    glTexCoord2f(1.0f, 0.0f);
    glVertex2f((x + t->get_texture_width())*scale, (y + t->get_texture_height())*scale );


    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(x*scale, (y + t->get_texture_height())*scale );

    glEnd();
    glDisable(GL_BLEND);
    my_assert(glGetError()==GL_NO_ERROR);


    GLdouble X,Y;

    getScreenCoord(x*scale, -1.0*y*scale + ((float)t->get_text_height())*scale/16, 0.0 , &X, &Y, NULL);
    if (where)
    {
        where->x1=X;
        where->y1=Y;
    }
    getScreenCoord((x + t->get_text_width())*scale, -1.0*y*scale + t->get_text_height()*scale/16 , 0.0, &X, &Y, NULL);
    if (where)
    {
        where->x2=X;
        where->y2=Y;
    }

    getScreenCoord( (x + t->get_text_width())*scale, -1.0*(y + t->get_text_height())*scale , 0, &X, &Y, NULL);
    if (where)
    {
        where->x3=X;
        where->y3=Y;
    }

    getScreenCoord( x*scale, -1.0*(y + t->get_text_height())*scale , 0.0, &X, &Y, NULL);
    if (where)
    {
        where->x4=X;
        where->y4=Y;
    }
}


static void rectToCoord(float &x1, float&y1, float &x2, float &y2, float lx1, float ly1, float lx2, float ly2, float aratio, int halign)
{

    float l_aratio=(lx2-lx1)/(ly2-ly1);

    float xalign=0;

    if (aratio > l_aratio) { // fill full width, center vertically
        float h=(lx2-lx1)/aratio;
        float lh=ly2-ly1;
        x1=lx1;
        y1=ly1+((lh-h)/2.0);
        x2=lx2;
        y2=y1+h;
        float xalign=0;
    }else{  //fill full height

        y1=ly1;
        float w=(ly2-ly1)*aratio;
        float lw=lx2-lx1;

        x1=lx1+(lw-w)/2.0;
        y2=ly2;
        x2=x1+w;
        switch(halign){
        case TEXT_ALIGN_LEFT:
            xalign=-( (lx2-lx1)-(x2-x1) )/2.0;
            break;
        case TEXT_ALIGN_RIGHT:
            xalign=( (lx2-lx1)-(x2-x1) )/2.0;
        case TEXT_ALIGN_CENTER:
            break;
        }
    }

    x1=x1+xalign;
    x2=x2+xalign;
}


void Text::draw3D( float x1, float y1, float z1, float x2, float y2, float z2, std::string text, int size, SDL_Color fgc, SDL_Color bgc, int align )
{
    if (text.size()<=0)
        return;
    SRef<Text_Texture*> t =  get_texture_object(text, size, fgc, bgc);

    glEnable(GL_TEXTURE_2D);
    //glBlendFunc(GL_ONE, GL_ONE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    my_assert(glGetError()==GL_NO_ERROR);

    glEnable(GL_BLEND);
    my_assert(glGetError()==GL_NO_ERROR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int tex = t->get_texture();
    my_assert(glGetError()==GL_NO_ERROR);
    rectToCoord(x1,y1,x2,y2,x1,y1,x2,y2, (float)t->textwidth/(float)t->textheight, align);


    glBindTexture(GL_TEXTURE_2D, tex );
    my_assert(glGetError()==GL_NO_ERROR);
    glBegin(GL_QUADS);

    glTexCoord2f( 0.0f, (float)t->textheight/(float)t->textureheight);
    glVertex3f( x1 ,y1, z1);


    glTexCoord2f( (float)t->textwidth/(float)t->texturewidth ,  (float)t->textheight/(float)t->textureheight);
    glVertex3f( x2, y1, z1);


    glTexCoord2f( (float)t->textwidth/(float)t->texturewidth, 0.0f);
    glVertex3f( x2, y2, z1 );


    glTexCoord2f( 0.0f, 0.0f);
    glVertex3f( x1, y2, z1 );

    glEnd();

    my_assert(glGetError()==GL_NO_ERROR);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}


int Text::get_texture_width(std::string text, int size, SDL_Color fgc, SDL_Color bgc )
{
    SRef<Text_Texture*> t =  get_texture_object(text, size, fgc, bgc);
    return t->get_texture_width();
}

int Text::get_texture_height(std::string text, int size, SDL_Color fgc, SDL_Color bgc )
{
    SRef<Text_Texture*> t =  get_texture_object(text, size, fgc, bgc);
    return t->get_texture_height();
}

int Text::get_text_width(std::string text, int size, SDL_Color fgc, SDL_Color bgc )
{
    if(text.size()==0)return 0;
    SRef<Text_Texture*> t =  get_texture_object(text, size, fgc, bgc);
    return t->get_text_width();
}

int Text::get_text_height(std::string text, int size, SDL_Color fgc, SDL_Color bgc )
{
    SRef<Text_Texture*> t =  get_texture_object(text, size, fgc, bgc);
    return t->get_text_height();
}

void Text::restart_gl()
{
    textures.clear();
    //        list<SRef<MFont*> >::iterator i;
    //        for (i=fonts.begin(); i!=fonts.end(); i++)
    //        {
    //            TTF_CloseFont((*i)->getFont());
    //        }
    fonts.clear();
}

TTF_Font* Text::get_font(int size,int style)
{
    list<SRef<SFont*> >::iterator i;
    for (i=fonts.begin(); i!=fonts.end(); i++)
        if ( (*i)->get_point_size()==size)
            return (*i)->get_font();


    TTF_Font* font;

    if(!(font = TTF_OpenFont(fontPath.c_str(), size)))
    {
        printf("Error loading font: %s", TTF_GetError());
        exit(1);
    }

    fonts.push_back(new SFont(font, size, style));
    return font;
}


bool Screen_Location::contains(float x, float y)
{
    if (x>=x1 && x<=x2 && y>=y3 && y<=y1)    // FIXME: assumes 2D rectangle
        return true;
    return false;
}

void Text_Fragment::create_text_surface(int &returnWidth, int &returnHeight)
{
    if(text_surface)
        SDL_FreeSurface(text_surface);
    if(text.empty())
        text_surface = NULL;
    else
    {
        SDL_Color sdlColor;
        sdlColor.r = color.r;
        sdlColor.g = color.g;
        sdlColor.b = color.b;
        text_surface = TTF_RenderUTF8_Blended(font->get_font(), text.c_str(), sdlColor);
    }
    if(text_surface)
    {
        returnWidth = text_surface->w;
        returnHeight = text_surface->h;
    }
    else
    {
        returnWidth = returnHeight = 0;
    }
}

const SDL_Surface* Text_Fragment::blit_onto_surface(SDL_Surface* surface, const int &offsetX, const int &offsetY)
{
    if(text_surface)
    {
        SDL_Rect destRect;
        destRect.x = offsetX;
        destRect.y = offsetY;
        destRect.w = text_surface->w;
        destRect.h = text_surface->h;
        if(SDL_BlitSurface(text_surface, NULL, surface, &destRect) != 0)
            std::cerr << "SDL_BlitSurface error" << std::endl;
    }
    return text_surface;
}

void Text_Line::create_text_surfaces(int &returnWidth, int &returnHeight)
{
    returnWidth=0;
    returnHeight=0;
    for(std::list<Text_Fragment>::iterator it=text_line.begin(), end=text_line.end(); it!=end; ++it)
    {
        int width, height;
        it->create_text_surface(width, height);
        returnWidth += width;
        returnHeight = std::max(returnHeight, height);
    }
}

int Text_Line::blit_onto_surface(SDL_Surface* surface, const int &offsetX, const int &offsetY)
{
    int currentOffsetX = offsetX, maxHeight=0;
    for(std::list<Text_Fragment>::iterator it=text_line.begin(), end=text_line.end(); it!=end; ++it)
    {
        const SDL_Surface* textFragmentSurface = it->blit_onto_surface(surface, currentOffsetX, offsetY);
        if(textFragmentSurface)
        {
            currentOffsetX += textFragmentSurface->w;
            maxHeight = std::max(textFragmentSurface->h, maxHeight);
        }
    }
    return maxHeight;
}


Text_Plane::Text_Plane(const int &_padding, const RGBA &_bgColor)
    : padding(_padding), bg_color(_bgColor), plane(NULL)
{
}

Text_Plane::~Text_Plane()
{
    if(plane)
        SDL_FreeSurface(plane);
}


void Text_Plane::draw()
{
    if(plane)
        SDL_FreeSurface(plane);
    int maxWidth=0, overallHeight=0;
    for(std::list<SRef<Text_Line *> >::iterator it=text.begin(), end=text.end(); it!=end; ++it)
    {
        int width, height;
        (*it)->create_text_surfaces(width, height);
        maxWidth = std::max(width, maxWidth);
        overallHeight += height;
    }
    plane = SDL_CreateRGBSurface(SDL_SWSURFACE, maxWidth + 2*padding, overallHeight + 2*padding, 32);
    if(plane)
    {
        SDL_FillRect(plane, NULL, SDL_MapRGBA(plane->format, bg_color.r, bg_color.g, bg_color.b, bg_color.a));
        int heightOffset = padding;
        for(std::list<SRef<Text_Line *> >::iterator it=text.begin(), end=text.end(); it!=end; ++it)
        {
            heightOffset += (*it)->blit_onto_surface(plane, padding, heightOffset);
        }
    }
}

const SDL_Surface* Text_Plane::get_surface()
{
    if(plane == NULL)
        draw();
    return plane;
}

void Text_Plane::add_text_line(const SRef<Text_Line *> &textLine)
{
    text.push_back(textLine);
}

void Text_Plane::clear()
{
    text.clear();
    if(plane)
    {
        SDL_FreeSurface(plane);
        plane = NULL;
    }
}
