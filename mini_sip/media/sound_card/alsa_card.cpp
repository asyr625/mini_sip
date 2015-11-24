#include "alsa_card.h"
Alsa_Card::Alsa_Card( std::string cardname, std::string devname )
    : card_name(cardname),
      dev_name(devname)
{
}

std::string Alsa_Card::get_card_name()
{
    return card_name;
}

std::string Alsa_Card::get_dev_name()
{
    return dev_name;
}

std::list<Alsa_Card *> Alsa_Card::get_card_list()
{
    list<Alsa_Card *> output;
    snd_ctl_t *handle;
    int card = -1;
    char name[ 32 ];

    snd_ctl_card_info_t *info;
    snd_pcm_info_t *pcminfo;
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_alloca(&pcminfo);

    while( snd_card_next( &card ) >= 0 && card >= 0 )
    {
        sprintf( name, "hw:%u", card );

        if( snd_ctl_open( &handle, name, 0 ) < 0 )
        {
            cerr << "Could not open card number" << card << endl;
            continue;
        }

        if( snd_ctl_card_info( handle, info ) < 0 )
        {
            cerr << "Could not get info regarding card number " << card << endl;
            continue;
        }

        output.push_back( new Alsa_Card(
                std::string( snd_ctl_card_info_get_name( info ) ),
                std::string( snd_ctl_card_info_get_mixername( info )
                      )));
    }

    return output;
}
