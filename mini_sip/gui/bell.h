 #ifndef BELL_H
#define BELL_H

#include <string>
#include "my_types.h"

class Bell
{
public:
    Bell();
    ~Bell();
    void start();
    void stop();
    void loop();
    void timeout(const std::string &command);
    virtual std::string get_mem_object_type() const { return "Bell"; }
private:
    volatile bool running;
    int32_t delay_index;
#ifdef IPAQ
    int ipaq_buzzer;
#endif
};

#ifdef IPAQ
#define BUZZER_IOCTL_BASE       'Z'
struct buzzer_time{
    unsigned int on_time;
    unsigned int off_time;
};
#define IOC_SETBUZZER   _IOW(BUZZER_IOCTL_BASE, 0, struct buzzer_time)
#endif

#endif // BELL_H
