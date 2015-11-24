#include "statistics_text_plane.h"

Statistics_Text_Plane::Statistics_Text_Plane()
    : Text_Plane(20, RGBA(40, 40, 80, 255*0.8))
{
}


Statistics_Text_Plane::~Statistics_Text_Plane()
{
}

void Statistics_Text_Plane::generate(std::list< SRef<Session *> > sessions)
{
    clear();
    Text_Fragment textFragment;
    textFragment.color = RGB(255, 255, 255);

    if (sessions.size()==0)
    {
        textFragment.font = new SFont(36);
        textFragment.text = "No sessions";
        Text_Line *line = new Text_Line;
        line->text_line.push_back(textFragment);
        add_text_line(line);
        return;
    }

    for(std::list< SRef<Session *> >::iterator it = sessions.begin(), end = sessions.end(); it!=end; ++it)
    {
        if(!(*it)->is_active())
            continue;

        textFragment.font = new SFont(12);
        textFragment.text = " ";
        Text_Line *line = new Text_Line;
        line->text_line.push_back(textFragment);
        add_text_line(line);

        textFragment.font = new SFont(36);
        textFragment.text = (*it)->get_name();
        line = new Text_Line;
        line->text_line.push_back(textFragment);
        add_text_line(line);

        std::list< SRef<Realtime_Media_Stream_Receiver *> > mediaStreamReceivers = (*it)->get_realtime_media_stream_receivers();
        {
            for(std::list< SRef<Realtime_Media_Stream_Receiver *> >::iterator rmsr_it = mediaStreamReceivers.begin(), end=mediaStreamReceivers.end(); rmsr_it!=end; ++rmsr_it)
            {
                Realtime_Media *realtimeMedia = *(*rmsr_it)->get_realtime_media();
                if(realtimeMedia)
                {
                    if(rmsr_it == mediaStreamReceivers.begin())
                    {
                        textFragment.font = new SFont(32);
                        textFragment.text = "   Received streams";
                        line = new Text_Line;
                        line->text_line.push_back(textFragment);
                        add_text_line(line);
                    }
                    std::list<Receiving_MSS_Reporter *> mediaSources = realtimeMedia->get_receiving_media_sources();
                    for(std::list<Receiving_MSS_Reporter *>::iterator rmssr_it = mediaSources.begin(), end=mediaSources.end(); rmssr_it!=end; ++rmssr_it)
                    {
                        char tmp[30];
                        textFragment.font = new SFont(24);
                        textFragment.text = "        ";
                        textFragment.text += (*rmssr_it)->get_decoder_description();
                        line = new Text_Line;
                        line->text_line.push_back(textFragment);
                        add_text_line(line);

                        float fps = (*rmssr_it)->get_received_video_framerate_fps();
                        if(fps >= 0)
                        {
                            textFragment.text = "        Framerate: ";
                            sprintf(tmp, "%.1f fps", fps);
                            textFragment.text += tmp;
                            line = new Text_Line;
                            line->text_line.push_back(textFragment);
                            add_text_line(line);
                        }

                        textFragment.text = "        Bandwidth: ";
                        sprintf(tmp, "%u", (*rmssr_it)->get_received_throughput_kbps());
                        textFragment.text += tmp;
                        textFragment.text += " kbps";
                        line = new Text_Line;
                        line->text_line.push_back(textFragment);
                        add_text_line(line);

                        textFragment.text = "        Packets lost: ";
                        sprintf(tmp, "%llu", (*rmssr_it)->get_number_of_missing_packets());
                        textFragment.text += tmp;
                        textFragment.text += "/";
                        uint64_t totalNumberOfPackets = (*rmssr_it)->get_number_of_missing_packets() + (*rmssr_it)->get_number_of_received_packets();
                        sprintf(tmp, "%llu", totalNumberOfPackets);
                        textFragment.text += tmp;
                        textFragment.text += " (";
                        sprintf(tmp, "%.1f", float((*rmssr_it)->get_number_of_missing_packets() * 100) / totalNumberOfPackets);
                        textFragment.text += tmp;
                        textFragment.text += "%)";
                        line = new Text_Line;
                        line->text_line.push_back(textFragment);
                        add_text_line(line);

                        textFragment.font = new SFont(12);
                        textFragment.text = " ";
                        line = new Text_Line;
                        line->text_line.push_back(textFragment);
                        add_text_line(line);
                    }
                }
            }
        }

        std::list< SRef<Realtime_Media_Stream_Sender *> > mediaStreamSenders = (*it)->get_realtime_media_stream_senders();
        for(std::list< SRef<Realtime_Media_Stream_Sender *> >::iterator rmss_it = mediaStreamSenders.begin(), end=mediaStreamSenders.end(); rmss_it!=end; ++rmss_it)
        {
            if(rmss_it==mediaStreamSenders.begin())
            {
                textFragment.font = new SFont(30);
                textFragment.text = "   Sent streams";
                line = new Text_Line;
                line->text_line.push_back(textFragment);
                add_text_line(line);
            }
            char tmp[20];
            textFragment.font = new SFont(24);
            textFragment.text = "        ";
            textFragment.text += (*rmss_it)->get_encoder_description();
            line = new Text_Line;
            line->text_line.push_back(textFragment);
            add_text_line(line);

            float fps = (*rmss_it)->get_sent_video_framerate_fps();
            if(fps >= 0)
            {
                textFragment.text = "        Framerate: ";
                sprintf(tmp, "%.1f fps", fps);
                textFragment.text += tmp;
                line = new Text_Line;
                line->text_line.push_back(textFragment);
                add_text_line(line);
            }

            textFragment.text = "        Bandwidth: ";
            sprintf(tmp, "%u", (*rmss_it)->get_sent_throughput_kbps());
            textFragment.text += tmp;
            textFragment.text += " kbps";
            line = new Text_Line;
            line->text_line.push_back(textFragment);
            add_text_line(line);

            textFragment.text = "        Packets sent: ";
            sprintf(tmp, "%llu", (*rmss_it)->get_number_of_sent_packets());
            textFragment.text += tmp;
            line = new Text_Line;
            line->text_line.push_back(textFragment);
            add_text_line(line);

            textFragment.font = new SFont(12);
            textFragment.text = " ";
            line = new Text_Line;
            line->text_line.push_back(textFragment);
            add_text_line(line);
        }
    }
}
