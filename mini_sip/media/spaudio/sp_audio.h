#ifndef SP_AUDIO_H
#define SP_AUDIO_H

#include "my_types.h"
#include "sobject.h"

#define SPATIAL_POS 5

#define SPATIAL_MAXSOURCES 10

class Sound_Source;

class Sp_Audio : public SObject
{
public:
    Sp_Audio(int32_t numPos);

    void init();

    virtual std::string get_mem_object_type() const {return "SpAudio";}
    int32_t get_num_pos();

    int32_t spatialize (short *input, SRef<Sound_Source *> src, short *outbuff);

    int32_t assign_pos(int row, int col);

    int32_t lchdelay[SPATIAL_POS];

    int32_t rchdelay[SPATIAL_POS];
    int32_t assmatrix[SPATIAL_MAXSOURCES][SPATIAL_MAXSOURCES];

    short int lookup_left_global[65536][SPATIAL_POS];
    short int lookup_right_global[65536][SPATIAL_POS];

private:
    int32_t nPos;
};

#endif // SP_AUDIO_H
