#ifndef CONTACT_DB_H
#define CONTACT_DB_H

#include <list>
#include "sobject.h"
#include "my_types.h"

class Contact_Db;
class Phone_Book;
class Phone_Book_Person;

#define CONTACT_STATUS_ONLINE 1
#define CONTACT_STATUS_OFFLINE 2
#define CONTACT_STATUS_UNKNOWN 3

class Contact_Entry : public SObject
{
public:
    Contact_Entry();
    Contact_Entry( std::string uri, std::string desc,
                    SRef< Phone_Book_Person * > person = NULL );

    ~Contact_Entry();

    static void set_db( SRef<Contact_Db *> db );

    std::string get_name();
    std::string get_uri();
    std::string get_desc();

    void set_desc(std::string d );
    void set_uri( std::string uri );

    uint32_t get_id();

    bool is_online() { return online_status == CONTACT_STATUS_ONLINE; }
    bool is_offline() { return online_status == CONTACT_STATUS_OFFLINE; }
    void set_online_status(int s) { online_status = s; }
    void set_online_status_desc( std::string s) { online_status_desc = s; }

    uint32_t get_person_index() { return person_index; }

    virtual std::string get_mem_object_type() const { return "ContactEntry"; }
private:
    static SRef<Contact_Db *> db;

    uint32_t id;
    std::string uri;
    std::string desc;
    uint32_t type;
    SRef< Phone_Book_Person * > person;
    uint32_t person_index;

    std::string location;
    int online_status;
    std::string online_status_desc;
    friend class Phone_Book_Person;
    friend class Phone_Book;
};

class Contact_Db : public SObject
{
public:
    Contact_Db();

    Contact_Entry* look_up(std::string uri);
    Contact_Entry* look_up(unsigned int id);

    void add_entry( Contact_Entry * entry );
    void del_entry( Contact_Entry * entry );

private:
    std::list<Contact_Entry*> entries;
};

#endif // CONTACT_DB_H
