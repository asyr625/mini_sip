#ifndef DBG_H
#define DBG_H
#include <string>
#include <set>
#include "my_types.h"
#include "mutex.h"
class Dbg_Handler
{
public:
    virtual ~Dbg_Handler();
protected:
    virtual void display_message(std::string output, int style = -1 ) = 0;
private:
    friend class Dbg;
};

class Dbg
{
public:
    Dbg(std::string name="", bool error_output=false, bool enabled=true);
    ~Dbg();

    Dbg &operator<<( const std::string& );
    Dbg &operator<<( int );
    Dbg &operator<<( unsigned int );
    Dbg &operator<<( long long );
    Dbg &operator<<( uint64_t );
    Dbg &operator<<( const char );
    Dbg &operator<<( const char *);
    Dbg &operator<<( void *);

    //accepts std::endl as parameter
    Dbg &operator<<( std::ostream&(*arg)(std::ostream&) );

    void set_enabled(bool enabled);
    bool get_enabled();

    void set_external_handler(Dbg_Handler * dbg_handler);

    void set_print_stream_name(bool b);

    void set_print_timestamp(bool b);

    Dbg& operator()(std::string oClass);
    void include(std::string);
    void exclude(std::string);

private:
    void update_filter();
    std::string _name;
    bool _error_out;
    bool _enabled;
    std::string _str;
    Dbg_Handler * _debug_handler;

    bool _default_include;
    std::string _cur_class;
    std::set< std::string > _include_set;
    std::set< std::string > _exclude_set;
    bool _filter_blocking;
    static Mutex _lock;
    bool _print_name;
    bool _print_timestamp;
};


extern Dbg my_out;
extern Dbg my_err;
extern Dbg my_dbg;

extern bool output_state_machine_debug;
#endif // DBG_H
