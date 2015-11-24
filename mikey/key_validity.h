#ifndef KEY_VALIDITY_H
#define KEY_VALIDITY_H

#include "mikey_defs.h"
#include "sobject.h"

#define KEYVALIDITY_NULL     0
#define KEYVALIDITY_SPI      1
#define KEYVALIDITY_INTERVAL 2

#define Key_Validity_Null Key_Validity

class Key_Validity : public SObject
{
public:
    Key_Validity();
    Key_Validity( const Key_Validity& );
    ~Key_Validity();

    void operator =( const Key_Validity& );
    virtual int length();
    int type();
    virtual void write_data( byte_t * start, int expectedLength );
    virtual std::string debug_dump();
    virtual std::string get_mem_object_type() const { return "Key_Validity"; }
protected:
    int type_value;
};

class Key_Validity_SPI : public Key_Validity
{
public:
    Key_Validity_SPI();
    Key_Validity_SPI(const Key_Validity_SPI& source);
    Key_Validity_SPI( byte_t * rawData, int lengthLimit );
    Key_Validity_SPI( int length, byte_t * spi );
    virtual ~Key_Validity_SPI();

    void operator =(const Key_Validity_SPI& source);
    virtual int length();
    virtual void write_data( byte_t * start, int expectedLength );
    virtual std::string debug_dump();
private:
    int spi_length;
    byte_t *spi_ptr;
};

class Key_Validity_Interval : public Key_Validity
{
public:
    Key_Validity_Interval();
    Key_Validity_Interval(const Key_Validity_Interval& source);
    Key_Validity_Interval(byte_t * raw_data, int length_limit );
    Key_Validity_Interval( int vfLength, byte_t * vf,
                         int vtLength, byte_t * vt );
    virtual ~Key_Validity_Interval();

    void operator =(const Key_Validity_Interval& source);
    virtual int length();
    virtual void write_data(byte_t * start, int expectedLength);
    virtual std::string debug_dump();
private:
    int vf_length;
    byte_t * vf;
    int vt_length;
    byte_t * vt;
};

#endif // KEY_VALIDITY_H
