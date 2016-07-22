#include <iostream>
#include<string>

#include "HTTPClient.h"

using namespace std;

bool testHTTPManager();
bool testHTTPReq();

int main()
{
    testHTTPReq();
    return 0;
}

bool testHTTPManager()
{/*
    HTTPClient httpC;

    //string url = "https://tkqserv:8080/pablito/ogeis\0";
    //string url = "https://tkqserv/pablito/ogeis\0";
    //string url = "https://tkqserv:8080/\0";
    //string url = "https://tkqserv:8080\0";
    //string url = "https://tkqserv/\0";
    //string url = "https://tkqserv\0";

    string body = "<root><h1>CHINGON MI SUPERMAN!!!</h1></root>";
    HTTPMessage* msg1 = httpC.createHTTPMessage(HTTP_METHOD_GET, (char*)url.c_str(), "par1=sdsa&par2=34",(char*)body.c_str(),(unsigned long)body.size());
    HTTPMessage* msg2 = httpC.createHTTPMessage(HTTP_METHOD_POST, (char*)url.c_str(), "par1=sdsa&par2=34",(char*)body.c_str(),(unsigned long)body.size());
    HTTPMessage* msg3 = httpC.createHTTPMessage(HTTP_METHOD_GET, (char*)url.c_str(), NULL,(char*)body.c_str(),(unsigned long)body.size());
    HTTPMessage* msg4 = httpC.createHTTPMessage(HTTP_METHOD_POST, (char*)url.c_str(), NULL,(char*)body.c_str(),(unsigned long)body.size());

    httpC.sendHTTPRequest(msg1);
    httpC.sendHTTPRequest(msg2);
    httpC.sendHTTPRequest(msg3);
    httpC.sendHTTPRequest(msg4);
*/
    return true;
}

bool testHTTPReq()
{
    HTTPClient httpC;
    map<string,string>::iterator it;


    string url = "http://www.jorgemedra.net/";
    //
    //string url = "http://192.168.1.112:8080/tkqproperty/readproperty";

    //HTTPMessage* request = httpC.createHTTPMessage(HTTP_METHOD_POST, (char*)url.c_str(),"usr=edgnet&pwd=edgnet&entity=BROKER_SERVICE",NULL,0);
    //HTTPMessage* request = httpC.createHTTPMessage(HTTP_METHOD_GET, (char*)url.c_str(),"page_id=265",NULL,0);
    HTTPMessage* request = httpC.createHTTPMessage(HTTP_METHOD_POST, (char*)url.c_str(),NULL,NULL,0);

/*
    string url = "https://www.jorgemedra.net";
    HTTPMessage* request = httpC.createHTTPMessage(HTTP_METHOD_GET, (char*)url.c_str(),NULL,NULL,0);
*/

    cout << "REQUEST -----------------------------------------------" << endl;

    cout << "URL: [" << request->url << "]" << endl;
    cout << "METHOD: [" << request->method << "]" << endl;

    if(request->params != NULL)
        cout << "PARAMS: [" << request->params << "]" << endl;

    cout << endl;

    for(it = request->header.begin(); it != request->header.end(); it++)
    {
        cout << (string)it->first << ": " << (string)it->second << endl;
    }


    cout << "----------------------------------------------- REQUEST" << endl << endl;



    //Envia un REQUEST y espera revibir el RESPONSE
    HTTPMessage* response = httpC.sendHTTPRequest(request);




    cout << "RESPONSE -----------------------------------------------" << endl;

    cout << "HEADER-----------------------------------------------" << endl;

    cout << response->respLine << endl;
    //Anade el Header.
    for(it = response->header.begin(); it != response->header.end(); it++)
        cout << (string)it->second << endl;

    cout << "----------------------------------------------- HEADER" << endl;

    if(response->body != NULL)
        cout << response->body;
    cout << endl << "-----------------------------------------------RESPONSE" << endl;

    //httpC.freeHTTPMessage(request);
    //httpC.freeHTTPMessage(response);
}

