#ifndef CERT_H
#define CERT_H


#include "sobject.h"
#include "cache_item.h"
#include "exception.h"
#include "mutex.h"
#include "cache_item.h"

#include<string>
#include<list>
#include<vector>

#ifdef _MSC_VER
#pragma warning (disable: 4251)
#endif

class Certificate;

class Certificate_Pair : public SObject
{
public:
    Certificate_Pair();
    Certificate_Pair(SRef<Certificate*> issuedToThisCA);
    Certificate_Pair(SRef<Certificate*> issuedToThisCA, SRef<Certificate*> issuedByThisCA);
    SRef<Certificate*> issuedToThisCA;
    SRef<Certificate*> issuedByThisCA;
};

class Certificate_Set_Item : public Cache_Item
{
public:
    enum CERTSETITEM_IMPORTMETHOD
    {
        IMPORTMETHOD_OTHER = 0,
        IMPORTMETHOD_FILE = 1,
        IMPORTMETHOD_DIRECTORY = 2
    };

    virtual ~Certificate_Set_Item();

    Certificate_Set_Item();
    Certificate_Set_Item(std::string certUri);
    Certificate_Set_Item(SRef<Certificate*> cert);

    std::string get_subject() const;
    std::vector<std::string> get_subject_alt_names() const;
    std::string get_subject_key_identifier() const;

    std::string get_issuer() const;
    std::vector<std::string> get_issuer_alt_names() const;
    std::string get_issuer_key_identifier() const;

    bool is_self_signed() const;

    std::string get_certificate_uri() const;
    SRef<Certificate*> get_certificate();

    CERTSETITEM_IMPORTMETHOD get_import_method() const;
    std::string get_import_parameter() const;


    void set_certificate(const SRef<Certificate*> cert);
    void set_certificate_uri(const std::string);

    void set_import_method(const CERTSETITEM_IMPORTMETHOD type);
    void set_import_parameter(const std::string param);

    void load_cert_and_index();
    void unload_certificate_from_memory();

    bool operator ==(const Certificate_Set_Item item2)
    {
        return ( item2.get_import_method() == get_import_method() && item2.get_import_parameter() == get_import_parameter());
    }

    void reindex_cert();

private:
    std::string subject;
    std::vector<std::string> subjectAltNames;
    std::string subjectKeyIdentifier;

    std::string issuer;
    std::vector<std::string> issuerAltNames;
    std::string issuerKeyIdentifier;

    bool selfSigned;

    std::string certificateUri;
    SRef<Certificate*> certificate;

    CERTSETITEM_IMPORTMETHOD importMethod;

    std::string importParameter;
};

class Certificate_Set : public SObject
{
public:
    virtual ~Certificate_Set();
    static Certificate_Set *create();

    virtual Certificate_Set* clone();

    void add_directory( std::string dir );
    SRef<Certificate_Set_Item*> add_file( std::string file );
    virtual SRef<Certificate_Set_Item*> add_certificate( SRef<Certificate *> cert );
    std::vector<SRef<Certificate_Set_Item*> > find_items(const std::string searchFor, const std::string issuer = "");

    virtual std::list<SRef<Certificate_Set_Item*> > &get_items();
    virtual SRef<Certificate_Set_Item*> get_next();
    virtual void init_index();
    virtual void lock();
    virtual void unlock();

    virtual void remove( SRef<Certificate_Set_Item*> removedItem );

protected:
    Certificate_Set();
    virtual void add_item( SRef<Certificate_Set_Item*> item );
    virtual SRef<Certificate_Set_Item*> create_cert_item( SRef<Certificate*> cert );

private:
    std::list<SRef<Certificate_Set_Item*> >::iterator items_index;
    std::list<SRef<Certificate_Set_Item*> > items;
    Mutex mLock;
};


class Private_Key : public SObject
{
public:
    static Private_Key* load( const std::string private_key_filename );
    static Private_Key* load( char *derEncPk, int length,
                              std::string password,
                              std::string path );

    virtual ~Private_Key();

    virtual const std::string &get_file() const = 0;

    virtual bool check_cert( Certificate * cert) = 0;

    virtual int sign_data( unsigned char * data, int data_length, unsigned char * sign, int * sign_length ) = 0;

    virtual int denvelope_data( unsigned char * data, int size, unsigned char *retdata, int *retsize,
                                unsigned char *enckey, int enckeylgth, unsigned char *iv) = 0;

    virtual bool private_decrypt(const unsigned char *data, int size, unsigned char *retdata, int *retsize)=0;
protected:
    Private_Key();
};

class Certificate : public SObject
{
public:
    enum SubjectAltName
    {
        SAN_DNSNAME = 1,
        SAN_RFC822NAME,
        SAN_URI,
        SAN_IPADDRESS
    };

    static Certificate* load( const std::string cert_filename );
    static Certificate* load( const std::string cert_filename, const std::string private_key_filename );
    static Certificate* load( unsigned char * der_cert, int length );
    static Certificate* load( unsigned char * certData, int length, std::string path );
    virtual ~Certificate();


    virtual int control( Certificate_Set * cert_db ) = 0;

    virtual int get_der_length() = 0;
    virtual void get_der( unsigned char * output, unsigned int * length ) = 0;

    virtual int envelope_data( unsigned char * data, int size, unsigned char *retdata, int *retsize,
                              unsigned char *enckey, int *enckeylgth, unsigned char** iv) = 0;

    int denvelope_data( unsigned char * data, int size, unsigned char *retdata, int *retsize,
                       unsigned char *enckey, int enckeylgth, unsigned char *iv);

    int sign_data( unsigned char * data, int data_length, unsigned char * sign, int * sign_length );
    virtual int verif_sign( unsigned char * data, int data_length, unsigned char * sign, int sign_length ) = 0;

    virtual bool public_encrypt(const unsigned char *data, int size, unsigned char *retdata, int *retsize) = 0;

    int private_decrypt(const unsigned char *data, int size, unsigned char *retdata, int *retsize);

    virtual std::string get_name() = 0;
    virtual std::string get_cn() = 0;
    virtual std::vector<std::string> get_alt_name( SubjectAltName type ) = 0;
    virtual std::vector<std::string> get_subject_info_access() = 0;
    virtual std::string get_issuer() = 0;
    virtual std::string get_issuer_cn() = 0;

    bool verify_signed_by(SRef<Certificate*> cert);

    bool has_alt_name_sip_uri(std::string uri);
    bool has_alt_name(std::string uri);
    bool has_alt_name(std::string uri, SubjectAltName type);

    std::string get_file();
    std::string get_pk_file();

    SRef<Private_Key*> get_pk();
    void set_pk( SRef<Private_Key *> pk);
    void set_pk( const std::string &file );
    void set_encpk(char *derEncPk, int length, const std::string &password, const std::string &path);

    bool has_pk();

protected:
    Certificate();

    std::string file;

    SRef<Private_Key *> m_pk;
};

class Certificate_Chain : public SObject
{
public:
    static Certificate_Chain* create();
    virtual ~Certificate_Chain();

    virtual Certificate_Chain* clone();
    virtual void add_certificate( SRef<Certificate *> cert );
    virtual void add_certificate_first( SRef<Certificate *> cert );

    virtual void remove_last();

    virtual int control( SRef<Certificate_Set *> cert_db ) = 0;
    virtual SRef<Certificate *> get_next();
    virtual SRef<Certificate *> get_first();
    virtual SRef<Certificate *> get_last();

    virtual void clear();

    virtual int length();
    virtual void lock();
    virtual void unlock();

    virtual bool is_empty();

    virtual void init_index();

protected:
    Certificate_Chain();
    Certificate_Chain( SRef<Certificate *> cert );

    std::list< SRef<Certificate *> > cert_list;
    std::list< SRef<Certificate *> >::iterator item;
    Mutex mLock;
};

class Certificate_Exception : public Exception
{
public:
    Certificate_Exception( const char *desc) : Exception(desc) {}
};


class Certificate_Exception_File : public Certificate_Exception
{
public:
    Certificate_Exception_File( const char *message ):Certificate_Exception(message){}
};

class  Certificate_Exception_Init : public Certificate_Exception
{
public:
    Certificate_Exception_Init( const char *message ):Certificate_Exception(message){}
};

class  Certificate_Exception_Pkey : public Certificate_Exception
{
public:
    Certificate_Exception_Pkey( const char *message ):Certificate_Exception(message){}
};

class  Certificate_Exception_Chain : public Certificate_Exception
{
public:
    Certificate_Exception_Chain( const char *message ):Certificate_Exception(message){}
};
#endif // CERT_H
