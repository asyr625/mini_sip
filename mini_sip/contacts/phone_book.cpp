#include "phone_book.h"

#include "contact_db.h"
#include "xml_phone_book_io.h"
#include "dbg.h"

using namespace std;

Phone_Book::Phone_Book()
{
}
SRef<Phone_Book*> Phone_Book::create(SRef<Phone_Book_Io*> io)
{
    if( !io.is_null() )
    {
        SRef<Phone_Book * > phonebook = io->load();
        if( !phonebook.is_null() )
        {
            phonebook->set_io( io );
        }
        return phonebook;
    }
    return NULL;
}

void Phone_Book::save()
{
    if( !io.is_null() )
    {
        io->save( this );
    }
}

void Phone_Book::set_io(SRef<Phone_Book_Io*> io)
{
    this->io = io;
}

void Phone_Book::set_name(std::string n)
{
    name = n;
}

std::string Phone_Book::get_name()
{
    return name;
}

void Phone_Book::add_person(SRef<Phone_Book_Person*> person)
{
    person->set_phone_book( this );
    persons.push_back( person );
}

void Phone_Book::del_person(SRef<Phone_Book_Person*> person)
{
    list< SRef< Phone_Book_Person * > >::iterator i;

    for( i = persons.begin(); i != persons.end(); i++ )
    {
        if( *(*i) == *person )
        {
            i = persons.erase( i );
        }
    }
}

std::list<SRef<Phone_Book_Person*> >& Phone_Book::get_persons()
{
    return persons;
}

std::string Phone_Book::get_phone_book_id()
{
    return io->get_phone_book_id();
}


Phone_Book_Person::Phone_Book_Person(std::string n)
    : name(n)
{
}

Phone_Book_Person::~Phone_Book_Person()
{
    if( entries.size() != 0 )
    {
        list< SRef<Contact_Entry *> >::iterator i;

        for( i = entries.begin(); i != entries.end(); i++ )
        {
            del_entry( *i );
        }
    }
}

std::string Phone_Book_Person::get_name()
{
    return name;
}

void Phone_Book_Person::set_name(std::string n)
{
    name = n;
}

void Phone_Book_Person::set_phone_book(SRef<Phone_Book*> pb)
{
    phone_book = pb;
}

std::list<SRef<Contact_Entry*> >& Phone_Book_Person::get_entries()
{
    return entries;
}

void Phone_Book_Person::add_entry(SRef<Contact_Entry*> entry)
{
    entry->person_index = (uint32_t)entries.size();
    entries.push_back( entry );
}

void Phone_Book_Person::del_entry(SRef<Contact_Entry*> ce)
{
    list< SRef< Contact_Entry * > >::iterator i;
        uint32_t index = 0;

    for( i = entries.begin(); i != entries.end(); i++ )
    {
        if( *(*i) == *entry )
        {
            i = entries.erase( i );
            if( i == entries.end() )
            {
                break;
            }
        }
        (*i)->person_index = index;
        index++;
    }

    if( entries.size() == 0 )
    {
        phone_book->del_person( this );
    }
}


Phone_Book_Io_Driver::Phone_Book_Io_Driver(SRef<Library*> lib)
    : SPlugin( lib )
{
}

Phone_Book_Io_Driver::Phone_Book_Io_Driver()
    : SPlugin()
{
}

Phone_Book_Io_Registry::Phone_Book_Io_Registry()
{
    register_plugin( new Xml_Phone_Book_Io_Driver( NULL ) );
}
SRef<Phone_Book*> Phone_Book_Io_Registry::create_phone_book(const std::string &name)
{
    string driverId;
    string deviceId;

#ifdef DEBUG_OUTPUT
    my_dbg << "Phone_Book_Io_Registry: name =  " << name << endl;
#endif
    size_t pos = name.find( ':', 0 );
    if( pos == string::npos )
    {
        return NULL;
    }

    driverId = name.substr( 0, pos );

#ifdef DEBUG_OUTPUT
    my_dbg << "Phone_Book_Io_Registry: driverId =  " << driverId << endl;
#endif

    list< SRef<SPlugin*> >::iterator iter;
    list< SRef<SPlugin*> >::iterator stop = plugins.end();

    for( iter = plugins.begin(); iter != stop; iter++ )
    {
        SRef<SPlugin*> plugin = *iter;

        SRef<Phone_Book_Io_Driver*> driver = dynamic_cast<Phone_Book_Io_Driver*>(*plugin);

        if( !driver )
        {
            my_err << "Not a PhoneBookIoDriver? " << plugin->get_name() << endl;
        }

        if( driver && driver->get_prefix() == driverId )
        {
            SRef<Phone_Book_Io*> io =
                driver->create_phone_book_io( name );

            if( io ){
                return io->load();
            }
            else{
                my_dbg << "Phone_Book_Io_Registry: no io" << name << endl;
            }
        }
    }
    my_dbg << "Phone_Book_Io_Registry: device not found " << name << endl;
    return NULL;
}
