#include "file_system.h"

#include<fstream>

#include<sys/stat.h>
#include<sys/types.h>

#ifdef WIN32
#include<io.h>
#include<direct.h>
#endif

using namespace std;

class Local_File : public File
{
public:
    Local_File(string path);
    virtual int32_t read(void *buf, int32_t count);
    virtual void write(void *buf, int32_t count);
    virtual bool eof();
    virtual void seek(int64_t pos );
    virtual int64_t offset();
    virtual int64_t size();
    virtual void flush();

private:
    fstream file;
};


Local_File::Local_File(string path)
{
    file.open( path.c_str(), ios::out | ios::in | ios::binary );
    if (!file.is_open()){
        throw File_Exception("Could not open file");
    }
}

int32_t Local_File::read(void *buf, int32_t count)
{
    int64_t spos = offset();
    file.read( (char*)buf, count );
    return (int32_t) ( offset() - spos );
}

void Local_File::write(void *buf, int32_t count)
{
    file.write( (char*)buf, count);
}

bool Local_File::eof()
{
    return !file;
}

void Local_File::seek(int64_t pos )
{
    file.seekg( pos, ios::beg );
}

int64_t Local_File::offset()
{
    return file.tellg();
}

int64_t Local_File::size()
{
    int64_t tmp = file.tellg();		// save position
    file.seekg(0, std::ios_base::beg);
    std::ifstream::pos_type begin_pos = file.tellg();
    file.seekg(0, std::ios_base::end);
    int64_t s = static_cast<int64_t>( file.tellg() - begin_pos );
    file.seekg( tmp, ios::beg ); 		// restore position
    return s;
}

void Local_File::flush()
{
    file.flush();
}


void Local_File_System::mkdir( const std::string & name )
{
#ifdef WIN32
    if ( _mkdir( name.c_str() ) != 0 ){
        throw File_System_Exception("Could not create directory");
    }

#else
    if ( ::mkdir( name.c_str(), 0 ) != 0 ){
        throw File_System_Exception("Could not create directory");
    }
#endif
}

SRef<File*> Local_File_System::open( const std::string & name, bool createIfNotExist )
{
    string tmp = name;
    if ( name.size()>0  &&  name[0]!='/' )
    {
        tmp = def_prefix + name;
    }
    return new Local_File(tmp);
}


File_Exception::File_Exception( string why ) : Exception(why){ }

File_System_Exception::File_System_Exception( string why ) : Exception(why){ }

void File_System::set_default_path(std::string dp)
{
    if (dp[dp.size()-1]=='/')
        def_prefix = dp;
    else
        def_prefix = dp+'/';

}

std::string File_System::get_default_path()
{
    return def_prefix;
}
