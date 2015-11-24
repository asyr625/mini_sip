#ifndef VIDEO_STREAM_PLAYER_H
#define VIDEO_STREAM_PLAYER_H
#include <deque>

#include "stream_player.h"
#include "media_processor.h"
#include "image_handler.h"

struct VideoStreamPlayerQueueElement
{
    SRef<SImage*> image;

    VideoStreamPlayerQueueElement(SRef<SImage*> _image = NULL)
            : image(_image)
    {}
};

class Video_Stream_Player : public Stream_Player, public Image_Handler, public Media_Pipeline_Output_Handler
{
public:
    Video_Stream_Player(IStream_To_Streams_Player *_owner, SRef<Image_Handler *> _sink,
                        const int &_rtpTimestampSamplingRate, const unsigned int &bufferOverflowUnderflowModifier_us,
                        const unsigned int &synchronizationToleration_us);

    virtual ~Video_Stream_Player();
    void handle_data(const SRef<Processing_Data*>& data);
    virtual void run();
    virtual void handle(const SRef<SImage*>& image);
protected:
    void drop_oldest_non_resizing_queued_frame_if_older_than(const unsigned int &olderThan_us);
    int64_t timestampOfLastQueuedFrame_us;
    int64_t fromTimestampToPlaybackRTC_us;
    SRef<Image_Handler *> sink;
    std::deque<VideoStreamPlayerQueueElement> queue;
};

#endif // VIDEO_STREAM_PLAYER_H
