#ifndef HTTP_DOWNLOADER_H
#define HTTP_DOWNLOADER_H
#include <map>
#include <vector>
#include "tcp_socket.h"
#include "downloader.h"

#define HTTP_RESPONSECODE_OK 			200
#define HTTP_RESPONSECODE_NOTFOUND 		404
#define HTTP_RESPONSECODE_MOVEDPERMANENTLY 	301
#define HTTP_RESPONSECODE_MOVEDTEMPORARILY 	302
#define HTTP_RESPONSECODE_SERVERERROR 		500

#define HTTP_HEADER_FROM 			"From"
#define HTTP_HEADER_USERAGENT 			"User-Agent"
#define HTTP_HEADER_SERVER 			"Server"
#define HTTP_HEADER_HOST 			"Host"
#define HTTP_HEADER_ACCEPTS 			"Accepts"
#define HTTP_HEADER_CONTENTTYPE 		"Content-Type"
#define HTTP_HEADER_CONTENTLENGTH 		"Content-Length"
#define HTTP_HEADER_LASTMODIFIED 		"Last-Modified"

class Http_Downloader : public Downloader
{
public:
    Http_Downloader(std::string url);
    Http_Downloader(std::string url, SRef<Stream_Socket*> sock);
    virtual ~Http_Downloader();

    char*	get_chars(int *length);

    bool 	download_to_file(std::string filename);
    std::string 	download_to_string();
    bool 	download_headers();
    std::string 	get_header(std::string header);
    int 	get_response_code() const;

    void 	set_follow_redirects(bool const val);
    bool 	get_follow_redirects() const;

    virtual std::string get_mem_object_type() const {return "HttpDownloader";}

private:
    int 	fetch(std::string request, std::ostream & bodyStream);

    void 	parse_header(std::string line);
    int 	parse_headers(std::stringstream & headers);
    void 	parse_url();

    void	split(std::string data, std::string token, std::vector<std::string> &res, int maxChars = -1);
    std::string	trim(std::string s);

    std::string	build_request_string(std::string method, std::string file);

    std::string 	url;
    std::string 	remote_hostname;
    std::string 	remote_protocol;
    std::string 	remote_file;
    int 	remote_port;
    std::map<std::string, std::string> headers;
    int 	resp_code;
    bool	follow_redirect;
    SRef<Stream_Socket *> sock;
    bool 	internal_socket_object;
};

#endif // HTTP_DOWNLOADER_H
