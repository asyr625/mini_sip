#ifndef SMART_CARD_H
#define SMART_CARD_H

#include "sobject.h"
#include "thread.h"
#include "message_router.h"

#include <map>


typedef long SCARDCONTEXT;
typedef long SCARDHANDLE;

class Smart_Card
{
public:
    Smart_Card();
    ~Smart_Card();

    void close();

    void start_transaction();
    void end_transaction();

    bool transmit_apdu(unsigned long sendLength, unsigned char * sendBufferPtr,
                      unsigned long & recvLength, unsigned char * recvBufferPtr);

    void set_pin(const char * pinCode);
    void set_admin_pin(const char * adminPinCode);
    virtual bool verify_pin(int verifyMode) = 0;
    virtual bool change_pin( const char * newPinCode) = 0;

protected:
    bool established_connection;
    unsigned long  reader_length;
    char * reader_names_ptr;
    std::map <int,char *> reader_map;
    SCARDCONTEXT hcontext;
    SCARDHANDLE hcard;

    unsigned char * user_pin_code;

    unsigned char * admin_pin_code;
    const void* prot_pci;
};

class Command_Receiver;

class Smart_Card_Detector : public virtual Runnable, public virtual Command_Receiver
{
public:
    Smart_Card_Detector(SRef<Command_Receiver*> callback);

    virtual void handle_command(std::string subsystem, const Command_String& command);
    virtual Command_String handle_command_resp(std::string subsystem, const Command_String&);

    void run();
    void start();
    void join();

private:
    void connect();
    std::string reader;
    SRef<Command_Receiver*> callback;
    bool do_stop;
    Thread_Handle th;
    bool pin_verified;
};

#endif // SMART_CARD_H
