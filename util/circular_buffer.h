#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

class Circular_Buffer
{
public:
    Circular_Buffer(int size);
    Circular_Buffer(const Circular_Buffer &);
    virtual ~Circular_Buffer();

    virtual bool write( const short *s, int len, bool forcedWrite = false);

    virtual bool read( short *s, int len);

    bool remove(int len);

    void clear();

    int get_size() { return size; }

    int get_max_size() { return max_size; }

    int get_free() { return (get_max_size() - get_size() ); }

    unsigned long get_byte_counter() { return byte_counter; }

private:
    Circular_Buffer& operator=(const Circular_Buffer& ) { return *this; }

    char * getReadStart();

    short *buf;
    int max_size;
    int size;
    int read_idx;
    int write_idx;
    unsigned long byte_counter;
};

#endif // CIRCULAR_BUFFER_H
