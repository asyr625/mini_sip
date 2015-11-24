#ifndef XML_PHONE_BOOK_IO_H
#define XML_PHONE_BOOK_IO_H

#include "phone_book.h"
#include <string>

class Xml_Phone_Book_Io : public Phone_Book_Io
{
public:
    Xml_Phone_Book_Io( std::string fileName );

    virtual void save(SRef< Phone_Book * > pb );
    virtual SRef< Phone_Book * > load();
    virtual std::string get_mem_object_type() const {return "PhoneBookIo";}

    virtual std::string get_phone_book_id();
private:
    std::string get_default_phone_book_string();
    void create_default();
    std::string file_name;
};

class Xml_Phone_Book_Io_Driver : public Phone_Book_Io_Driver
{
public:
    Xml_Phone_Book_Io_Driver( SRef<Library *> lib );
    virtual ~Xml_Phone_Book_Io_Driver();

    virtual std::string get_mem_object_type() const {return "PhoneBookIo";}

    virtual std::string get_description() const { return "Xml PhoneBook IO driver"; }
    virtual std::string get_name() const { return "XmlPhoneBookIo"; }
    virtual uint32_t get_version() const {return 0x00000001;}

    virtual SRef<Phone_Book_Io*> create_phone_book_io(std::string name) const;
    virtual std::string get_prefix() const { return "file"; }
};

#endif // XML_PHONE_BOOK_IO_H
