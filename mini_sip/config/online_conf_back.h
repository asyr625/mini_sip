#ifndef ONLINE_CONF_BACK_H
#define ONLINE_CONF_BACK_H
#include <vector>
using namespace std;

#include "conf_backend.h"

class Tls_Srp_Socket;
class Certificate;

struct contdata
{
    char *data;
    int size;
};

class Online_Conf_Back
{
public:
    Online_Conf_Back(string addr, int port, string user, string pass);
    ~Online_Conf_Back();
    int download_req(string user, string type, vector<struct contdata*> &result);
    void upload_req(string user, string type, string data );
    string base64_encode(char *data, int length);
    string get_user()
    {
        return usrname;
    }
    void set_online_cert(Certificate *cer);
    Certificate * get_online_cert();

    string attach_file(string mimeheader, string data);
private:
    string read_http_header();
    string usrname;
    Tls_Srp_Socket *tls;
    Certificate *cert;
};

#endif // ONLINE_CONF_BACK_H
