#include "http_downloader.h"


#include <fstream>
#include <sstream>

#include <string>
#include <iostream>
#include <string.h>
#include <stdlib.h>
using namespace std;

#define BUFFERSIZE 4096

#define HTTP_METHOD_1_0                         "HTTP/1.0"
#define HTTP_HEADER_CRLF                        "\r\n"

Http_Downloader::Http_Downloader(std::string url)
    : url (url), remote_port(80), resp_code(-1), follow_redirect(true)
{
    parse_url();
    if (remote_port > 0 && remote_hostname != "") {
        sock = new TCP_Socket(remote_hostname, remote_port);
    }
}
Http_Downloader::Http_Downloader(std::string url, SRef<Stream_Socket*> sock)
    : url (url), remote_port(80), resp_code(-1), follow_redirect(true), sock(sock)
{
    parse_url();
}
Http_Downloader::~Http_Downloader()
{
}

char* Http_Downloader::get_chars(int *length)
{
    int tries = 3;
    *length = 0;
    while (tries)
    {
        std::ostringstream body;
        int fetchRes = fetch(build_request_string("GET ", remote_file), body);
        if (fetchRes == HTTP_RESPONSECODE_MOVEDPERMANENTLY || resp_code == HTTP_RESPONSECODE_MOVEDTEMPORARILY)
        {
            url = get_header("Location");
            parse_url();
            if (!follow_redirect)
            {
                return NULL;
            }
        }
        else if (fetchRes == HTTP_RESPONSECODE_OK)
        {
            *length = (int)body.str().length();
            char* res = new char[*length];
            memcpy(res, body.str().c_str(), *length);
            return res;
        } else {
            return NULL;
        }
        tries--;
    }
    return NULL;
}


int Http_Downloader::get_response_code() const
{
    return resp_code;
}

void Http_Downloader::set_follow_redirects(bool const val)
{
    follow_redirect = val;
}

bool Http_Downloader::get_follow_redirects() const
{
    return follow_redirect;
}

int Http_Downloader::fetch(std::string request, std::ostream & bodyStream)
{
    if ( sock.is_null() ) {
        // TODO: Error check for socket object
    }

    std::stringstream headerStream;
    int32_t bytesWritten = 0, bytesRead = 0;

    /* Buffer for holding data read from the network stream */
    char buffer[BUFFERSIZE];
    memset(buffer, 0, sizeof(buffer)); // Zero out the buffer used when recieving data


    /* Send request */
    bytesWritten = sock->write(request.c_str(), (int32_t)request.length());
    if (bytesWritten < 0) {
        //cerr << "Error: Could not send request" << endl;
        return false;
    } else if (bytesWritten < (int32_t)request.length()) {
        return false;
    }


    /* Read response */
    bool headerMode = true;
    int headerParseResult = 0;
    while ((bytesRead = sock->read(buffer, BUFFERSIZE)) > 0)
    {
        if (headerMode) {
            // Search for headers/body boundary in lastly fetched data
            char* bodyStr = strstr(buffer, "\r\n\r\n");
            int bodyLen = ((int)(buffer-bodyStr)) + bytesRead;

            if (bodyStr != NULL)
            {
                // Found boundary
                headerStream.write(buffer, (int)(bodyStr - (char*)buffer));
                // Error checking!
                if (headerStream.fail()) {
                    return 0;
                }

                bodyStream.write(bodyStr+4, bodyLen-4);
                // Error checking!
                if (bodyStream.fail()) {
                    return 0;
                }

                headerMode = false;

                headerParseResult = parse_headers(headerStream);
                switch (headerParseResult)
                {
                case HTTP_RESPONSECODE_OK:
                    break;
                default:
                    return headerParseResult;
                    break;

                }
            } else {
                headerStream.write(buffer, bytesRead);
                // Error checking!
                if (headerStream.fail()) {
                    return 0;
                }
            }
        } else {
            bodyStream.write(buffer, bytesRead);
            // Error checking!
            if (bodyStream.fail()) {
                return 0;
            }
        }
    }

    /*
    if (bytesRead < 0) {
        //cerr << "Error: Could not receive response" << endl;
        return 0;
    }
*/
    return HTTP_RESPONSECODE_OK;
}


void Http_Downloader::parse_url()
{
    size_t pos = 0;
    size_t lastPos = 0;
    // Find protocol
    if ((pos = url.find("://", 0)) != std::string::npos)
    {
        remote_protocol = url.substr(lastPos, pos - lastPos);
        lastPos = pos + 3;
    }
    // Find host (and possibly file on remote host, e.g. "index.html")
    if ((pos = url.find("/", lastPos)) != std::string::npos)
    {
        // At least the root of the webserver should be fetched (e.g. "http://www.sunet.se/", but more likely "http://www.sunet.se/index.html")
        remote_hostname = url.substr(lastPos, pos - lastPos);
        lastPos = pos;
        remote_file = url.substr(lastPos);
    } else {
        //Only host part specified (e.g. "http://www.sunet.se")
        remote_hostname = url.substr(lastPos);
    }
    // Find remote port
    if ((pos = remote_hostname.find(":", 0)) != std::string::npos)
    {
        remote_hostname = remote_hostname.substr(0, pos);
        remote_port = atoi(remote_hostname.substr(pos + 1).c_str());
    }
}

void Http_Downloader::split(std::string data, std::string token, std::vector<std::string> &res, int maxChars)
{
    size_t count = 0;
    size_t lastpos = 0;
    size_t tokenlen = token.length();
    size_t pos = data.find(token,lastpos);
    while(std::string::npos != pos && ((maxChars > 0 && (int)pos < maxChars) || maxChars <= 0))
    {
        count = pos - lastpos;
        res.push_back(data.substr(lastpos,count));
        lastpos = pos + tokenlen;
        pos = data.find(token,lastpos);
    }
    /**
     * If the entire string is to be scanned then we want to add the last part of the string/data
     * to the result list. This splitis not necessary is there is an upper limit when no more characters
     * are of interest.
     */
    if (maxChars <= 0)
    {
        res.push_back(data.substr(lastpos));
    }
}

std::string	Http_Downloader::trim(std::string s)
{
    size_t trimLeftPos = s.find_first_not_of(" \n\t\r");
    size_t trimRightPos = s.find_last_not_of(" \n\t\r");
    size_t pos = 0;
    size_t len = 0;

    if (trimLeftPos != std::string::npos)
        pos = trimLeftPos;

    if (trimRightPos != std::string::npos)
        len = trimRightPos + 1 - pos;
    else
        len = s.length() - pos;

    return s.substr(pos, len);
}

bool Http_Downloader::download_headers()
{
    int tries = 3;
    while (tries)
    {
        std::ostringstream body;
        int fetchRes = fetch(build_request_string("HEAD ", remote_file), body);
        if (fetchRes == HTTP_RESPONSECODE_MOVEDPERMANENTLY || resp_code == HTTP_RESPONSECODE_MOVEDTEMPORARILY)
        {
            url = get_header("Location");
            parse_url();
            if (!follow_redirect) {
                return false;
            }
        } else if (fetchRes == HTTP_RESPONSECODE_OK) {
            return true;
        } else {
            return false;
        }
        tries--;
    }
    return false;
}

std::string Http_Downloader::download_to_string()
{
    int tries = 3;
    while (tries)
    {
        std::ostringstream body;
        int fetchRes = fetch(build_request_string("GET ", remote_file), body);
        if (fetchRes == HTTP_RESPONSECODE_MOVEDPERMANENTLY || resp_code == HTTP_RESPONSECODE_MOVEDTEMPORARILY)
        {
            url = get_header("Location");
            parse_url();
            if (!follow_redirect) {
                return "";
            }
        } else if (fetchRes == HTTP_RESPONSECODE_OK) {
            return body.str();
        } else {
            return "";
        }
        tries--;
    }
    return "";
}

bool Http_Downloader::download_to_file(std::string filename)
{
    int tries = 3;
    while (tries)
    {
        std::ofstream file(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
        if (!file)
        {
            return false;
        }

        int fetchRes = fetch(build_request_string("GET ", remote_file), file);
        if (fetchRes == HTTP_RESPONSECODE_MOVEDPERMANENTLY || resp_code == HTTP_RESPONSECODE_MOVEDTEMPORARILY)
        {
            url = get_header("Location");
            parse_url();
            if (!follow_redirect) {
                return false;
            }
        } else if (fetchRes == HTTP_RESPONSECODE_OK)
        {
            return !file.fail();
        } else {
            return false;
        }
        tries--;
    }
    return false;
}

int Http_Downloader::parse_headers(std::stringstream & headers)
{
    std::vector<std::string> lines;
    std::vector<std::string> initialLine;
    std::string bodySep = HTTP_HEADER_CRLF;
    bodySep += HTTP_HEADER_CRLF;

    split(headers.str(), HTTP_HEADER_CRLF, lines);
    split(lines[0], " ", initialLine);

    resp_code = atoi(initialLine[1].c_str());

    for (size_t i=1; i<lines.size(); i++) {
        parse_header(lines.at(i));
    }

    return resp_code;
}

void Http_Downloader::parse_header(std::string line)
{
    size_t pos = line.find(':');
    if (pos != std::string::npos)
    {
        //cout << "Found header: [" << line.substr(0, pos) << " = " << trim(line.substr(pos+1)) << "]" << endl;
        headers[line.substr(0, pos)] = trim(line.substr(pos+1));
    } else {
        cerr << "(Http_Downloader::parse_header)ERROR: Header \"" << line << "\" is not valid!" << endl;
    }
}


std::string Http_Downloader::get_header(std::string header)
{
    std::map<std::string, std::string>::iterator iter = headers.find(header);
    if (iter != headers.end())
    {
        return iter->second;
    }
    return "";
}

std::string	Http_Downloader::build_request_string(std::string method, std::string file)
{
    std::string res = method + " " + file + " " + HTTP_METHOD_1_0;
    res.append(HTTP_HEADER_CRLF);

    res.append(HTTP_HEADER_FROM);
    res.append(": anonymous@minisip.org");
    res.append(HTTP_HEADER_CRLF);

    res.append(HTTP_HEADER_USERAGENT);
    res.append(": ssip-FileDownloader/0.1");
    res.append(HTTP_HEADER_CRLF);

    res.append(HTTP_HEADER_CRLF);
    //res.append(HTTP_HEADER_CRLF);

    return res;
}
