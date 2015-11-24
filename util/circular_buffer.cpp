#include "circular_buffer.h"
#include <string.h>


Circular_Buffer::Circular_Buffer(int size_)
    : max_size(size_),
      size(0),
      read_idx(0),
      write_idx(0),
      byte_counter(0)
{
    buf = new short[size_];
}

Circular_Buffer::Circular_Buffer(const Circular_Buffer &b):
    max_size(b.max_size),
    size(b.size),
    read_idx(b.read_idx),
    write_idx(b.write_idx),
    byte_counter(b.byte_counter)
{
    buf = new short[size];
    memcpy(buf, b.buf, size*sizeof(short));
}

Circular_Buffer::~Circular_Buffer()
{
    if( buf != NULL )
    {
        delete[] buf;
    }
}

bool Circular_Buffer::write(const short *s, int len, bool forcedWrite )
{

    if( len > get_free() )
    {
        if( !forcedWrite )
        {
            return false; // overflow
        }
        else
        {
            if( len > get_max_size() )
            {
                return false; //force write but not enough capacity
            } else
            { //remove enough stuff to fit this forces write
                int removeNum;
                removeNum = len - get_free();
                // 				printf("CBRM\n");
                if( ! remove( removeNum )  )
                {
                    return false; //strange error ...
                }
            }
        }
    }

    //from here, just write ...
    //if the write is forced, we already made space for it to fit ...

    byte_counter += (unsigned long)len;

    //Check if write block crosses the circular border
    if ( write_idx + len > get_max_size() )
    {
        int lenLeft = get_max_size() - write_idx; // size left until circular border crossing
        memcpy(buf + write_idx, s, lenLeft * sizeof(short));
        memcpy(buf, s + lenLeft, (len - lenLeft)* sizeof(short));
        write_idx = len - lenLeft;
    }
    else
    {
        memcpy(buf + write_idx, s, len * sizeof(short));
        write_idx += len;
        if (write_idx >= get_max_size())
            write_idx -= get_max_size();
    }
    size += len;
    return true;
}

bool Circular_Buffer::read(short *s, int len)
{
    if (len > get_size() )
    {
        return false; // not enough chars
    }

    if (read_idx + len > get_max_size() )
    { // block crosses circular border
        int lenLeft = get_max_size() - read_idx;
        if (s)
        {
            memcpy(s, buf + read_idx, lenLeft * sizeof(short));
            memcpy(s + lenLeft, buf, (len - lenLeft) * sizeof(short));
        }
        read_idx = len - lenLeft;
    }
    else
    {
        if (s)
        {
            memcpy(s, buf + read_idx, len * sizeof(short));
        }
        read_idx += len;
        if ( read_idx >= get_max_size() )
            read_idx -= get_max_size();
    }
    size -= len;

    if ( get_size() == 0 )
    {
        read_idx = write_idx = 0;
    }
    return true;
}

bool Circular_Buffer::remove(int len)
{
    return read(NULL, len);
}

void Circular_Buffer::clear()
{
    read( NULL, get_size() ); //remove all current elements ... = clear
}
