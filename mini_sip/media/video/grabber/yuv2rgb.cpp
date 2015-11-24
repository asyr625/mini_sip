#include "yuv2rgb.h"

#include "my_assert.h"

void yuv2rgb(const unsigned char* y,
             int linesizey,
             const unsigned char* u,
             int linesizeu,
             const unsigned char* v,
             int linesizev,
             int width,
             int height,
             unsigned char *rgb)
{
    my_assert(y);
    my_assert(u);
    my_assert(v);
    my_assert(rgb);

    static const int precision=32768;

    static const int coefficientY  = (int)(1.164*precision+0.5);
    static const int coefficientRV = (int)(1.596*precision+0.5);
    static const int coefficientGU = (int)(0.391*precision+0.5);
    static const int coefficientGV = (int)(0.813*precision+0.5);
    static const int coefficientBU = (int)(2.018*precision+0.5);

    for (int h=0; h<height; h++)
    {
        for (int w=0; w<width; w++)
        {
            int k = h * linesizey + w;
            int i = (h/2) * (linesizeu) + (w/2);
            int Y = y[k];
            int U = u[i];
            int V = v[i];

            if (!(i&0x0F))
            {
                __builtin_prefetch (&y[k+32], 0); // prefetch for read
                __builtin_prefetch (&u[i+32], 0); // prefetch for read
                __builtin_prefetch (&v[i+32], 0); // prefetch for read
            }

            int R = coefficientY*(Y-16)+coefficientRV*(V-128);
            int G = coefficientY*(Y-16)-coefficientGU*(U-128)-coefficientGV*(V-128);
            int B = coefficientY*(Y-16)+coefficientBU*(U-128);

            R = (R+precision/2)/precision;
            G = (G+precision/2)/precision;
            B = (B+precision/2)/precision;
            if (R<0) R=0;
            if (G<0) G=0;
            if (B<0) B=0;
            if (R>255) R=255;
            if (G>255) G=255;
            if (B>255) B=255;
            rgb[(h*width+w)*3+0]=R;
            rgb[(h*width+w)*3+1]=G;
            rgb[(h*width+w)*3+2]=B;
        }
    }
}
