#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <string>
#include <iostream>
#include <map>
#include <curl/curl.h>

#define HTTP_METHOD_POST     "POST"
#define HTTP_METHOD_GET      "GET"
#define HTTP_MIME_TYPE_DEF   "text/html"

#define HTTP_HDR_CONTENT_LENGHT     "Content-Length"
#define HTTP_HDR_CONTENT_TYPE       "Content-Type"
#define HTTP_HDR_USER_AGENT         "User-Agent"
#define HTTP_HDR_HOST               "Host"



using namespace std;

typedef struct HTTPMessage
{
    char* method;
    char* url;
    char* params;
    unsigned long content_length;
    char* body;
    string respLine;
    map<string,string> header;
    long socketErrorCode;
};

class HTTPClient
{
public:
    HTTPClient();
    virtual ~HTTPClient();

    HTTPMessage* createHTTPMessage(char* method, char* url, char* params, char* body, long sizeBody);
    void setHeader(HTTPMessage* msg, string keyheader, string data);
    void setBody(HTTPMessage* httpM, char* body, unsigned long size);

    void freeHTTPMessage(HTTPMessage* httpM);


    HTTPMessage* sendHTTPRequest(HTTPMessage *request);

private:

    CURL *curl;
    curl_socket_t sockfd;
    long socketTx;

    string rHost;
    string urlRsc;
    string urlFin;
    int rPort;

    HTTPMessage* doReqRecResp(HTTPMessage *rq, char * rqBytes, long sizeBytes);
    string getKey(string data);
    string getValue(string data);


    bool setHost(char* urlPtr);
    void setPort(char* urlPtr);
    void setResource(char* urlPtr);

    void closeChannel();
    int openTxChannel();

    int wait_for_socket(int for_recv, long timeout_ms);
    int tx(char* buffOut, int sizeBuf);
    char* rx(int expectedBytes, int* errCode);
    string rxLine(int* errCode);

};

#endif // HTTPCLIENT_H
