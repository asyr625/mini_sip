#include "sip_message_content_mime.h"

#include "sip_message_content_factory.h"
#include "sip_message.h"

Sip_Message_Content_Mime::Sip_Message_Content_Mime(std::string t)
{
    this->_content_type = t;
    this->_boundry = "boun=_dry";
    this->_message = "";
    this->_unique_boundry = "_Minisip";
}

Sip_Message_Content_Mime::Sip_Message_Content_Mime(std::string content, std::string t)
{
    size_t index2;
    std::string cont;
    this->_unique_boundry = "_Minisip";
    if(t.substr(0,9) == "multipart")
    {
        this->_content_type = t.substr(0 , t.find(";",0));
        index2 = t.find(";boundary=",0);
        my_assert(index2 != std::string::npos);
        this->_boundry = t.substr(index2 + 10 , t.find(";",index2 + 10));

        this->_message = "";
        size_t index1 = 0;
        std::string part = "";

        // Find and add first bodypart
        part = "application/sdp";
        index1 = content.find(part, 0) + part.length() + 2; //hack ;)
        index2 = content.find("--"+this->_boundry, index1) - 4;

        cont = part;
        Sip_Message_Content_Factory_Func_Ptr contentFactory = Sip_Message::content_factories.get_factory(cont);
        add_part(contentFactory(content.substr(index1,index2-index1+1), cont));

//		cerr << endl << "part1=" << _content.substr(index1,index2-index1+1) << endl;

        // Find and add second bodypart
        part = "application/resource-lists+xml";
        index1 = content.find(part, 0) + part.length() + 2; //hack ;)
        index2 = content.rfind("--"+this->_boundry+"--", content.length()) - 2;

        cont = part;
        contentFactory = Sip_Message::content_factories.get_factory(cont);
        add_part(contentFactory(content.substr(index1,index2-index1+1), cont));

//		cerr << endl << "part2=" << _content.substr(index1,index2-index1+1) << endl;
    }
    else{
        this->_content_type = t;
        this->_message = content;
        this->_boundry = "";
    }
}

Sip_Message_Content_Mime::Sip_Message_Content_Mime(std::string t, std::string m, std::string b)
{
    this->_message = m;
    this->_content_type = t;
    this->_boundry = b;
    this->_unique_boundry = "_Minisip";
}


std::string Sip_Message_Content_Mime::get_string() const
{
    if(_content_type.substr(0,9) == "multipart")
    {
        std::list <SRef<Sip_Message_Content*> >::const_iterator iter;
        std::string mes;
        if(_message != "")
            mes = _message + "\r\n\r\n";
        if(_parts.empty())
            mes ="--" + _boundry + "\r\n\r\n";
        else
            for( iter = _parts.begin(); iter != _parts.end()  ; iter++ )
            {
                mes = mes + "--" + _boundry + "\r\n";
                mes = mes + "Content-type: " + (*iter)->get_content_type() + "\r\n\r\n";
                mes = mes + (*iter)->get_string() + "\r\n\r\n";
            }
        mes = mes + "--" + _boundry + "--" + "\r\n";
        return mes;
    }
    return _message;
}
std::string Sip_Message_Content_Mime::get_content_type() const
{
    if(_content_type.substr(0,9) == "multipart")
        return _content_type +"; boundary=" + _boundry;
    else
        return _content_type;
}

std::string Sip_Message_Content_Mime::get_content_type_without_parameters() const
{
    return _content_type;
}

void Sip_Message_Content_Mime::add_part(SRef<Sip_Message_Content*> part)
{
    if( (part->get_content_type()).substr(0,9) == "multipart")
        if(((Sip_Message_Content_Mime*)*part)->get_boundry() == _boundry)
        {
            ((Sip_Message_Content_Mime*)*part)->set_boundry(_boundry + _unique_boundry);
            _unique_boundry = _unique_boundry + "_Rules";
        }
    _parts.push_back(part);
}

int Sip_Message_Content_Mime::replace_part(SRef<Sip_Message_Content*> part)
{
    std::list<SRef<Sip_Message_Content*> >::const_iterator iter;
    for (iter = _parts.begin(); iter != _parts.end(); iter++)
    {
        if ((*iter)->get_mem_object_type() == part->get_mem_object_type())
        {
            _parts.remove(*iter);
            break;
        }
    }
    _parts.push_back(part);
    return _parts.size();
}

SRef<Sip_Message_Content*> Sip_Message_Content_Mime::pop_first_part()
{
    if(!_parts.empty())
    {
        SRef<Sip_Message_Content*> part = _parts.front();
        _parts.pop_front();
        return part;
    }
    else
        return NULL;
}

SRef<Sip_Message_Content*> Sip_Message_Content_Mime::get_part_by_type(std::string objectType)
{
    std::list<SRef<Sip_Message_Content*> >::const_iterator iter;
    for (iter = _parts.begin(); iter != _parts.end(); iter++)
    {
        if ((*iter)->get_mem_object_type() == objectType)
        {
            return (*iter);
        }
    }
    return NULL;
}

int Sip_Message_Content_Mime::remove_part_by_type(std::string objectType)
{
    std::list<SRef<Sip_Message_Content*> >::const_iterator iter;
    for (iter = _parts.begin(); iter != _parts.end(); iter++)
    {
        if ((*iter)->get_mem_object_type() == objectType)
        {
            _parts.remove(*iter);
            return _parts.size();
        }
    }
    return _parts.size();
}

void Sip_Message_Content_Mime::set_boundry(std::string boundry)
{
    _boundry = boundry;
}

std::string Sip_Message_Content_Mime::get_boundry()
{
    return _boundry;
}
