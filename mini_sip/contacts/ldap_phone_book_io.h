#ifndef LDAP_PHONE_BOOK_IO_H
#define LDAP_PHONE_BOOK_IO_H

#include "ldap_url.h"
#include "phone_book.h"

#include<string>

class Ldap_Phone_Book_Io : public Phone_Book_Io
{
public:
    Ldap_Phone_Book_Io( std::string fileName );

    virtual SRef<Phone_Book *> load();
    virtual void save(SRef<Phone_Book *> pb);
    virtual std::string get_mem_object_type() const {return "LdapPhoneBookIo";}

    virtual std::string get_phone_book_id();

private:
    Ldap_Url url;
};

class Ldap_Phone_Book_Io_Driver : public Phone_Book_Io_Driver
{
public:
    Ldap_Phone_Book_Io_Driver( SRef<Library *> lib );
    virtual ~Ldap_Phone_Book_Io_Driver();

    virtual std::string get_mem_object_type() const {return "LdapPhoneBookIoDriver";}

    virtual std::string get_description() const { return "Ldap PhoneBook IO driver"; }
    virtual std::string get_name() const { return "LdapPhoneBookIo"; }
    virtual uint32_t get_version() const {return 0x00000001;}

    virtual SRef<Phone_Book_Io*> create_phone_book_io(std::string name) const;
    virtual std::string get_prefix() const { return "ldap"; }
};

#endif // LDAP_PHONE_BOOK_IO_H
