#ifndef XML_PARSER_H
#define XML_PARSER_H

#include "my_types.h"
#include "sobject.h"
#include "exception.h"
#include "file_system.h"

#include<list>

#ifdef DEBUG_OUTPUT
#include<iostream>
#endif

#define XML_NODE_TYPE_ROOT	 	0
#define XML_NODE_TYPE_ATTRIBUTE 	1
#define XML_NODE_TYPE_ELEMENT		2

#include<string>

/*
In the following example "name" is a attribute and FILE is an element

<SETTINGS>
  <PROJECT name="tstproject">		 get(SETTINGS/PROJECT/name) -> tstproject
    <FILE name="tstfile1.txt" type="1"/> get(SETTINGS/PROJECT/FILE/type) -> 1
    <FILE name="tstfile1.txt" type="" /> get(SETTINGS/PROJECT/FILE[1]/name) -> tstfile1.txt
    <text>
        Testtext			 getEnclosed(SETTINGS/PROJECT/text) -> Testtext
    </text>
  </PROJECT>
</SETTINGS>
*/


class Xml_Exception : public Exception
{
public:
    Xml_Exception(std::string m) : Exception(m.c_str()) {}
};

class Xml_Element_Not_Found : public Xml_Exception
{
public:
    Xml_Element_Not_Found(std::string m) : Xml_Exception(m) {}
};

class Xml_File_Not_Found : public Xml_Exception
{
public:
    Xml_File_Not_Found(std::string m) : Xml_Exception(m) {}
};

class Xml_Parser_Callback
{
public:
    virtual ~Xml_Parser_Callback() {}
    virtual bool parsed_element(std::string path, std::string enclosedText) = 0;
    virtual bool parsed_attribute(std::string path, std::string value) = 0;
};

class Xml_Node;

class Xml_Parser : public SObject
{
public:
    Xml_Parser(Xml_Parser_Callback *cb=NULL);
    ~Xml_Parser();

    void remove(std::string path);

    std::string get_value(std::string path);
    std::string get_value(std::string path, std::string defaultValue);

    int32_t get_int_value(std::string path);
    int32_t get_int_value(std::string path, int32_t defaultValue);

    void print();

    std::string xml_string();

    void add_value(std::string elementPath, std::string value);
    void change_value(std::string elemPath, std::string value, bool addIfMissing = true);

    std::string get_mem_object_type(){return "XMLParser";}

protected:
    void add_value(Xml_Node *root, const char *elementPath, std::string &value, int32_t start = 0);
    void parse_string(const std::string &s);

    Xml_Parser_Callback *callback;
    Xml_Node *root;
};

class Xml_File_Parser : public Xml_Parser
{
public:
    Xml_File_Parser(std::string filename="", Xml_Parser_Callback *cb = NULL);
    Xml_File_Parser(std::string filename, SRef<File_System*> fs, Xml_Parser_Callback *cb = NULL);
    void save_to_file(std::string file="");
private:
    void init();

    std::string filename;
    SRef<File_System*> fs;
};

class Xml_String_Parser : public Xml_Parser
{
public:
    Xml_String_Parser(const std::string &s, Xml_Parser_Callback *cb = NULL);
};

class Xml_Node
{
public:
    Xml_Node(int32_t type, std::string name, std::string value="");
    virtual ~Xml_Node();

    int32_t get_type() {return type;}

    void add_node(Xml_Node *node);

    virtual void print() {}

    std::list<Xml_Node *>& get_nodes();
    std::string get_name() {return name;}
    std::string get_value(){return value;}
    void set_value(std::string v) { value = v;}

    static Xml_Node *get_node(Xml_Node *searchNode, const char *path, int32_t start);
    static void remove_node(Xml_Node *searchNode, Xml_Node *parent, const char *path, int32_t start);

    static std::string generate_string(int32_t index, Xml_Node *cur);

    std::list<Xml_Node *> subnodes;
protected:
    std::string name;
    std::string value;
    int32_t type;
};

class Xml_Attribute : public Xml_Node
{
public:
    Xml_Attribute(std::string name, std::string value);
    ~Xml_Attribute();
};

class Xml_Element : public Xml_Node
{
public:
    Xml_Element(std::string name);
    ~Xml_Element();
    std::string get_enclosed();
    void set_enclosed(std::string encl);
};
#endif // XML_PARSER_H
