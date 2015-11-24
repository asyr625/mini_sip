#ifndef RTCP_REPORT_APP_CAMCTRL_H
#define RTCP_REPORT_APP_CAMCTRL_H
#include "my_types.h"
#include "rtcp_report.h"

class Rtcp_Report_App_Camctrl : public Rtcp_Report
{
public:
    Rtcp_Report_App_Camctrl(unsigned subtype, uint32_t ssrc, int8_t horizontalSpeed, int8_t verticalSpeed, int8_t zoomSpeed, uint8_t durationHundredsS);
    Rtcp_Report_App_Camctrl(void * build_from, int max_length);
    virtual ~Rtcp_Report_App_Camctrl();

    int get_length();
    void write_data(char* buf);

    virtual void debug_print();
    unsigned int get_subtype();
    unsigned int get_senderssrc();

    int get_hspeed() { return horizontal_speed; }
    int get_vspeed() { return vertical_speed; }
    int get_zspeed() { return zoom_speed; }
    int get_duration() { return duration_hundredsS; }

private:
    unsigned sender_ssrc;
    int8_t horizontal_speed;
    int8_t vertical_speed;
    int8_t zoom_speed;
    uint8_t duration_hundredsS;
};

#endif // RTCP_REPORT_APP_CAMCTRL_H
