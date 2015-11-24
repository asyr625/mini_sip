#ifndef TEXT_H
#define TEXT_H

#include "sobject.h"

#ifdef __APPLE__
#define DARWIN 1
#endif

#ifdef _MSC_VER
#include<Winsock2.h>
#include<windows.h>
#include"../../SDL-1.2.13/include/SDL.h"
#include"../../SDL-1.2.13/include/SDL_opengl.h"
#warn include SDL_ttf header here

#else
#include "SDL.h"
#include "SDL_opengl.h"
#ifdef __APPLE__
#include "SDL_ttf.h"
#else
#include "SDL_ttf.h"
#endif
#endif

#include<list>

#define TEXT_ALIGN_LEFT 1
#define TEXT_ALIGN_CENTER 2
#define TEXT_ALIGN_RIGHT 3

class Text_Texture : public SObject
{
public:
    Text_Texture(TTF_Font* font, const std::string &t, int size, SDL_Color fgc, SDL_Color bgc);
    ~Text_Texture();

    GLuint get_texture();//{return texture_handle;}
    std::string get_text(){return text;}
    int get_size(){return fontsize;}

    int get_text_width(){return textwidth;}
    int get_text_height(){return textheight;}

    int get_texture_width(){return texturewidth;}
    int get_texture_height(){return textureheight;}

    SDL_Color get_fg_color(){return fgColor;}
    SDL_Color get_bg_color(){return bgColor;}

    int textwidth;
    int textheight;

    int texturewidth;
    int textureheight;


private:
    std::string text;
    int fontsize;
    SDL_Color bgColor;
    SDL_Color fgColor;
    GLuint texture_handle;
};

class SFont : public SObject
{
public:
    SFont(TTF_Font* f, int pointsize, const int &extraStyle/*=TTF_STYLE_NORMAL*/)
        : font(f)
    {
        TTF_SetFontStyle(font, extraStyle);
        // TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
        //size = TTF_FontHeight(font);
        size=pointsize;
    }
    SFont(const int &_size,
          const std::string &path="/usr/share/fonts/truetype/freefont/FreeSans.ttf"
            //#ifdef LINUX // FIXME: add default fonts for other platforms
            //		       ="/usr/share/fonts/truetype/freefont/FreeSans.ttf"
            //#elif WIN32
            //		      ="%WINDOWS%/Fonts/micross.ttf"
            //#endif
            , const int &extraStyle=TTF_STYLE_NORMAL)
        : font(TTF_OpenFont(path.c_str(), _size)), size(_size)
    {
        if(extraStyle != TTF_STYLE_NORMAL && font)
            TTF_SetFontStyle(font, extraStyle);
    }
    ~SFont(){ TTF_CloseFont(font); }
    TTF_Font* get_font() { return font; }
    int get_point_size() { return size; }

private:
    TTF_Font *font;
    int size;
};


class Screen_Location
{
public:
    float x1,y1, x2,y2, x3,y3, x4,y4;
    bool contains(float x, float y);
    std::string cmd;
};

class Text
{
public:
    Text(std::string fp);
    ~Text();

    int get_texture(std::string text, int size, SDL_Color fgcolor, SDL_Color bgcolor);
    SRef<Text_Texture*> get_texture_object(std::string text, int size, SDL_Color fgcolor, SDL_Color bgcolor);
    SRef<Text_Texture*> get_texture_object(std::string text, int size, SDL_Color fgcolor, SDL_Color bgcolor,int style);

    void draw2D(int x, int y, std::string text, int size, SDL_Color fgc, SDL_Color bgc);
    void draw2D(int x, int y, std::string text, int size, SDL_Color fgc, SDL_Color bgc,int style);
    void draw3D(float x, float y, float z, float scale, std::string text, int size, SDL_Color fgc, SDL_Color bgc, Screen_Location *where=NULL);
    void draw3D(float x1, float y1, float z1, float x2, float y2, float z2, std::string text, int size, SDL_Color fgc, SDL_Color bgc, int align);
    int get_texture_width(std::string text, int size, SDL_Color fgc, SDL_Color bgc);
    int get_texture_height(std::string text, int size,  SDL_Color fgc, SDL_Color bgc);

    //The actual text within a texture is smaller than the
    //texture itself (that must be a power of two in dimensions).
    int get_text_width(std::string text, int size, SDL_Color fgc, SDL_Color bgc);
    int get_text_height(std::string text, int size,  SDL_Color fgc, SDL_Color bgc);

    void restart_gl();

private:
    TTF_Font* get_font(int size,int style=TTF_STYLE_NORMAL);
    std::string fontPath;

    std::list<SRef<SFont*> > fonts;
    std::list<SRef<Text_Texture*> > textures;
};


struct RGB
{
    RGB(const unsigned char &_r=255, const unsigned char &_g=0, const unsigned char &_b=255)
        : r(_r), g(_g), b(_b)
    {
    }
    unsigned char r, g, b;
};

struct RGBA : public RGB
{
    RGBA(const unsigned char &_r=255, const unsigned char &_g=0, const unsigned char &_b=255, const unsigned char &_a=255)
        : RGB(_r, _g, _b), a(_a)
    {
    }
    unsigned char a;
};

struct Text_Fragment
{
    Text_Fragment() : text_surface(NULL) {}
    ~Text_Fragment()
    {
        if(text_surface)
            SDL_FreeSurface(text_surface);
    }
    void create_text_surface(int &returnWidth, int &returnHeight);
    const SDL_Surface* blit_onto_surface(SDL_Surface* surface, const int &offsetX, const int &offsetY);

    std::string text;
    SRef<SFont *> font;
    RGB color;
    SDL_Surface *text_surface;
};

struct Text_Line : public SObject
{
    void create_text_surfaces(int &returnWidth, int &returnHeight);
    int blit_onto_surface(SDL_Surface* surface, const int &offsetX, const int &offsetY);

    std::list<Text_Fragment> text_line;
};

class Text_Plane
{
public:
    Text_Plane(const int &padding, const RGBA &_bgColor);
    virtual ~Text_Plane();
    virtual void draw();
    virtual const SDL_Surface *get_surface();
    virtual void add_text_line(const SRef<Text_Line *> &textLine);
    virtual void clear();

protected:
    SDL_Surface *plane;
    int padding;
    RGBA bg_color;
    std::list<SRef<Text_Line *> > text;
};

#endif // TEXT_H
