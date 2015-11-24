#include "file_system_utils.h"

#ifdef WIN32
    #include <io.h>
#else
    #include <sys/types.h>
    #include <dirent.h>
    #include <errno.h>
#endif
#include <list>
#include <string>

std::list<std::string> File_System_Utils::directory_contents(std::string dir, bool includeSubdirectories)
{
    std::list<std::string> res;
    directory_contents_internal(dir, includeSubdirectories, res);
    return res;
}
void File_System_Utils::directory_contents_internal(std::string dir, bool includeSubdirectories, std::list<std::string> & res)
{
#ifdef WIN32
    struct _finddatai64_t data;
    // First create the filename that will be use to initialize the find.
    // "*.*" are wild card characters that tells the find function to return a
    // list of all the files and directories.  You can limit this if you wish
    // to just file with specific extensions, for example "*.txt".  If you do that
    // then finder will not return any directory names.
    std::string dirComplete = dir + "\\*.*";
    // start the finder -- on error _findfirsti64() will return -1, otherwise if no
    // error it returns a handle greater than -1.
    intptr_t h = _findfirsti64(dirComplete.c_str(), &data);
    if(h >= 0) {
        //FILELIST thisList;
        // add empty FILELIST structure to the linked list argument
        //theList.push_back(thisList);
        // get pointer to the FILELIST just added to the linked list above.
        //list<FILELIST>::iterator it = theList.end();
        //it--;
        // set current path
        //(*it).path = path;
        do {
            if( (data.attrib & _A_SUBDIR) )
            {
                // make sure we skip "." and "..".  Have to use strcmp here because
                // some file names can start with a dot, so just testing for the
                // first dot is not suffient.
                if (strcmp(data.name,".") != 0 && strcmp(data.name,"..") != 0 && includeSubdirectories)
                {
                    // We found a sub-directory, so get the files in it too
                    std::string nextDir = dir + "\\" + data.name;
                    // recursion here!
                    directoryContentsInternal(nextDir, includeSubdirectories, res);
                }

            }
            else
            {
                // this is just a normal filename.  So just add it to our vector
                res.push_back(dir + "\\" + data.name);
                //(*it).theList.push_back(data.name);

            }
        }while( _findnexti64(h, &data) == 0);
        // close the find handle.
        _findclose(h);
    }
#else
    DIR *dp;
    struct dirent *dirp;
    if((dp = opendir(dir.c_str())) == NULL)
    {
        //cout << "Error(" << errno << ") opening " << dir << endl;
        //return errno;
        return;
    }

    while ((dirp = readdir(dp)) != NULL)
    {
        std::string fileName(dirp->d_name);
        if ("." != fileName && ".." != fileName)
        {
            if (DT_DIR == dirp->d_type)
            {
                if (includeSubdirectories)
                {
                    std::string nextDir = dir + "/" + fileName;
                    // recursion here!
                    directory_contents_internal(nextDir, includeSubdirectories, res);
                }
            } else {
                res.push_back(dir + "/" + fileName);
            }
        }
    }
    closedir(dp);
#endif
}
