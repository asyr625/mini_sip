#ifndef PHONE_BOOK_H
#define PHONE_BOOK_H

#include "sobject.h"
#include "splugin.h"
#include "ssingleton.h"

#include "contact_db.h"

class Phone_Book_Io;
class Phone_Book_Person;

class Phone_Book : public SObject
{
public:
    static SRef<Phone_Book*> create(SRef<Phone_Book_Io*> io);

    void save();

    void set_io(SRef<Phone_Book_Io*> io);
    void set_name(std::string n);
    std::string get_name();

    void add_person(SRef<Phone_Book_Person*> person);
    void del_person(SRef<Phone_Book_Person*> person);

    std::list<SRef<Phone_Book_Person*> >& get_persons();

    std::string get_phone_book_id();
    virtual std::string get_mem_object_type() const { return "PhoneBook"; }

private:
    Phone_Book();
    SRef<Phone_Book_Io*> io;
    std::string name;
    std::list<SRef<Phone_Book_Person*> > persons;
};

class Phone_Book_Io : public SObject
{
public:
    virtual void save(SRef<Phone_Book*> pb) = 0;
    virtual SRef<Phone_Book*> load() = 0;
    virtual std::string get_phone_book_id() = 0;
};


class Phone_Book_Person : public SObject
{
public:
    Phone_Book_Person(std::string n);
    ~Phone_Book_Person();

    std::string get_name();

    void set_name(std::string n);

    void set_phone_book(SRef<Phone_Book*> pb);

    std::list<SRef<Contact_Entry*> >& get_entries();

    void add_entry(SRef<Contact_Entry*> ce);
    void del_entry(SRef<Contact_Entry*> ce);

    virtual std::string get_mem_object_type() const { return "PhoneBookPerson"; }
private:
    std::string name;
    SRef<Phone_Book*> phone_book;
    SRef<Contact_Entry*> default_entry;
    std::list<SRef<Contact_Entry*> > entries;
};


class Phone_Book_Io_Driver : public SPlugin
{
public:
    virtual std::string get_prefix() const = 0;
    virtual SRef<Phone_Book_Io*> create_phone_book_io(std::string name) const = 0;
    std::string get_plugin_type() const { return "PhoneBookIo"; }
private:
    Phone_Book_Io_Driver(SRef<Library*> lib);
    Phone_Book_Io_Driver();
};


class Phone_Book_Io_Registry : public SPlugin_Registry, public SSingleton<Phone_Book_Io_Registry>
{
public:
    std::string get_plugin_type() const { return "PhoneBookIo"; }
    SRef<Phone_Book*> create_phone_book(const std::string &name);
protected:
    Phone_Book_Io_Registry();
private:
    friend class SSingleton<Phone_Book_Io_Registry>;
};

#endif // PHONE_BOOK_H
