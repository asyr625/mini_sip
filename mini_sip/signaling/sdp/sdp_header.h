#ifndef SDP_HEADER_H
#define SDP_HEADER_H

#include "my_types.h"
#include "sobject.h"

#define SDP_HEADER_TYPE_A	0
#define SDP_HEADER_TYPE_C	1
#define SDP_HEADER_TYPE_M	2
#define SDP_HEADER_TYPE_O	3
#define SDP_HEADER_TYPE_S	4
#define SDP_HEADER_TYPE_T	5
#define SDP_HEADER_TYPE_V	6
#define SDP_HEADER_TYPE_I	7
#define SDP_HEADER_TYPE_B	8

class Sdp_Header : public SObject
{
public:
    Sdp_Header(int type, int priority);
    Sdp_Header(int type, int priority, std::string value);

    virtual std::string get_string() const = 0;

    int32_t get_priority() const {return _priority;}
    int32_t get_type() const {return _type;}

    void set_priority(int32_t prio){this->_priority = prio;}
protected:
    bool _string_representation_up2date;
    std::string _string_representation;

private:
    int32_t _type;
    int32_t _priority;
};

#endif // SDP_HEADER_H
