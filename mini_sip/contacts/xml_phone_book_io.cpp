#include "xml_phone_book_io.h"
#include "contact_db.h"
#include "xml_parser.h"

#include "string_utils.h"

#include<string>
#include<fstream>
#include<iostream>

using namespace std;

Xml_Phone_Book_Io::Xml_Phone_Book_Io( std::string fileName )
{
    if (fileName.substr(0,7)=="file://")
    {
        file_name = fileName.substr(7);
    }
}

void Xml_Phone_Book_Io::save( SRef< Phone_Book * > pb )
{
    Xml_File_Parser parser;

    list< SRef<Phone_Book_Person *> >::iterator iPerson;
    list< SRef<Phone_Book_Person *> >persons = pb->get_persons();
    list< SRef<Contact_Entry *> >::iterator iContact;
    list< SRef<Contact_Entry *> > contacts;
    string personPath, contactPath;
    int nPerson = 0;
    int nContact = 0;

    parser.change_value( "phonebook/name", pb->get_name() );

    for( iPerson = persons.begin(); iPerson != persons.end() ; iPerson++ )
    {
        personPath = "phonebook/contact[" + itoa(nPerson) + "]";

        parser.change_value( personPath+"/name", (*iPerson)->get_name() );

        nContact = 0;
        contacts = (*iPerson)->get_entries();
        for( iContact = contacts.begin(); iContact != contacts.end(); iContact ++ )
        {
            contactPath = personPath + "/pop[" + itoa( nContact ) + "]";
            parser.change_value( contactPath+"/desc", (*iContact)->get_desc() );
            parser.change_value( contactPath+"/uri", (*iContact)->get_uri() );
            nContact ++;
        }
        nPerson ++;
    }
    parser.save_to_file( fileName );
}

SRef< Phone_Book * > Xml_Phone_Book_Io::load()
{
    SRef<Phone_Book *> phonebook = new Phone_Book;
    Contact_Entry * entry;
    Xml_Parser * parser;

    if( file_name.empty() )
        return NULL;

    try{
        parser = new Xml_File_Parser( file_name );
    }

    catch( Xml_File_Not_Found& )
    {
        cerr << "Phonebook file not found. Creating default one." << endl;
        create_default();
        try{
            parser = new Xml_File_Parser( fileName );
        }
        catch( Xml_Exception & ){
            return NULL;
        }

    }
    if( parser == NULL ){
        return NULL;
    }

    string q = "phonebook/name";
    string name =  parser->get_value( q,"" );
    phonebook->set_name( name );

    if( name != "" )
    {
        string contact;
        int nContacts = 0;

        do{
            string q = "phonebook/contact["+ itoa( nContacts )+"]/name";
            contact = parser->get_value( q, "" );
            if( contact != "" )
            {
                SRef<Phone_Book_Person *> person
                        = new Phone_Book_Person( contact );

                phonebook->add_person( person );

                int nEntries = 0;
                string qbase;
                string desc;
                do{
                    string uri;
                    string qbase = "phonebook/contact[" +
                            itoa( nContacts ) +
                            "]/pop[" +
                            itoa( nEntries ) + "]/";

                    desc = parser->get_value( qbase+"desc","");
                    if( desc != "" )
                    {
                        uri = parser->get_value( qbase+"uri", "" );
                        entry = new Contact_Entry( uri, desc, person );
                        person->add_entry( entry );
                    }
                    nEntries ++;
                } while( desc != "" );

            }
            nContacts ++;
        } while( contact != "" );

    }

    delete parser;
    phonebook->set_io( this );
    return phonebook;
}

std::string Xml_Phone_Book_Io::get_phone_book_id()
{
    return "file://" + file_name;
}

std::string Xml_Phone_Book_Io::get_default_phone_book_string()
{
    string defaultPhonebook =
            //                "<version>" CONFIG_FILE_VERSION_REQUIRED_STR "</version>"
            "<phonebook name=Example>\n"
            "<contact name=\"Contact\">\n"
            "<pop desc=\"Phone\" uri=\"0000000000\"></pop>\n"
            "<pop desc=\"Laptop\" uri=\"sip:contact@minisip.org\"></pop>\n"
            "</contact>\n"
            "</phonebook>\n";
    return defaultPhonebook;
}

void Xml_Phone_Book_Io::create_default()
{
    ofstream phonebookFile( file_name.c_str() );

    phonebookFile << get_default_phone_book_string() << endl;
}

Xml_Phone_Book_Io_Driver::Xml_Phone_Book_Io_Driver( SRef<Library *> lib )
    : Phone_Book_Io_Driver( lib )
{
}

Xml_Phone_Book_Io_Driver::~Xml_Phone_Book_Io_Driver()
{

}
SRef<Phone_Book_Io*> Xml_Phone_Book_Io_Driver::create_phone_book_io(std::string name) const
{
    return new Xml_Phone_Book_Io( name );
}
