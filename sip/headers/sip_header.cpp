#include <vector>

#include "sip_header.h"
#include "string_utils.h"
#include "sip_header_unknown.h"

Sip_Header_Factories Sip_Header::header_factories = Sip_Header_Factories();

Sip_Header_Parameter::Sip_Header_Parameter(std::string parse_from)
{
    std::vector<std::string> key_val = split(parse_from,true,'=');
    _key = key_val[0];
    _has_equal = false;
    if (key_val.size()==2)
    {
        _value = key_val[1];
        _has_equal = true;
    }
}

Sip_Header_Parameter::Sip_Header_Parameter(std::string key, std::string value, bool hasEqual)
    :_key(key), _value(value), _has_equal(hasEqual)
{
}

std::string Sip_Header_Parameter::get_key() const
{
    return _key;
}

std::string Sip_Header_Parameter::get_value() const
{
    return _value;
}
void Sip_Header_Parameter::set_value(std::string v)
{
    _value = v;
}

std::string Sip_Header_Parameter::get_string() const
{
    if (_has_equal || _value.size()>0)
    {
        return _key + "=" + _value;
    }
    else
    {
        return _key;
    }
}

void Sip_Header_Factories::add_factory(std::string headerType, Sip_Header_Factory_Func_Ptr f)
{
    std::string ht;
    for (unsigned i = 0; i < headerType.size(); i++)
    {
        ht += toupper(headerType[i]);
    }
    _factories[ht] = f;
}

Sip_Header_Factory_Func_Ptr Sip_Header_Factories::get_factory(std::string headerType) const
{
    std::string ht;
    for (unsigned i=0; i< headerType.size();i++)
    {
        ht += toupper(headerType[i]);
    }

    std::map<std::string, Sip_Header_Factory_Func_Ptr >::const_iterator res;
    res = _factories.find(ht);
    if( res != _factories.end())
    {
        return (*_factories.find(ht)).second;
    }else
    {
        return NULL;
    }
}

Sip_Header_Value::Sip_Header_Value(int type, const std::string &hName)
    : header_name(hName), _type(type)
{
}

void Sip_Header_Value::set_parameter(std::string key, std::string val)
{
    if( val.size() > 0 )
    {
        SRef<Sip_Header_Parameter*> param = new Sip_Header_Parameter(key,val,true);
        add_parameter(param);
    }
    else
    {
        remove_parameter(key);
    }
}

void Sip_Header_Value::add_parameter(SRef<Sip_Header_Parameter*> p)
{
    for (int i=0; i< _parameters.size();i++)
    {
        if (_parameters[i]->get_key() == p->get_key())
        {
            _parameters[i]->set_value(p->get_value());
            //cerr<<"p->getValue() "+p->getValue()<<endl;
            return;
        }
    }
    _parameters.push_back(p);
}

bool Sip_Header_Value::has_parameter(const std::string &key) const
{
    for (int i=0; i< _parameters.size();i++)
    {
        if (_parameters[i]->get_key()==key)
        {
            return true;
        }
    }
    return false;
}

std::string Sip_Header_Value::get_parameter(std::string key) const
{
    for (int i=0; i< _parameters.size();i++)
    {
        if (_parameters[i]->get_key()==key)
        {
            return _parameters[i]->get_value();
        }
    }
    return "";
}

void Sip_Header_Value::remove_parameter(std::string key)
{
    for (int i=0; i< _parameters.size(); i++)
    {
        if (_parameters[i]->get_key()==key)
        {
            _parameters.remove(i);
            i=0;
        }
    }
}

std::string Sip_Header_Value::get_string_with_parameters() const
{
    std::string parameterList;
    int nparam = _parameters.size();
    for (int i=0; i< nparam; i++){
        if( i == 0 )
            parameterList += get_first_parameter_separator();
        else
            parameterList += get_parameter_separator();
        parameterList += _parameters[i]->get_string();
    }

    return get_string() + parameterList;
}



Sip_Header::Sip_Header(SRef<Sip_Header_Value*> value)
    : _header_name(value->header_name)
{
    _type = value->get_type();
    _header_values.push_back(value);
}

Sip_Header::~Sip_Header()
{
}

std::string Sip_Header::get_string() const
{
    std::string ret = _header_name +": ";
    bool first = true;
    for (int i = 0; i< _header_values.size(); i++)
    {
        if (!first)
        {
            ret += ",";
        }
        else
        {
            first = false;
        }
        ret += _header_values[i]->get_string_with_parameters();
    }
    return ret;
}
void Sip_Header::add_header_value(SRef<Sip_Header_Value*> v)
{
    my_assert(_type == v->get_type());
    _header_values.push_back(v);
}

int32_t Sip_Header::get_type() const
{
    return _type;
}
int Sip_Header::get_no_values() const
{
    return _header_values.size();
}

SRef<Sip_Header_Value *> Sip_Header::get_header_value(int i) const
{
    my_assert(i < _header_values.size() );
    return _header_values[i];
}

void Sip_Header::remove_header_value(int i)
{
    my_assert(i < _header_values.size() );
    _header_values.remove(i);
}

static std::string get_header(const std::string &line,int &endi)
{
    std::string ret;
    int i;
    for (i=0; (i<(int)line.size()) && line[i]!=' ' && line[i]!='\t' && line[i]!=':'   ;  i++)
    {
        ret+=line[i];
    }
    while (line[i]==' ' || line[i]=='\t' || line[i]==':')
    {
        i++;
    }
    endi = i;
    return ret;
}


std::string findHeaderType(std::string s)
{
    std::string ht;
    int ssize = (int)s.size();
    for (int i=0; s[i] != ':' && i < ssize; i++)
    {
        ht += s[i];
    }
    return trim(ht);
}

SRef<Sip_Header *> Sip_Header::parse_header(const std::string &line)
{
    int hdrstart = 0;
    SRef<Sip_Header*> h;

    std::string hdr = get_header(line,hdrstart);
    //cerr<< endl << "PARSEHDR: Sip Header: header"+hdr<<endl;
    std::string valueline = line.substr(hdrstart);

    //cerr << "PARSEHDR: hdr parsed to <"<< hdr <<">"<< endl;
    //cerr << "PARSEHDR: valueline parsed to "<< valueline<< endl;

    std::string headerType = findHeaderType(line);
    std::string ht = up_case(headerType);
    char valueSeparator;
    char paramSeparator;

    if( ht == "WWW-AUTHENTICATE" ||
            ht == "PROXY-AUTHENTICATE" ||
            ht == "AUTHORIZATION" ||
            ht == "PROXY-AUTHORIZATION" ||
            ht == "AUTHENTICATION-INFO" ) {
        valueSeparator = '\0';
        paramSeparator = ',';
    } else {
        valueSeparator = ',';
        paramSeparator = ';';
    }

    std::vector<std::string> values = split(valueline,true, valueSeparator);

    for (unsigned i=0; i< values.size(); i++)
    {
        std::vector<std::string> value_params;
        std::string value_zero; //value zero is the non-parameter part of the header
        if( values[i].size() == 0 )
        { //an empty value??? BUG!
            // 				cerr << "BUGBUGBUG: Sip_Header::parseHeader : Found Empty header value in Sip message!:: " << valueline << endl;
            continue;
        }
        // 			cerr << "PARSER: First value+params line: "<< values[i]<<""<<endl;

        if( ht == "ACCEPT-CONTACT" )
        {
            value_params = split(values[i],true,'\n');
            if (value_params.size()>0)
                value_zero = value_params[0];
            // 				cerr<<"valueline.substr(2): "+valueline.substr(2)<<endl;
        }
        else if( ht == "CONTACT" )
        {
            size_t ltPos = values[i].find( '<' );
            size_t gtPos = values[i].find( '>' );
            if( ltPos!=std::string::npos && gtPos != std::string::npos )
            {
                //if the string contains '<' and '>' ...
                //remove them
                value_zero = values[i].substr( ltPos + 1, gtPos - ltPos - 1 );
                //now split the outside parameters ...
                value_params = split(
                            //note that it should be gtPos -1, but we need value_params[0] to be
                            //not a param ...so we take some junk from the uri previous to the first ;
                            values[i].substr( gtPos - 1, values[i].size() - (gtPos - 1) ),
                            true,
                            ';' );
            } else { //if there is no < or >, then just split into parameters ...
                value_params = split(values[i],true,';');
                if (value_params.size()>0)
                    value_zero = value_params[0];
            }
        }
        else if( ht == "FROM" || ht == "TO" || ht == "ROUTE" || ht == "RECORD-ROUTE" )
        {
            size_t ltPos = values[i].find( '<' );
            size_t gtPos = values[i].find( '>' );
            if( ltPos!=std::string::npos && gtPos!=std::string::npos )
            {
                value_zero = values[i].substr( 0, gtPos + 1 );
                //now split the outside parameters ...
                value_params = split(
                            //note that it should be gtPos -1, but we need value_params[0] to be
                            //not a param ...so we take some junk from the uri previous to the first ;
                            values[i].substr( gtPos - 1, values[i].size() - (gtPos - 1) ),
                            true,
                            ';' );
            } else { //if there is no < or >, then just split into parameters ...
                value_params = split(values[i],true,';');
                if (value_params.size()>0)
                    value_zero = value_params[0];
            }
        }
        else
        {
            value_params = split(values[i],true,paramSeparator);
            if (value_params.size()>0)
                value_zero = value_params[0];
        }

        //		cerr << "PARSER: Header type is: "<< headerType << endl;
        //		cerr << "PARSER: Creating value from string: "<< value_zero <<endl;
        Sip_Header_Factory_Func_Ptr factory;
        factory = Sip_Header::header_factories.get_factory(headerType);
        SRef<Sip_Header_Value *> hval;
        if (factory)
        {
            hval = factory(value_zero);
        }
        else
        {
            hval = new Sip_Header_Value_Unknown(headerType, value_zero);
        }

        for(unsigned j=1; j<value_params.size(); j++)
        {
            hval->add_parameter(new Sip_Header_Parameter(value_params[j]));
        }
        if ( i == 0 )
        {
            h= new Sip_Header(hval);
        }
        else
        {
            h->add_header_value(hval);
        }
    }

    if (!h)
    {
        //Special case for headers that are allowed to exist
        //without any header value. This list is compatibel
        //with: RFC3261, RFC3262(none)
        if ( 		ht=="ACCEPT" ||
                    ht=="ACCEPT-ENCODING" ||
                    ht=="ACCEPT-LANGUAGE" ||
                    ht=="ALLOW" ||
                    ht=="ORGANIZATION" ||
                    ht=="SUBJECT" ||
                    ht=="SUPPORTED") {
            Sip_Header_Factory_Func_Ptr factory;
            factory = Sip_Header::header_factories.get_factory(headerType);
            SRef<Sip_Header_Value *> hval;
            if (factory)
            {
                hval = factory("");
            }
            else
            {
                hval = new Sip_Header_Value_Unknown(headerType, "");
            }
            h= new Sip_Header(hval);
        }
    }
    return h;
}
