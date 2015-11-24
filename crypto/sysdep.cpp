#include "sysdep.h"

#include <stdio.h>
#include "rand.h"
#include "sysdep.h"
#include "my_time.h"
#include "my_types.h"

/* system dependent call to get IEEE node ID.
   This sample implementation generates a random node ID. */
void get_ieee_node_identifier(uuid_node_t *node)
{
    char seed[6];

    memset(seed, 0, sizeof(seed));
    Rand::randomize(seed, sizeof(seed));
    seed[0] |= 0x01;
    memcpy(node, seed, sizeof(*node));
}

void get_system_time(uuid_time_t *uuid_time)
{
    /* Offset between UUID formatted times and Unix formatted times.
       UUID UTC base time is October 15, 1582.
       Unix base time is January 1, 1970.*/
    *uuid_time = ((unsigned64_t)my_time() * 10000)
        + 0x01B21DD213814000LL;
}
