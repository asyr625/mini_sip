#ifndef SDP_HEADERM_H
#define SDP_HEADERM_H
#include <vector>
#include <list>

#include "sdp_header.h"
#include "sdp_headera.h"
#include "sdp_headerb.h"
#include "sdp_headerc.h"
#include "sdp_headeri.h"

class Sdp_HeaderM : public Sdp_Header
{
public:
    Sdp_HeaderM(std::string buildFrom);
    Sdp_HeaderM(std::string media_, int32_t port_, int32_t n_ports, std::string transp);
    Sdp_HeaderM(const Sdp_HeaderM &src);
    virtual ~Sdp_HeaderM();

    Sdp_HeaderM &operator=(const Sdp_HeaderM &src);

    virtual std::string get_mem_object_type() const {return "Sdp_HeaderM";}

    void add_format(std::string f);
    int32_t get_nr_formats() const;
    std::string get_format(int32_t i) const;

    std::string get_media() const;
    void set_media(std::string m);

    int32_t get_port() const;
    void set_port(int32_t p);

    int32_t get_nr_ports() const;
    void set_nr_ports(int32_t n);

    std::string get_transport() const;
    void set_transport(std::string t);

    virtual std::string get_string() const;

    void add_attribute(SRef<Sdp_HeaderA*> a);

    std::string get_attribute(std::string key, uint32_t n) const;
    std::list<SRef<Sdp_HeaderA*> > get_attributes() const;

    std::string get_rtp_map(std::string format) const;

    std::string get_fmtp_param(std::string format) const;

    void set_bandwidth( SRef<Sdp_HeaderB*> b );
    SRef<Sdp_HeaderB*> get_bandwidth() const;

    void set_connection( SRef<Sdp_HeaderC*> c );
    SRef<Sdp_HeaderC*> get_connection() const;

    void set_information(SRef<Sdp_HeaderI*> i);
    SRef<Sdp_HeaderI*> get_information() const;

    void copy_attributes ( SRef <Sdp_HeaderM * > m , bool changing_direction);

    std::string get_direction_attribute() const;

    void set_direction_attribute(std :: string str );
    void swap_direction_attribute ();
    std ::string str_attr;

private:
    std::string media;
    int32_t port;
    int32_t nPorts;
    std::string transport;
    std::vector<std::string> formats;
    std::list<SRef<Sdp_HeaderA*> >attributes;
    SRef<Sdp_HeaderB*> bandwidth;
    SRef<Sdp_HeaderC*> connection;
    SRef<Sdp_HeaderI*> information;
};

#endif // SDP_HEADERM_H
