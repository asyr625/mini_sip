#include "ldap_phone_book_io.h"
#include "ldap_connection.h"
#include "contact_db.h"

#include<iostream>

using namespace std;

static std::list<std::string> pluginList;
static int initialized;

extern "C"
std::list<std::string> *ldappb_LTX_listPlugins( SRef<Library *> lib )
{
    if( !initialized )
    {
        pluginList.push_back("getPlugin");
        initialized = true;
    }
    return &pluginList;
}

extern "C"
SPlugin * ldappb_LTX_getPlugin( SRef<Library *> lib )
{
    return new Ldap_Phone_Book_Io_Driver( lib );
}

Ldap_Phone_Book_Io::Ldap_Phone_Book_Io( std::string fileName )
    : url( fileName )
{
    url.print_debug();
}

SRef<Phone_Book *> Ldap_Phone_Book_Io::load()
{
    if (!url.is_valid())
        return NULL;

    SRef<Phone_Book *> phonebook = new Phone_Book;
    SRef<Ldap_Connection*> conn = new Ldap_Connection(url.get_host(), url.get_port());

    std::vector<SRef<Ldap_Entry*> > entries;
    vector<string> attrs = url.get_attributes();

    phonebook->set_name( url.get_dn() );
    // FIXME handle empty attrs
    string label = attrs[0];

    entries = conn->find( url.get_dn(), url.get_filter(), attrs);

    std::vector<SRef<Ldap_Entry*> >::iterator i;
    for (i = entries.begin(); i != entries.end(); i++)
    {
        SRef<Phone_Book_Person *> person;
        SRef<Ldap_Entry*> entry = *i;
        string contact;

        try {
            contact = entry->get_attr_value_string(label);
        } catch( Ldap_Exception & ) {
            continue;
        }

        cerr << "Reading: " << contact << endl;
        person = new Phone_Book_Person( contact );

        vector<string> attrs = entry->get_attr_names();
        vector<string>::iterator j;
        int num_entries = 0;

        for (j = attrs.begin(); j != attrs.end(); j++)
        {
            const string &attr_name = *j;

            if (attr_name == label)
                continue;

            const string &value = entry->get_attr_value_string(attr_name);

            Contact_Entry * entry;
            entry = new Contact_Entry( value, attr_name, person );
            person->add_entry( entry );
            cerr << "Entry: " << attr_name << ":" << value << endl;
            num_entries++;
        }

        if( num_entries > 0 )
        {
            phonebook->add_person( person );
        }
    }
    phonebook->set_io( this );
    return phonebook;
}

void Ldap_Phone_Book_Io::save(SRef<Phone_Book *> pb)
{
    // Save unsupported
}

std::string Ldap_Phone_Book_Io::get_phone_book_id()
{
    return url.get_string();
}

Ldap_Phone_Book_Io_Driver::Ldap_Phone_Book_Io_Driver( SRef<Library *> lib )
    : Phone_Book_Io_Driver( lib )
{
}

Ldap_Phone_Book_Io_Driver::~Ldap_Phone_Book_Io_Driver()
{
}

SRef<Phone_Book_Io*> Ldap_Phone_Book_Io_Driver::create_phone_book_io(std::string name) const
{
    return new Ldap_Phone_Book_Io( name );
}
