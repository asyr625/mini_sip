#include "presence_message_content.h"


SRef<Sip_Message_Content*> presence_sip_message_content_factory(const std::string &buf, const std::string &ContentType)
{
    return new Presence_Message_Content(buf);
}




Presence_Message_Content::Presence_Message_Content(std::string from, std::string to, std::string onlineStatus,
                                                   std::string onlineStatusDesc)
    : from_uri(from), to_uri(to), online_status(onlineStatus),
      online_status_desc(onlineStatusDesc)
{
    std::string status;
    if( onlineStatus == "online")
    {
        status = "open";
    }
    else
    {
        status = "inuse";
    }
    document = "<?xml version=\"1.0\"?>\n"
        "<!DOCTYPE presence\n"
        "PUBLIC \"-//IETF//DTD RFCxxxx XPIDF 1.0//EN\" \"xpidf.dtd\">\n"
        "<presence>\n"
        "  <presentity uri=\"" +to_uri+";method=SUBSCRIBE\" />\n"
        "  <atom id=\"1000\">\n"
        "    <address uri=\""+from_uri+"\">\n"
        "      <status status=\""+status+"\" />\n" 			//"open"|("closed")|"inuse"
        "      <msnsubstatus substatus=\""+online_status+"\" />\n"
        "    </address>\n"
        "  </atom>\n"
        "</presence>\n";
}

Presence_Message_Content::Presence_Message_Content(const std::string &buildFrom)
    : document(buildFrom)
{
}

std::string Presence_Message_Content::get_string() const
{
    return document;
}
