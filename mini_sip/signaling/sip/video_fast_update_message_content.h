#ifndef VIDEO_FAST_UPDATE_MESSAGE_CONTENT_H
#define VIDEO_FAST_UPDATE_MESSAGE_CONTENT_H

#include "sip_message_content.h"

SRef<Sip_Message_Content*> videoFastUpdateMessageContentFactory(const std::string &, const std::string &ContentType);

class Video_Fast_Update_Message_Content : public Sip_Message_Content
{
    public:
        Video_Fast_Update_Message_Content();
        Video_Fast_Update_Message_Content(const std::string &buildFrom);
        virtual std::string get_mem_object_type() const { return "VideoFastUpdateMessageContent"; }
        virtual std::string get_string() const;
        virtual std::string get_content_type() const { return "application/media_control+xml"; }
};

#endif // VIDEO_FAST_UPDATE_MESSAGE_CONTENT_H
