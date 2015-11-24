#include "rollover_prone_to_monotonic.h"
#include "my_assert.h"

template <class Rollover_Prone, class Monotonic>
Rollover_Prone_To_Monotonic<Rollover_Prone, Monotonic>::Rollover_Prone_To_Monotonic()
    : rollover_addition(0), rollover_addition_count(0)
{
    my_assert(sizeof(Rollover_Prone) < sizeof(Monotonic));
    for(int i = 0; i < sizeof(Rollover_Prone); ++i)
        rollover_addition |= 0xff << (i*8);

    for(int i = 0; i < count; ++i)
        value_present[i]=false;
}


template <class Rollover_Prone, class Monotonic>
Monotonic Rollover_Prone_To_Monotonic<Rollover_Prone, Monotonic>::monotonic(const Rollover_Prone &rollover_prone)
{
    Value value;
    if( rollover_prone < rollover_addition / 4 )
        value = low;
    else if( rollover_prone > rollover_addition / 4 * 3 )
        value = high;
    else
        value = medium;

    if( value_present[(value + count - 1) % count] )
        value_present[(value + count - 2) % count] = false;

    if( value == medium && !value_present[medium] )
        ++rollover_addition_count;
    value_present[value] = true;

    unsigned int effectiveRolloverAdditionCount = (value == low && !value_present[medium]) ? rollover_addition_count+1 : rollover_addition_count;
    return Monotonic(rollover_prone) + rollover_addition * effectiveRolloverAdditionCount;
}
