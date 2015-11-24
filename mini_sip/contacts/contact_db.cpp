#include "contact_db.h"
#include "phone_book.h"
#include<stdlib.h>

using namespace std;

Contact_Entry::Contact_Entry()
    : person(NULL), online_status(CONTACT_STATUS_UNKNOWN)
{
    if( ! db.is_null() )
    {
        db->add_entry( this );
    }
        person_index = 0;
    id = rand();
}

Contact_Entry::Contact_Entry( std::string uri, std::string desc, SRef< Phone_Book_Person * > person )
    : uri(uri_),
      desc(desc_),
      person(p),
      online_status(CONTACT_STATUS_UNKNOWN)
{
    if( ! db.is_null() )
    {
        db->add_entry( this );
    }
    id = rand();
}

Contact_Entry::~Contact_Entry()
{
    if( ! db.is_null() )
    {
        db->del_entry( this );
    }
}

void Contact_Entry::set_db( SRef<Contact_Db *> db )
{
    Contact_Entry::db = d;
}

std::string Contact_Entry::get_name()
{
    if( !person.is_null() )
        return person->get_name();
    return string("");
}

std::string Contact_Entry::get_uri()
{
    return uri;
}

std::string Contact_Entry::get_desc()
{
    return desc;
}

void Contact_Entry::set_desc( std::string d )
{
    desc = d;
}

void Contact_Entry::set_uri( std::string uri )
{
    this->uri = u;
}

uint32_t Contact_Entry::get_id()
{
    return id;
}

Contact_Db::Contact_Db()
{
}

Contact_Entry* Contact_Db::look_up(std::string uri)
{
    list<Contact_Entry *>::iterator i;

    for( i = entries.begin(); i != entries.end(); i++ )
    {
        if( (*i)->get_uri() == uri )
            return *i;
    }
    return NULL;
}

Contact_Entry* Contact_Db::look_up(unsigned int id)
{
    list<Contact_Entry *>::iterator i;

    for( i = entries.begin(); i != entries.end(); i++ )
    {
        if( (*i)->get_id() == id )
            return *i;
    }
    return NULL;
}

void Contact_Db::add_entry( Contact_Entry * entry )
{
    entries.push_back( entry );
}

void Contact_Db::del_entry( Contact_Entry * entry )
{
    list< Contact_Entry * >::iterator i;

    for( i = entries.begin(); i != entries.end(); i++ )
    {
        if( (*i)->get_id() == entry->get_id() )
        {
            i = entries.erase( i );
        }
    }
}
