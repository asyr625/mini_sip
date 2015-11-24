#include "ldap_entry.h"
#include "ldap_exception.h"

#ifdef ENABLE_LDAP
#ifdef _MSC_VER
#include <windows.h>
#include <winldap.h>
#include <winber.h>
#else
// FIXME: We use the deprecated API. This should be updated to the newest
// one.
#define LDAP_DEPRECATED 1
#include <ldap.h>
#include <lber.h>
#endif
#endif

Ldap_Entry::Ldap_Entry(void* ld, void* entry)
{
#ifdef ENABLE_LDAP
    BerElement* ber;
    struct berval** binaries;
    char* attr;
    char** strings;
    int i=0;

    values_binary.clear();
    values_strings.clear();

    for( attr = ldap_first_attribute((LDAP*)ld, (LDAPMessage*)entry, &ber); attr != NULL; attr = ldap_next_attribute((LDAP*)ld, (LDAPMessage*)entry, ber))
    {

        std::string attrName(attr);

        // Separate binary attributes from the rest simply to testing what the returned attribute is called. Simple and effective.
        if (string_ends_with(attr, ";binary"))
        {
            // Process binary attributes
            if ((binaries = ldap_get_values_len((LDAP*)ld, (LDAPMessage*)entry, attr)) != NULL)
            {
                for (i=0; binaries[i] != NULL; i++)
                {
                    values_binary[attrName].push_back(SRef<Ldap_Entry_Binary_Value*>(new Ldap_Entry_Binary_Value(binaries[i]->bv_val, binaries[i]->bv_len)));
                }
                ldap_value_free_len(binaries);
            }
        } else {
            // Process string attributes
            if ((strings = ldap_get_values((LDAP*)ld, (LDAPMessage*)entry, attr)) != NULL)
            {
                for (i = 0; strings[i] != NULL; i++)
                {
                    values_strings[attrName].push_back(std::string(strings[i]));
                }
                ldap_value_free(strings);
            }
        }
        ldap_memfree(attr);
    }
    if (ber != NULL)
    {
        ber_free(ber,0);
    }
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}

bool Ldap_Entry::has_attribute(std::string attr)
{
#ifdef ENABLE_LDAP
    std::map<std::string, std::vector<SRef<Ldap_Entry_Binary_Value*> > >::iterator binaryIter = values_binary.find(attr);
    std::map<std::string, std::vector<std::string> >::iterator stringIter = values_strings.find(attr);

    if( binaryIter != values_binary.end() || stringIter != values_strings.end() ) {
        return true;
    }

    return false;
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}

std::string Ldap_Entry::get_attr_value_string(std::string attr) throw (Ldap_Attribute_Not_Found_Exception)
{
#ifdef ENABLE_LDAP
    std::map<std::string, std::vector<std::string> >::iterator stringIter = values_strings.find(attr);

    if (stringIter != values_strings.end()) {
        return values_strings[attr].at(0);
    }

    throw Ldap_Attribute_Not_Found_Exception(attr);
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}


std::vector<std::string> Ldap_Entry::get_attr_values_strings(std::string attr) throw (Ldap_Attribute_Not_Found_Exception)
{
#ifdef ENABLE_LDAP
    std::map<std::string, std::vector<std::string> >::iterator i = values_strings.find(attr);

    if (i != values_strings.end()) {
        return values_strings[attr];
    }

    throw Ldap_Attribute_Not_Found_Exception(attr);
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}



std::vector< SRef<Ldap_Entry_Binary_Value*> > Ldap_Entry::get_attr_values_binary(std::string attr) throw (Ldap_Attribute_Not_Found_Exception)
{
#ifdef ENABLE_LDAP
    std::map<std::string, std::vector<SRef<Ldap_Entry_Binary_Value*> > >::iterator i = values_binary.find(attr);

    if (i != values_binary.end()) {
        return values_binary[attr];
    }

    throw Ldap_Attribute_Not_Found_Exception(attr);

#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}

std::vector< SRef<Ldap_Entry_Binary_Value*> > Ldap_Entry::get_attr_values_binary_pairs(std::string attr) throw (Ldap_Attribute_Not_Found_Exception)
{
#ifdef ENABLE_LDAP
    std::vector< SRef<Ldap_Entry_Binary_Value*> > rawBinary;
    std::vector< SRef<Ldap_Entry_Binary_Value*> >::iterator rawIter;
    std::vector< SRef<Ldap_Entry_Binary_Pair_Value*> > result;

    try {
        rawBinary = get_attr_values_binary(attr);
        if (rawBinary.size() > 0)
        {
            for (rawIter = rawBinary.begin(); rawIter != rawBinary.end(); rawIter++)
            {

                int len = (*rawIter)->length;
                char* dataPair = new char[len];
                memcpy(dataPair, (*rawIter)->value, len);

                BerElement *berPair;
                struct berval *bervalPair;


                bervalPair = new struct berval;
                bervalPair->bv_val = dataPair;
                bervalPair->bv_len = len;

                berPair = ber_init(bervalPair);

                struct berval *bervalCertIssuedTo, *bervalCertIssuedBy;
                bervalCertIssuedTo = new struct berval;
                bervalCertIssuedBy = new struct berval;

                ber_scanf(berPair, "{oo}", bervalCertIssuedTo, bervalCertIssuedBy);

                /*
                std::cout << "Analysis of certificate-pair file generated for inclusion in directory of " << params->shortNameA << ":" << std::endl;
                std::cout << "  Length of certificate in issued-TO-this-CA field: " << bervalCaACertIssuedTo->bv_len << std::endl;
                std::cout << "  Length of certificate in issued-BY-this-CA field: " << bervalCaACertIssuedBy->bv_len << std::endl;

                std::cout << "Analysis of certificate-pair file generated for inclusion in directory of " << params->shortNameB << ":" << std::endl;
                std::cout << "  Length of certificate in issued-TO-this-CA field: " << bervalCaBCertIssuedTo->bv_len << std::endl;
                std::cout << "  Length of certificate in issued-BY-this-CA field: " << bervalCaBCertIssuedBy->bv_len << std::endl;
                */
                char* dataCertIssuedTo = new char[bervalCertIssuedTo->bv_len];
                char* dataCertIssuedBy = new char[bervalCertIssuedBy->bv_len];
                memcpy(dataCertIssuedTo, bervalCertIssuedTo->bv_val, bervalCertIssuedTo->bv_len);
                memcpy(dataCertIssuedBy, bervalCertIssuedBy->bv_val, bervalCertIssuedBy->bv_len);

                SRef<Ldap_Entry_Binary_Value*> first(new Ldap_Entry_Binary_Value(dataCertIssuedTo, bervalCertIssuedTo->bv_len));
                SRef<Ldap_Entry_Binary_Value*> second(new Ldap_Entry_Binary_Value(dataCertIssuedBy, bervalCertIssuedBy->bv_len));

                result.push_back(SRef<Ldap_Entry_Binary_Pair_Value*>(new Ldap_Entry_Binary_Pair_Value(first, second)));

                ber_bvfree(bervalPair);
                ber_bvfree(bervalCertIssuedTo);
                ber_bvfree(bervalCertIssuedBy);
                /*
                delete bervalPair;
                delete bervalCertIssuedTo;
                delete bervalCertIssuedBy;
                */
                ber_free(berPair, 1);
            }
        }
    }
    catch (Ldap_Attribute_Not_Found_Exception & /*ex*/)
    {
        throw; // Re-throw exception
    }
    return result;
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}

std::vector<std::string> Ldap_Entry::get_attr_names()
{
#ifdef ENABLE_LDAP
    std::vector<std::string> res;
    std::map<std::string, std::vector<std::string> >::iterator i;
    std::map<std::string, std::vector<SRef<Ldap_Entry_Binary_Value*> > >::iterator j;

    for (i = values_strings.begin(); i != values_strings.end(); i++)
        res.push_back(i->first);

    for (j = values_binary.begin(); j != values_binary.end(); j++)
        res.push_back(j->first);

    return res;
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}
