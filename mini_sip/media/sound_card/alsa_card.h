#ifndef ALSA_CARD_H
#define ALSA_CARD_H
#include <string>
#include <list>

extern "C"{
    #include <alsa/asoundlib.h>
}

class Alsa_Card
{
public:
    Alsa_Card( std::string cardname, std::string devname );

    std::string get_card_name();
    std::string get_dev_name();

    static std::list<Alsa_Card *> get_card_list();
private:
    std::string card_name;
    std::string dev_name;
};

#endif // ALSA_CARD_H
