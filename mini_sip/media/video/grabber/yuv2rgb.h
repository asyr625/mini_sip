#ifndef YUV2RGB_H
#define YUV2RGB_H


void yuv2rgb(const unsigned char* y,
                  int linesizey,
                  const unsigned char* u,
                  int linesizeu,
                  const unsigned char* v,
                  int linesizev,
                  int width,
                  int height,
                  unsigned char *rgb);

#endif // YUV2RGB_H
