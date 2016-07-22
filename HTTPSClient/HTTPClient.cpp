#include "HTTPClient.h"
#include <sstream>

HTTPClient::HTTPClient()
{
    rHost = "";
    rPort = 80; //Puderto por default.
    urlRsc = "/";
}

HTTPClient::~HTTPClient()
{
    //dtor
}


HTTPMessage* HTTPClient::createHTTPMessage(char* method, char* url, char* params, char* body, long sizeBody)
{
    if (method == NULL || url == NULL ||  strlen(url) <= 0)
        return NULL;

    HTTPMessage* httpM = new HTTPMessage();
    long size = 0;


    httpM->content_length = 0;
    httpM->body = NULL;
    httpM->socketErrorCode = 0;
    httpM->params = NULL;

    size = strlen(method);
    httpM->method = new char[size+1];
    memset(httpM->method,'\0',size+1);
    memcpy(httpM->method, method,size);

    size = 0;
    size = strlen(url);
    httpM->url = new char[size+1];
    memset(httpM->url,'\0',size+1);
    memcpy(httpM->url, url,size);

    if (params != NULL && strlen(params) >0)
    {
        size = 0;
        size = strlen(params);
        httpM->params = new char[size+1];
        memset(httpM->params,'\0',size+1);
        memcpy(httpM->params, params,size);
    }

    httpM->header[HTTP_HDR_USER_AGENT] = "TKQ_HTTP_CLIENT/V1.0"; //Por default se mete la descipcion del cliente.

    //Si el metodo es POST se asigna el MIME TYPE por default
    ostringstream sdata;
    if(strcmp(HTTP_METHOD_POST,httpM->method) == 0)
    {
        httpM->header[HTTP_HDR_CONTENT_TYPE] = "application/x-www-form-urlencoded";
        //Cuando el Metodo es POST el Body son los parametros de consulta.
        if(params != NULL)
        {
            string ps = params;
            httpM->content_length = ps.size();
            sdata << httpM->content_length;
        }
        else if( body != NULL && sizeBody >0)
        {
            httpM->body = new char[sizeBody+1];
            memset(httpM->body,'\0',sizeBody+1);
            memcpy(httpM->body, body,sizeBody);
            httpM->content_length = sizeBody;
            sdata << sizeBody;
        }
        else
        {
            httpM->content_length = 0;
            sdata << "0";
        }
    }
    else{ //Si no es de tipo POST.
        //Si se desea enviar con POST el mensaje.
        if( body != NULL && sizeBody >0)
        {
            httpM->body = new char[sizeBody+1];
            memset(httpM->body,'\0',sizeBody+1);
            memcpy(httpM->body, body,sizeBody);
            httpM->content_length = sizeBody;
        }
        sdata << sizeBody;
    }
    httpM->header[HTTP_HDR_CONTENT_LENGHT] = sdata.str();
    return httpM;
}


void HTTPClient::setHeader(HTTPMessage* msg, string keyheader, string data)
{
     msg->header[keyheader] = data;
}


void HTTPClient::freeHTTPMessage(HTTPMessage* httpM)
{
    if (httpM != NULL)
    {
        if (httpM->method != NULL) delete[] httpM->method;
        if (httpM->url != NULL) delete[] httpM->url;
        if (httpM->params != NULL) delete[] httpM->params;
        if (httpM->body != NULL) delete[] httpM->body;

        delete httpM;
    }
}


HTTPMessage* HTTPClient::sendHTTPRequest(HTTPMessage *rq)
{
    if(rq == NULL) return NULL;

     //Si la URL que se envia no es valida
    if( !setHost(rq->url)) return NULL;
    setPort(rq->url);
    setResource(rq->url);

    bool isPost = false;
    map<string,string>::iterator it;
    ostringstream hd;

    if(strcmp(HTTP_METHOD_POST,rq->method) == 0)
        isPost = true;

    //construye el header.
    hd << rq->method << " " <<  urlRsc;

    //TODO: Codificar los PARAMETROS.

    //Si es GET los parametros van dentro de la URL.
    //Si el metodo es post se procede a ignorar el BODY del contenido
    if( !isPost && rq->params != NULL)
    {
        hd << "?" << rq->params;

        ostringstream aux;
        aux << rq->url << "?" << rq->params;
        urlFin = aux.str();
    }
    else
        urlFin = rq->url;

    hd << " HTTP/1.1" << endl;

    //Anade el Header.
    for(it = rq->header.begin(); it != rq->header.end(); it++)
    {
        string k = (string)it->first;
        string d = (string)it->second;

        hd << k << ": " << d << endl;
    }

    hd << HTTP_HDR_HOST << ": " << rHost << endl;


    //Inserta el fin del Header.
    hd << endl;

    //Asigna el Boady al mensaje
    if(isPost && rq->content_length > 0 )
    {
        if(rq->params != NULL)
            hd << rq->params;
        else
            hd << rq->body;
    }
    else if( !isPost && rq->content_length > 0 )
        hd << rq->body; //Se le anade el fin de linea



    int err = 0;
    //1.- Abre el canal de comunicacion.


    string rqData =hd.str();

cout << endl << hd.str() << endl;

    err = openTxChannel();

    HTTPMessage *rsp = doReqRecResp(rq, (char*)rqData.c_str(), rqData.size());

    closeChannel();

    return rsp;
}


HTTPMessage* HTTPClient::doReqRecResp(HTTPMessage *rq, char * rqBytes, long sizeBytes)
{
    int err = 0;
    rq->socketErrorCode = err;
    if(err != 0)return NULL;


    //2.- Envia los datos del HTTP Request.
    err = tx(rqBytes, sizeBytes);
    rq->socketErrorCode = err;
    if(err != 0)return NULL;


    //2.1 Prepara el HTTPMessage con el que contrndra la respuesta
    HTTPMessage* rsp = new HTTPMessage();
    rsp->content_length = 0;

    //3.- Recibe linea por linea del RESPONSE
    string line = rxLine(&err);
    rsp->socketErrorCode = err;
    if(err != 0) return NULL;
    rsp->respLine = line;

    bool keepR = true;

    //Leera mientras la linea que se leyo sea mayor a cero.
    while(keepR)
    {
        line = rxLine(&err);
        rsp->socketErrorCode = err;

        //Hubo problemas para leer la linea.
        if(err != 0) return rsp;

        if(line.size() > 0)
        {

            string k = getKey(line);
            //Si la linea que llego corresponde al tamano del body.
            if( k.compare(HTTP_HDR_CONTENT_LENGHT) == 0 )
            {
                string value = getValue(line);
                rsp->content_length = (unsigned long)atol((char*)value.c_str());
            }
            rsp->header[k] = line; //anade el header
        }
        else //Es una linea en blanco. Termino el header.
            keepR = false;
    }

    if(rsp->content_length > 0)
    {
cout << "Leyendo BODY..." << endl;

        rsp->body = rx(rsp->content_length, &err);
        rsp->socketErrorCode = err;
    }

    return rsp;
}

string HTTPClient::getKey(string data)
{
    string key = "";
    if(data.size() == 0) return key;

    int posFirst = data.find(":",0); //Busca el separador del HEADER

    key = data.substr(0, posFirst);


    return key;
}

string HTTPClient::getValue(string data)
{
    string value = "";
    if(data.size() == 0) return value;

    int posFirst = data.find(":",0); //Busca el separador del HEADER

    value = data.substr(posFirst + 1);


    return value;
}

/**
    Forma de la URL

    http://host[:port]/...
**/
bool HTTPClient::setHost(char* urlPtr)
{
    if (urlPtr== NULL || strlen(urlPtr) <= 0) return false;

    string url = urlPtr;
    int posFirst = -1;
    int posLast = -1;

    posFirst = url.find("://",0);
    if (posFirst < 0) return false;
    posFirst = posFirst + 3; //Se salta estos tres caracteres.

    //Busca si esta dato de alta el puerto.
    posLast = url.find(":",posFirst);

    //Si no esta el puerto busca la diagonal del URL
    if (posLast < 0)
        posLast = url.find("/",posFirst);

    //Si no esta la diagonal busca el comienzo de los parametros.
    if (posLast < 0)
        posLast = url.find("?",posFirst);

    if (posLast < 0)
        rHost = url.substr(posFirst);
    else
    {
        rHost = url.substr(posFirst,(posLast - posFirst));
    }
    return true;
}

/**
    Forma de la URL

    http://host[:port]/...
**/
void HTTPClient::setPort(char* urlPtr)
{
    rPort = 80;
    if (urlPtr== NULL || strlen(urlPtr) <= 0) return;

    string aux = "80";

    string url = urlPtr;
    int posFirst = -1;
    int posLast = -1;

    posFirst = url.find("://",0);

    if (posFirst < 0) return;

    posFirst = posFirst + 3; //Se salta estos tres caracteres.

    //Busca si esta dato de alta el puerto.
    posFirst = url.find(":",posFirst);

    //Si Esta declarada la seccion de puerto
    if (posFirst != -1)
    {
        posFirst = posFirst + 1;

        posLast = url.find("/",posFirst);

        //Si no esta el puerto busca la diagonal del URL
        if (posLast < 0)
            posLast = url.find("/",posFirst);


        //Ontiene el PUERTO
        if (posLast < 0)
            posLast = url.find("?",posFirst);

        if (posLast < 0)
            aux = url.substr(posFirst);
        else
        {
            aux = url.substr(posFirst,(posLast - posFirst));
        }

        rPort = atoi((char*)aux.c_str());
    }

}

/**
    Forma de la URL

    http://host[:port][/Recurso/...]
**/
void HTTPClient::setResource(char* urlPtr)
{
    if (urlPtr== NULL || strlen(urlPtr) <= 0) return;

    string url = urlPtr;
    int posFirst = -1;
    int posLast = -1;

    posFirst = url.find("://",0);

    if (posFirst < 0) return;

    posFirst = posFirst + 3; //Se salta estos tres caracteres.

    //Busca si esta dato de alta el puerto.
    posLast = url.find(":",posFirst);

    //Si Esta declarada la seccion de puerto
    //Se considera buscar el recurso a partir de la posicion actual
    if (posLast > 0)
        posFirst = posLast + 1;
    else //Se actualiza la posicion de busqueda.
        posFirst++;

    //Busca la diagonal de inicio del recuros
    posLast = url.find("/",posFirst);

    //Si no la encontro
    if (posLast < 0)
        urlRsc = "/";
    else //Si la encontro procede a obtener el recurso.
        urlRsc = url.substr(posLast);
}

//------------------------------------------------------------------------------------
//          SOCKETS
//------------------------------------------------------------------------------------


void HTTPClient::closeChannel()
{
    curl_easy_cleanup(curl);
}


int HTTPClient::openTxChannel()
{
    int err = 0;
    curl = curl_easy_init();

    if(curl) {
        cout << "URL[" << urlFin << "]" << endl;

        curl_easy_setopt(curl, CURLOPT_URL, urlFin.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);

        err = curl_easy_perform(curl);

        if(CURLE_OK != err)
        {
          cout << "ERROR (curl_easy_perform): " << curl_easy_strerror( (CURLcode) err) << endl;
          return err;
        }

        err = curl_easy_getinfo(curl, CURLINFO_LASTSOCKET, &socketTx);

        sockfd = socketTx;

        if(CURLE_OK != err)
        {
          cout << "ERROR (curl_easy_getinfo): URL[" << urlFin << "]" << curl_easy_strerror( (CURLcode) err) << endl;
          return err;
        }

    }
    else
        err = -1;
    return err;
}


int HTTPClient::wait_for_socket(int for_recv, long timeout_ms)
{
  struct timeval tv;
  fd_set infd, outfd, errfd;
  int res;

  tv.tv_sec = 0;
  tv.tv_usec= timeout_ms * 1000;

  FD_ZERO(&infd);
  FD_ZERO(&outfd);
  FD_ZERO(&errfd);

  FD_SET(sockfd, &errfd); /* always check for error */

  if(for_recv == 1)
  {
    FD_SET(sockfd, &infd);
    res = select(sockfd, &infd, NULL, &errfd, &tv);
  }
  else
  {
    FD_SET(sockfd, &outfd);
    res = select(sockfd, NULL, &outfd, &errfd, &tv);
  }
  return res;
}


int HTTPClient::tx(char* buffOut, int sizeBuf)
{
    int bsend = 0;
    size_t iolen;

    wait_for_socket(0,100);

    if (buffOut == NULL || sizeBuf <= 0)
        return -1;

    bsend = curl_easy_send(curl, buffOut, sizeBuf, &iolen);

    return bsend;
}


string HTTPClient::rxLine(int* errCode)
{
    size_t iolen;
    ostringstream line;
    line  << "\0";

    *errCode = 0;
    int brcv = 0;//bytes recived

    char c[2];

//cout << "rxLine..." << endl;

wait_for_socket(1,60000);
    //Mientras espere recibir datos
    do{
        iolen = 0;
        memset(c,'\0',2);

        c[0] = '\0';
        int dataRecv = curl_easy_recv(curl, c, 1, &iolen);

        if (iolen <= 0)
        {
            if (dataRecv == 0)
                *errCode = 0;
            else
                *errCode = dataRecv;

            return line.str();
        }

        //Si es diferente de Salto de linea y Retorno de carro.
        if(c[0] != '\0' && c[0] != '\n' && c[0] != '\r')
            line << c[0] << "\0";

    }while(c[0] != '\n');

    return line.str();
}

char* HTTPClient::rx(int expectedBytes, int* errCode)
{

    if (expectedBytes <= 0)
    {
        *errCode = -1;
        return NULL;
    }

    size_t iolen;
    string aux = "";
    int sizeRes = 0;
    char* data = new char[expectedBytes + 1];

    *errCode = 0;
    memset(data,'\0',expectedBytes + 1);

    size_t brcv = 0;//bytes recived
    int mbytes = expectedBytes - brcv; //missing bytes  E100 R 0  M100

    //Mientras espere recibir datos
    while (mbytes > 0)
    {
        wait_for_socket(1,6000);

        int dataRecv = curl_easy_recv(curl, (char*)data + brcv, mbytes, &iolen);

        if (iolen <= 0)
        {
            //Si termino sin error
            if (dataRecv == 0)
                *errCode = 0;
            else
            {
                *errCode = dataRecv;

                if (data !=NULL)
                    delete[] data;

                return NULL;
            }

        }
        brcv +=  iolen;
        mbytes = expectedBytes - brcv;
    }

    data[expectedBytes] = '\0';
    return data;
}


