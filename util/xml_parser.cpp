#include "xml_parser.h"
#include "string_utils.h"

#include<fstream>
#include<iostream>
#include<sys/types.h>
#include <stdlib.h>

using namespace std;

static bool is_blank(char c)
{
    if (c=='\t' || c==' ' || c=='\r' || c=='\n')
        return true;
    else
        return false;
}

static int32_t skipws(const char *s, int32_t i)
{
    while (is_blank(s[i]))
        i++;
    return i;
}

static string parse_word(const char *s, int32_t &i)
{
    i = skipws(s,i);
    string word;

    if (s[i]=='\"')
    {
        i++;
        while (s[i]!='\"')
            word = word + s[i++];
        i++;
    }else{
        while ( ! ( is_blank(s[i]) || s[i]=='<' || s[i]=='/' || s[i]=='>' || s[i]=='=' || s[i]=='\"' || s[i]==0))
            word+=s[i++];
    }
    return word;
}


Xml_Parser::Xml_Parser(Xml_Parser_Callback *cb)
    :callback(cb),root(NULL)
{
}

Xml_Parser::~Xml_Parser()
{
    if (root)
        delete root;
}

void Xml_Parser::change_value(string elementPath, string value, bool addIfMissing)
{
    Xml_Node *n = Xml_Node::get_node(root, elementPath.c_str(), 0);
    if (n!=NULL)
        n->set_value(value);
    else{
        if (addIfMissing)
            add_value(root, elementPath.c_str(), value);
    }
}

void Xml_Parser::remove(std::string path)
{
    Xml_Node::remove_node( root, NULL, path.c_str(), 0);
}


void Xml_Parser::add_value(string path, string value)
{
    add_value(root,path.c_str(),value,0);
}

void Xml_Parser::add_value(Xml_Node *cur, const char *path, string &value, int32_t i)
{
    if (path[i]=='/')
        i++;
    if (path[i]==0)
    {
        cur->set_value(value);
        return;
    }

    int32_t index=0;
    string part = parse_word(path, i);
    if (part[part.length()-1]==']')
    {
        string sindex;
        int32_t ii=(int32_t)part.length()-2;
        while (part[ii]!='[')
        {
            sindex= part[ii]+sindex;
            ii--;
            if (i==0)
                throw Xml_Exception("Parse error in key: Missing '['?");
        }
        part = part.substr(0,ii);
        index = atoi(sindex.c_str());
    }

    for (list<Xml_Node *>::iterator itt = cur->subnodes.begin(); itt!= cur->subnodes.end(); itt++)
    {
        if ( (*itt)->get_name()==part )
            if ((index--)==0)
            {
                add_value( *itt, path, value, i);
                return;
            }
    }
    Xml_Node *newnode = new Xml_Element(part);
    cur->add_node(newnode);
    add_value(newnode, path, value, i/*+part.length()*/);
    return;
}

string Xml_Parser::get_value(string path)
{
    Xml_Node *cur = root;

    cur = Xml_Node::get_node(root, path.c_str(), 0);
    if (cur==NULL)
        throw Xml_Element_Not_Found("Element does not exist: "+path);
    //	throw new Xml_ElementNotFound("Element does not exist: "+path);
    return cur->get_value();
}

string Xml_Parser::get_value(string path, string defaultValue)
{
    Xml_Node *cur = root;

    cur = Xml_Node::get_node(root, path.c_str(), 0);
    if (cur==NULL)
        return defaultValue;
    return cur->get_value();
}

int32_t Xml_Parser::get_int_value(string path)
{
    return atoi(get_value(path).c_str());
}

int32_t Xml_Parser::get_int_value(string path, int32_t defaultValue)
{
    return atoi(get_value(path, itoa(defaultValue)).c_str());
}


string Xml_Node::generate_string(int32_t indent, Xml_Node *cur)
{
    string ret;
    int32_t j;
    for (j=0; j<indent; j++)
        ret = ret+"\t";
    if (indent>=0)
        ret=ret+"<"+cur->get_name();

    for (list<Xml_Node *>::iterator i=cur->subnodes.begin(); i!=cur->subnodes.end(); i++)
    {
        if ((*i)->get_type()==XML_NODE_TYPE_ATTRIBUTE)
        {
            ret=ret+" "+(*i)->get_name()+"=\""+(*i)->get_value()+"\"";
        }
    }
    if (indent>=0)
        ret = ret+">\n";
    if (cur->get_value().length()>0)
    {
        for (j=0; j<indent+1; j++)
            ret = ret+"\t";
        ret = ret +cur->get_value()+"";
        ret = ret+'\n';
    }
    bool hassub=false;
    for (list<Xml_Node *>::iterator itt=cur->subnodes.begin(); itt!=cur->subnodes.end(); itt++)
    {
        if ((*itt)->get_type()==XML_NODE_TYPE_ELEMENT)
        {
            ret = ret + generate_string(indent+1, *itt);
            hassub = true;
        }
    }
    for (j=0; j<indent; j++)
        ret = ret+"\t";

    if (indent>=0)
        ret = ret +"</"+cur->get_name()+">\n";

    return ret;
}

string Xml_Parser::xml_string()
{
    return Xml_Node::generate_string(-1, root);
}

Xml_File_Parser::Xml_File_Parser(string filename_, Xml_Parser_Callback *cb)
    : Xml_Parser(cb),
      filename(filename_)
{

    fs = new Local_File_System();
    init();
}

Xml_File_Parser::Xml_File_Parser(string filename_, SRef<File_System*> fs_, Xml_Parser_Callback *cb)
    : Xml_Parser(cb),
      filename(filename_), fs(fs_)
{
    init();
}

void Xml_File_Parser::init()
{
    string s = "";
    if (filename != "")
    {
        SRef<File*> file;
        try{
            file = fs->open(filename);
        }catch(const File_Exception &e){
            throw Xml_File_Not_Found( "Could not read file " + filename );
        }

        int32_t bufsize = 20;
        char *buf = (char *)calloc(bufsize,1);
        do{
            for (int32_t i=0; i<bufsize; i++)
                buf[i]=0;
            //file.read(buf,bufsize-1);
            file->read(buf, bufsize-1);
            s = s+string(buf);
        }while(!file->eof());
        free(buf);
    }
    parse_string(s);
}
