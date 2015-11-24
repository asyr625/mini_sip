#include "online_conf_back.h"
#include "tls_srp_socket.h"
#include "cert.h"
#include "network_exception.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <string.h>

using namespace std;

void split(string data, string token, vector<string> &res)
{
   int count =0;;
   int lastpos =0;
   int pos = data.find(token,lastpos);
   while(string::npos != pos)
     {
    count = pos -lastpos;
    res.push_back(data.substr(lastpos,count));
    lastpos=pos+1;
    pos = data.find(token,lastpos+1);
     }
   res.push_back(data.substr(lastpos));
}

Online_Conf_Back::Online_Conf_Back(string addr, int port, string user, string pass)
{
    usrname = user;
    tls = new Tls_Srp_Socket( addr, port, user , pass);
}

Online_Conf_Back::~Online_Conf_Back()
{
    if( tls )
        delete tls;
}

string Online_Conf_Back::read_http_header()
{
    char ch[65];/* ?? under 65 segfaultar programmet*/
    char prev[4];
    char buf[1024];
    memset (buf,0,1025);
    memset (prev,0,5);
    memset (ch, 0, 66);
    int ret;
    int a=0;
    for(int i = 0;i<1024;i++)/*read http header*/
    {
        ret = tls->read(&ch, 1);
        if (ret == 0)
        {
            printf ("\n- Peer has closed the GNUTLS connection\n");
            break;
        }
        else if (ret < 0)/* do some action*/
        {
            fprintf (stderr, "\n*** Received corrupted "
                    "data(%d). Closing the connection.\n\n", ret);
            break;
        }
        //prev[i%4]=ch[0];
        buf[i]=ch[0];
        if(ch[0]=='\r' || ch[0]=='\n')
        {
            prev[a%4]=ch[0];

            if(prev[0]=='\r' && prev[1]=='\n' &&prev[2]=='\r' &&prev[3]=='\n')
            {
                break;
            }
            a++;
        }
        else
            a=0;
    } /*endof read header*/
    string header(buf);
    return header;
}


int Online_Conf_Back::download_req(string user, string type, vector<struct contdata*> &result)
{
    string header("GET /"+ user +"/" + type +" HTTP/1.1\r\n" +"Host: cred.minisip.org:5556\r\n" +
            "Accept: application/octet-stream, text/xml, application/xml\r\n\r\n");
    tls->write(header);
    string respheader;
    cout<<"waiting for response"<<endl;
    respheader = read_http_header();
    cout<<"data recieved, parsing"<<endl;
    vector<string> res;
    split(respheader, "\r\n", res);
    int pos = res.at(0).find("200");
    if(pos != string::npos)/*the operation was successfull*/
    {
        int clgth=0;
        string boundary;
        for(int i=0;i<res.size();i++)
        {
            if((res.at(i).find("boundary=") != string::npos) || (res.at(i).find("Boundary=") !=string::npos))
            {
                vector<string> b;
                split(res.at(i), "=",b);
                //cout<<b.at(1)<<endl;
                boundary=b.at(1);
            }

            if((res.at(i).find("Length") != string::npos) || (res.at(i).find("length") !=string::npos))
            {
                vector<string> lgth;
                split(res.at(i), " ", lgth);
                //cout<<"Content-Length: "<<lgth.at(1)<<endl;
                clgth=atoi(lgth.at(1).c_str());
            }
        }

        char cont[clgth];
        char *tmp, *loc=NULL,*hloc=NULL, *eloc=NULL, *tmpstart;
        memset (cont, 0, clgth+1);
        int bytes;
        bytes = tls->read(&cont, clgth);
        if(bytes<=0)
            return -1;
        loc = strstr(cont,boundary.c_str());
        int i=0;

        struct contdata *Data;
        string content(cont);
        vector<string> datastr;
        split(content, boundary,datastr);
        for(int iter=0;iter<datastr.size();iter++) /*parsing of mime headers*/
        {
            int position =datastr.at(iter).find("\r\n\r\n");
            if(position !=string::npos)
            {
                //cout<<datastr.at(iter)<<endl;

                gnutls_datum_t enc, dec;
                string tmp = datastr.at(iter).substr(position+4);
                enc.data = (unsigned char*)tmp.c_str();
                enc.size = tmp.size();
                int fail =gnutls_srp_base64_decode_alloc (&enc, &dec);
                if(fail <0)
                {
                    cout<< "failed to decode content"<<endl;
                    return -1;
                }
                string sdfg((char*)dec.data,dec.size);
                Data = (struct contdata*)malloc(sizeof(struct contdata));
                Data->data = (char*)dec.data;
                Data->size = dec.size;
                result.push_back(Data);
            }
            else if ((position ==string::npos) && (result.size()>=1))
                return 1;
        }
    }
    else
    {
        return -1;
        /*do some action*/
    }
    return -1;
}

void Online_Conf_Back::upload_req(string user, string type, string data )
{
    string mimeheaders = "-----------192345\r\n" + data;
    char conv[100];
    sprintf(conv,"%d", mimeheaders.size());
    string header("PUT /"+ user +"/" + type +" HTTP/1.1\r\n" +"Host: cred.minisip.org:5556\r\n");
    header = header + "Content-Type: multipart/mixed; boundary=-----------192345\r\n";
    header = header + "Content-Length: ";
    header = header + conv;
    header = header + "\r\n";
    header = header + "Connection: Keep-Alive\r\n" +"\r\n";
    tls->write(header + mimeheaders);
}

string Online_Conf_Back::base64_encode(char *data, int length)
{
    gnutls_datum_t gnu;
    gnutls_datum_t gnures;
    gnu.data = (unsigned char*)data;
    gnu.size = length;
    gnutls_srp_base64_encode_alloc (&gnu, & gnures);
    string ret((char*)gnures.data,(int)gnures.size);
    return ret;
}
void Online_Conf_Back::set_online_cert(Certificate *cer)
{
    cert = cer;
}

Certificate * Online_Conf_Back::get_online_cert()
{
    return cert;
}

string Online_Conf_Back::attach_file(string mimeheader, string data)
{
    mimeheader = mimeheader + "Content-type: application/octet-stream\r\n";
    mimeheader = mimeheader + "Content-Transfer-Encoding: binary\r\n\r\n";
    mimeheader = mimeheader + data;
    mimeheader = mimeheader + "-----------192345\r\n";
    return mimeheader;
}
