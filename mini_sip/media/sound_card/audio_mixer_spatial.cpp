#include "audio_mixer_spatial.h"
#include "sound_source.h"
#include "sp_audio.h"

#include "string_utils.h"

#ifdef _WIN32_WCE
#	include "minisip_wce_extra_includes.h"
#endif

#include <string.h>

using namespace std;

Audio_Mixer_Spatial::Audio_Mixer_Spatial(SRef<Sp_Audio *> spatial)
{
    this->sp_audio = spatial;
}

Audio_Mixer_Spatial::~Audio_Mixer_Spatial()
{
}


short * Audio_Mixer_Spatial::mix(std::list<SRef<Sound_Source *> > sources)
{
    uint32_t size = frame_size * num_channels;
    int32_t pointer;

    memset( mix_buffer, '\0', size * sizeof( int32_t ) );

    for (list<SRef<Sound_Source *> >::iterator i = sources.begin(); i != sources.end(); i++)
    {
        (*i)->get_sound( input_buffer );

        /* spatial audio */
        pointer = this->sp_audio->spatialize(input_buffer, (*i),
                                             output_buffer); //use this buffer as temporary output

        (*i)->set_pointer(pointer);

        for (uint32_t j=0; j<size; j++)
        {
#ifdef IPAQ
            /* iPAQ hack, to reduce the volume of the
                * output */
            mix_buffer[j] += (output_buffer[j]/32);
#else
            mix_buffer[j] += output_buffer[j];
#endif
        }
    }
    //mix buffer is 32 bit to prevent saturation ...
    // some kind of normalization/scaling should be performed here
    //TODO: for now, simply copy the mix to the output buffer
    for( uint32_t k = 0; k < size; k++ )
        output_buffer[k] = (short)mix_buffer[k];

    return output_buffer;
}

bool Audio_Mixer_Spatial::set_sources_position( std::list<SRef<Sound_Source *> > &sources, bool addingSource )
{
    list< SRef<Sound_Source *> >::iterator it;
    int sourceIdx;

    if( addingSource )
    {
        int size = (int)sources.size();
        int newPosition = 1;
        //if we have 5 sources, optimize the result with this
        //previous knowledge we have
        if( SPATIAL_POS == 5 )
        {
            switch( size )
            {
            case 1:
            case 3:
            case 5: newPosition = 3; break;
            case 2: newPosition = 5; break;
            case 4: newPosition = 4; break;
            }
        }
        else
        {
            if( SPATIAL_POS % 2 )  //if odd number of positions
            {
                newPosition = (SPATIAL_POS/2) + 1; //tend to send it up high
            } else {
                newPosition = (SPATIAL_POS/2);
            }
        }
        it = sources.end();
        it --;
        (*it)->set_pos( newPosition );
        //cerr << "CESC: Audio_Mixer_Spatial:adding: newPosition = " << itoa(newPosition) << endl;
        sort_sound_source_list( sources );
        for( it = sources.begin(), sourceIdx = 1; it != sources.end(); it++, sourceIdx++ )
        {
            int pos = sp_audio->assign_pos(sourceIdx, (int)sources.size() );
#ifdef DEBUG_OUTPUT
            cerr << "Audio_Mixer_Spatial::set_sources_position: adding: set source id = " <<
                    itoa((*it)->get_id() ) << endl <<
                    "            to position = " << itoa( pos ) << endl;
#endif
            (*it)->set_pos( pos );
        }
    }
    else  //we have just removed a source ...
    {
        //sources are still sorted correctly ... simply reassing the positions ...
        for( it = sources.begin(), sourceIdx = 1; it != sources.end(); it++, sourceIdx++ )
        {
            int pos = sp_audio->assign_pos(sourceIdx, (int)sources.size() );
#ifdef DEBUG_OUTPUT
            cerr << "AudioMixerSpatial::setSourcesPosition:removing: set source id = " <<
                    itoa((*it)->get_id() ) << endl <<
                    "            to position = " << itoa( pos ) << endl;
#endif
            (*it)->set_pos( pos );
        }
    }
    return true;
}

bool Audio_Mixer_Spatial::sort_sound_source_list( std::list<SRef<Sound_Source *> > &srcList )
{
    //very rudimentary ... but much easier than using the sort from STL ..
    int max;
    SRef<Sound_Source *> ref;

    list<SRef<Sound_Source *> > tmpList;
    list<SRef<Sound_Source *> >::iterator srcIt, destIt;

    while( srcList.size() > 0 )
    {
        max = -1;
        for( srcIt = srcList.begin(); srcIt!=srcList.end(); srcIt++ )
        {
            if( (*srcIt)->get_pos() >= max )
            {
                ref = (*srcIt);
                max = ref->get_pos();
            }
        }
        tmpList.push_front( ref );
        srcList.remove( ref );
    }
    srcList = tmpList;
    return true;
}
