#include "opengl_display.h"
#include "opengl_window.h"

#include "my_time.h"
using namespace std;

//Purpose: Returns the largest centered sub-rectangle with a certain aspect ratio within another rectangle.
void rect_to_coord(float &x1, float&y1, float &x2, float &y2, float lx1, float ly1, float lx2, float ly2, float aratio)
{
    float l_aratio=(lx2-lx1)/(ly2-ly1);

    if (aratio > l_aratio) // fill full width, center vertically
    {
        float h = (lx2-lx1) / aratio;
        float lh= ly2 - ly1;
        x1 = lx1;
        y1 = ly1 + ((lh-h)/2.0);
        x2 = lx2;
        y2 = y1 + h;
    }
    else //fill full height
    {
        y1 = ly1;
        float w = (ly2 - ly1) * aratio;
        float lw= lx2 - lx1;
        x1 = lx1 + (lw - w) / 2.0;
        y2 = ly2;
        x2 = x1 + w;
    }
}

OpenGl_Display::OpenGl_Display(std::string _callId, uint32_t _ssrc, bool _fullscreen)
    : Video_Display(_callId, _ssrc), width(0), height(0), corner_radius_rercentage(0.005f)
{
    is_local_video = false;
    hidden = false;
    this->width = width;
    this->height = height;
    fullscreen = _fullscreen;
    ///	nallocated=0;
    //isLocalVideo=false;
    time_last_receive = 0;

    gfx= new struct mgl_gfx;

    memset(gfx, 0, sizeof(struct mgl_gfx));
    gfx->texture=-1;
    gfx->aratio=16/9;
    gfx->wu=1;
    gfx->hu=1;
    gfx->tex_dim=0;
    gfx->is_selected=false;
    gfx->video_name;

    rgba = NULL;
    need_upload = false;
    new_rgb_data = false;
    color_nbytes = 3;

    window = * (OpenGl_Window::get_window(_fullscreen) );
    my_assert(window);
}

void OpenGl_Display::init(const uint32_t &width, const uint32_t &height)
{
    this->width = width;
    this->height= height;
}

void OpenGl_Display::handle( const SRef<SImage *>& mimage )
{
    my_assert(mimage->linesize[1]==0);
    color_nbytes = mimage->linesize[0]/mimage->width;
    data_lock.lock();
    if (!rgba || width!=mimage->width || height!=mimage->height)
    {
        if (rgba)
            delete[] rgba;
        width = mimage->width;
        height= mimage->height;
        gfx->aratio = (float)width/(float)height;
        rgba = new uint8_t[width*height*color_nbytes + 16]; // +16 to avoid mesa bug
    }

    my_assert(rgba);
    memcpy(rgba, &mimage->data[0][0], width*height*color_nbytes); //TODO: don't copy since it is in correct format.
    new_rgb_data = true;
    time_last_receive = my_time();
    data_lock.unlock();
}

void OpenGl_Display::start()
{
    my_assert(window);
    window->start();
    window->add_display(this);
}

void OpenGl_Display::stop()
{
    window->remove_display(this);
    window->stop();
    gfx->texture = -1;
}

struct mgl_gfx* OpenGl_Display::get_texture(bool textureUpdate)
{
    data_lock.lock();
    if (textureUpdate && gfx->texture == -1)
    {
        if(glGetError()!=GL_NO_ERROR)
        {
            data_lock.unlock();
            return gfx;
        }
        my_assert(color_nbytes==3 || color_nbytes==4);
        int hw_max_dim = window->get_texture_max_size();
        int dim = (hw_max_dim>2048) ? 2048 : hw_max_dim;
        gfx->tex_dim = dim;

        glGenTextures( 1, (GLuint*)&(gfx->texture));
        glBindTexture( GL_TEXTURE_2D, gfx->texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        unsigned char* blackTextureDataSource = new unsigned char[2048*2048*4];
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, dim, dim, 0, GL_BGRA, GL_UNSIGNED_BYTE, blackTextureDataSource); // according to http://www.opengl.org/wiki/Common_Mistakes , this is the best way to do it
        delete[] blackTextureDataSource;

        if(glGetError()!=GL_NO_ERROR)
        {
            data_lock.unlock();
            return gfx;
        }
    }

    if (textureUpdate && new_rgb_data)
    {
        my_assert(gfx->texture > 0);
        new_rgb_data = false;
        glBindTexture( GL_TEXTURE_2D, gfx->texture);

        if (width > gfx->tex_dim)
        {
            int factor = 2;
            while (width/factor > gfx->tex_dim)
                factor++;
            cerr << "WARNING: insufficent OpenGl hardware. Resizing image to fit in texture (factor="<<factor<<")"<<endl;
            uint8_t* tmpbuf = new uint8_t[(width/factor)*(height/factor)*color_nbytes+16];

            int x,y;
            int newwidth = width / factor;
            int newheight = height / factor;
            for (y = 0; y < newheight; y++)
            {
                for (x = 0; x < newwidth; x++)
                {
                    my_assert("This will definitely not work. Needs correcting and making for additional alpha channel."==0);
                    tmpbuf[x*color_nbytes+y*newwidth*color_nbytes+0]=rgba[(x*color_nbytes+y*width*color_nbytes+0)*factor];
                    tmpbuf[x*color_nbytes+y*newwidth*color_nbytes+1]=rgba[(x*color_nbytes+y*width*color_nbytes+2)*factor];
                    tmpbuf[x*color_nbytes+y*newwidth*color_nbytes+2]=rgba[(x*color_nbytes+y*width*color_nbytes+1)*factor];
                }
            }

            glTexSubImage2D( GL_TEXTURE_2D, 0, 0,0 , newwidth, newheight, GL_RGB, GL_UNSIGNED_BYTE, tmpbuf );
            gfx->wu = (newwidth/*/(float)factor*/) / (float)gfx->tex_dim;
            gfx->hu = (newheight/*/(float)factor*/)/ (float)gfx->tex_dim;
            gfx->aratio = (float)width / (float)height;
            delete []tmpbuf;
        }
        else
        {
            if(glGetError()!=GL_NO_ERROR)
            {
                data_lock.unlock();
                return gfx;
            }
            GLenum pixFormat = (color_nbytes==3)?GL_RGB:GL_BGRA;

            glTexSubImage2D( GL_TEXTURE_2D, 0, 0,0 , width, height, pixFormat, GL_UNSIGNED_BYTE, rgba);
            if(glGetError()!=GL_NO_ERROR)
            {
                data_lock.unlock();
                return gfx;
            }
            gfx->wu = width / (float)gfx->tex_dim;
            gfx->hu = height / (float)gfx->tex_dim;
            gfx->aratio = (float)width / (float)height;
        }
    }
    data_lock.unlock();
    return gfx;
}

void OpenGl_Display::set_is_local_video(bool isLocal)
{
    is_local_video = isLocal;
}

bool OpenGl_Display::get_is_local_video()
{
    return is_local_video;
}

void OpenGl_Display::set_video_name(std::string id)
{
    my_assert(gfx);
    gfx->video_name = strdup(id.c_str());
}

void OpenGl_Display::set_call_id(std::string id)
{
    my_assert(gfx);
    gfx->call_id = strdup(id.c_str());
}

float OpenGl_Display::get_pix_gl_ratio_width()
{
    float ratioWidth;

    ratioWidth = ((float) window->get_window_width() / (float) window->get_gl_window_width()) ;

    //cout << "get_pix_gl_ratio_width W h: " <<window->get_window_width() <<" GL h: " <<window->get_gl_window_width()<<"ratio: "<<ratioWidth<<endl;
    return ratioWidth;
}

float OpenGl_Display::get_pix_gl_ratio_height()
{
    float ratioHeight;

    ratioHeight = ((float) window->get_window_height() / (float) window->get_gl_window_height());
    //  cout << "getPixGlRatioHeight W h: " <<window->get_window_height() <<" GL h: " <<window->get_gl_window_height()<<"ratio: "<<ratioHeight<<endl;
    return ratioHeight;
}

Rect OpenGl_Display::get_video_area(uint64_t time)
{
    return bounds.get_rect_aspect_ratio(gfx->aratio, time, 0, 0);
}

void OpenGl_Display::draw(uint64_t cur_time, bool softedge)
{
    if ( !gfx || gfx->texture <= 0)
        return;

    float lx1=bounds.get_x1(cur_time);
    float ly1=bounds.get_y1(cur_time);
    float lx2=bounds.get_x2(cur_time);
    float ly2=bounds.get_y2(cur_time);

    float sx1,sy1,sx2,sy2;
    float aratio = gfx->wu / gfx->hu;
    rect_to_coord(sx1,sy1,sx2,sy2, lx1,ly1,lx2,ly2, aratio);

    float a = 1.0;
    int cornerNSteps = 8;
    if (alpha)
    {
        a = alpha->get_val(cur_time);
        if (a<0.0001)
            return;
    }

    window->draw_texture(get_texture(true),sx1,sy1,sx2,sy2,0, a, corner_radius_rercentage, 8, softedge);
}
