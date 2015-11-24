#include "video_fast_update_message_content.h"

SRef<Sip_Message_Content*> videoFastUpdateMessageContentFactory(const std::string &buf, const std::string &ContentType)
{
    return new Video_Fast_Update_Message_Content(buf);
}

Video_Fast_Update_Message_Content::Video_Fast_Update_Message_Content()
{
}

Video_Fast_Update_Message_Content::Video_Fast_Update_Message_Content(const std::string &buildFrom)
{

}

std::string Video_Fast_Update_Message_Content::get_string() const
{
    return "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n\n"
             "<media_control>\n\n"
               "<vc_primitive>\n"
                 "<to_encoder>\n"
                   "<picture_fast_update/>\n"
                 "</to_encoder>\n"
               "</vc_primitive>\n\n"
             "</media_control>";
}
