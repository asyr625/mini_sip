#ifndef MATROX_GRABBER_H
#define MATROX_GRABBER_H

#include <stdint.h>
#include "mutex.h"
#include "grabber.h"
#include "my_semaphore.h"

#include<mil.h>

#define NB_GRAB_MAX 3

typedef struct {
    MIL_ID        *MilImage;
    MIL_ID        MilDigitizer;
    MIL_ID        GrabEndEvent;
    int 	      stopped;
    long          NbFrames;
    long          NbGrabStart;
    MIL_ID   MilApplication;
    MIL_ID   MilSystem;
} UserDataStruct;

class Matrox_Grabber : public Grabber
{
public:
    Matrox_Grabber( SRef<Video_Media*>, std::string deviceId );

    void open();
    void get_capabilities();
    bool set_image_chroma( uint32_t chroma );


    void read();
    virtual void run();
    virtual void stop();
    virtual void start();

    virtual void close();

    uint32_t get_height(){ return height; }
    uint32_t get_width(){ return width; }

    virtual void set_local_display(SRef<VideoDisplay*>);

private:
    void init();

    std::string device;
    MIL_ID   MilImage[NB_GRAB_MAX];
    MIL_TEXT_CHAR FrameIndex[10];
    UserDataStruct UserStruct;


    uint32_t height;
    uint32_t width;

    Mutex grabberLock;
    SRef<Video_Display*> localDisplay;

    bool initialized;
    SRef<Thread*> runthread;
    SRef<Semaphore*> startBlockSem;
    SRef<Semaphore*> initBlockSem;
};

class Matrox_Plugin : public Grabber_Plugin
{
public:
    Matrox_Plugin( SRef<Library *> lib ) : Grabber_Plugin( lib ){}

    virtual SRef<Grabber *> create( SRef<Video_Media*>, const std::string &device) const;

    virtual std::string get_mem_object_type() const { return "MatroxPlugin"; }

    virtual std::string get_name() const { return "matrox"; }

    virtual uint32_t get_version() const { return 0x00000001; }

    virtual std::string get_description() const { return "Matrox grabber"; }
};

#endif // MATROX_GRABBER_H
