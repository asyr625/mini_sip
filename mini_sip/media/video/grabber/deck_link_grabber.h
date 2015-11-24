#ifndef DECK_LINK_GRABBER_H
#define DECK_LINK_GRABBER_H

#include "mutex.h"
#include "my_semaphore.h"
#include "grabber.h"
#include"decklinksdk/DeckLinkAPI.h"

class DeckLinkCaptureDelegate : public IDeckLinkInputCallback
{
public:
    DeckLinkCaptureDelegate();

    virtual HRESULT STDMETHODCALLTYPE query_interface(REFIID iid, LPVOID *ppv) { return E_NOINTERFACE; }
    virtual ULONG STDMETHODCALLTYPE add_ref(void) { return 1; }
    virtual ULONG STDMETHODCALLTYPE  release(void) { return 1; }
    virtual HRESULT STDMETHODCALLTYPE video_input_format_changed(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);
    virtual HRESULT STDMETHODCALLTYPE video_input_frame_arrived(IDeckLinkVideoInputFrame*, IDeckLinkAudioInputPacket*);
    SRef<SImage*> get_image();
    int get_width() {return width;}
    int get_height() {return height;}
    void stop() {do_stop=true;}
    bool do_stop;
    bool get_no_source_flag() { return no_source_flag;}
private:
    void allocate_image();
    long unsigned int frame_count;

    void put_image(IDeckLinkVideoInputFrame* videoFrame);

    Mutex buffer_lock;
    Semaphore buffer_sem;
    bool filled;
    int width;
    int height;
    SRef<SImage *> frame;
    bool no_source_flag;
};

class Threaded_Frame_Converter;

class Deck_Link_Grabber : public Grabber
{
public:
    Deck_Link_Grabber(SRef<Video_Media*> _media, std::string dev );
    virtual ~Deck_Link_Grabber();

    void open();
    void get_capabilities();
    bool set_image_chroma( uint32_t chroma );


    void read();
    virtual void run();
    virtual void stop();
    virtual void start();

    virtual void close();

    uint32_t get_height();
    uint32_t get_width();

    virtual void set_local_display(SRef<Video_Display*>);

private:
    void init();

    bool do_stop;

    DeckLinkCaptureDelegate* capture;

    IDeckLinkInput                  *deck_link_input;
    IDeckLinkIterator               *deck_link_iterator;
    IDeckLink                       *deck_link;
    IDeckLinkDisplayModeIterator    *display_mode_iterator;

    std::string device;

    Mutex grabber_lock;

    bool initialized;
    SRef<Thread*> runthread;
    SRef<Semaphore*> start_block_sem;
    SRef<Semaphore*> init_block_sem;
    int deviceno;
    SRef<Video_Display*> local_display;
};

class Deck_Link_Plugin : public Grabber_Plugin
{
public:
    Deck_Link_Plugin( SRef<Library *> lib ) : Grabber_Plugin( lib ){}

    virtual SRef<Grabber *> create( SRef<Video_Media*>, const std::string &device) const;

    virtual std::string get_mem_object_type() const { return "DeckLinkPlugin"; }

    virtual std::string get_name() const { return "decklink"; }

    virtual uint32_t get_version() const { return 0x00000001; }

    virtual std::string get_description() const { return "DeckLink grabber"; }
};

#endif // DECK_LINK_GRABBER_H
