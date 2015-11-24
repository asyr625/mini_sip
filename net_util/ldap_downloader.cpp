#include "ldap_downloader.h"
#include "ldap_url.h"
#include "ldap_credentials.h"

#include "string_utils.h"

#include <fstream>
Ldap_Downloader::Ldap_Downloader(std::string originalUrl)
{
    url = Ldap_Url(originalUrl);
    if (url.is_valid())
    {
        conn = SRef<Ldap_Connection*>(new Ldap_Connection(url.get_host(), url.get_port(), SRef<Ldap_Credentials*>(new Ldap_Credentials("", ""))));
    }
}

void Ldap_Downloader::fetch()
{
    if (url.is_valid())
    {
        try {
            std::vector<std::string> attrs = url.get_attributes();
            entries = conn->find(url.get_dn(), url.get_filter(), attrs, url.get_scope());
            is_loaded = true;
        } catch (Ldap_Not_Connected_Exception & /*e*/) {
        } catch (Ldap_Exception & /*e*/) {
        }
    }
}

std::string Ldap_Downloader::next_filename(std::string baseName, int num)
{
    std::string newName = baseName;
    size_t pos = 0;
    if (1 < num)
    {
        pos = baseName.find_last_of('.', baseName.length());
        if (pos == std::string::npos)
        {
            newName = baseName + "." + itoa(num);
        } else {
            newName = baseName.substr(0, pos) + "." + itoa(num) + baseName.substr(pos);
        }
    }
    return newName;
}

char* Ldap_Downloader::get_chars(int *length)
{
    if (!is_loaded)
        fetch();
    if (is_loaded)
    {
        if (entries.size() > 0)
        {
            SRef<Ldap_Entry*> entry = entries.at(0);
            std::vector<std::string> attrs = entry->get_attr_names();
            if (attrs.size() > 0)
            {
                /*
                 * First try if the attribute is a string attribute.
                 */
                try {
                    std::string firstAttrValue = entry->get_attr_value_string(attrs.at(0));

                    *length = (int)firstAttrValue.length();
                    char* res = new char[*length];
                    memcpy(res, firstAttrValue.c_str(), *length);
                    return res;
                } catch (Ldap_Attribute_Not_Found_Exception & /*e*/)
                {
                    /*
                     * Could not find the first named attribute amongst the string
                     * attributes. Try to find the same attribute name in the
                     * collection of binary attributes.
                     */
                    try {
                        std::vector< SRef<Ldap_Entry_Binary_Value*> > firstAttrValues = entry->get_attr_values_binary(attrs.at(0));
                        if (firstAttrValues.size() > 0)
                        {
                            *length = firstAttrValues.at(0)->length;
                            char* res = new char[*length];
                            memcpy(res, firstAttrValues.at(0)->value, *length);
                            return res;
                        }
                    } catch (Ldap_Attribute_Not_Found_Exception & /*e*/)
                    {
                        /*
                         * Could not find attribute in collection of binary attributes
                         * either. This is very odd and should be be able to happen!
                         */
                    }
                }
            }
        }
    }
    *length = 0;
    return NULL;
}

std::vector<std::string> Ldap_Downloader::save_to_files(std::string attr, std::string filenameBase, bool onlyFirst )
{
    std::vector<std::string> filenames;

    if (!is_loaded)
        fetch();
    if (is_loaded)
    {
        if (entries.size() > 0)
        {
            SRef<Ldap_Entry*> entry = entries.at(0);
            try {
                std::vector< SRef<Ldap_Entry_Binary_Value*> > binaryData = entry->get_attr_values_binary(attr);

                for (size_t i=0; i<binaryData.size(); i++)
                {
                    std::string fileName = next_filename(filenameBase, (int)i+1);
                    std::ofstream file(fileName.c_str());
                    if (file.good())
                    {
                        SRef<Ldap_Entry_Binary_Value*> val = binaryData.at(i);
                        file.write(val->value, val->length);
                        file.close();
                        filenames.push_back(fileName);
                    }
                    if (onlyFirst && i==0)
                        break;
                }
            } catch (Ldap_Attribute_Not_Found_Exception & /*e*/) {

            }
        }
    }
    return filenames;
}

std::string Ldap_Downloader::get_string(std::string attr) throw (Ldap_Attribute_Not_Found_Exception, Ldap_Exception)
{
    if (!is_loaded)
        fetch();
    if (is_loaded)
    {
        if (entries.size() > 0)
        {
            SRef<Ldap_Entry*> entry = entries.at(0);
            try {
                return entry->get_attr_value_string(attr);
            } catch (Ldap_Attribute_Not_Found_Exception & /*e*/) {
                throw;
            }
        }
    }
    throw Ldap_Exception("No entry fetched");
}

SRef<Ldap_Entry_Binary_Value*> Ldap_Downloader::get_binary(std::string attr) throw (Ldap_Attribute_Not_Found_Exception, Ldap_Exception)
{
    if (!is_loaded)
        fetch();
    if (is_loaded)
    {
        if (entries.size() > 0)
        {
            SRef<Ldap_Entry*> entry = entries.at(0);
            try {
                std::vector< SRef<Ldap_Entry_Binary_Value*> > vals = entry->get_attr_values_binary(attr);
                if (vals.size() > 0)
                {
                    return vals.at(0);
                }
            } catch (Ldap_Attribute_Not_Found_Exception & /*e*/) {
                throw;
            }
        }
    }
    throw Ldap_Exception("No entry fetched");
}
