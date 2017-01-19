//////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                              //
//              Copyright © 2016, 2017 Boubacar DIENE                                           //
//                                                                                              //
//              This file is part of mmstreamer project.                                        //
//                                                                                              //
//              mmstreamer is free software: you can redistribute it and/or modify              //
//              it under the terms of the GNU General Public License as published by            //
//              the Free Software Foundation, either version 3 of the License, or               //
//              (at your option) any later version.                                             //
//                                                                                              //
//              mmstreamer is distributed in the hope that it will be useful,                   //
//              but WITHOUT ANY WARRANTY; without even the implied warranty of                  //
//              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   //
//              GNU General Public License for more details.                                    //
//                                                                                              //
//              You should have received a copy of the GNU General Public License               //
//              along with mmstreamer.  If not, see <http://www.gnu.org/licenses/>.             //
//                                                                                              //
//////////////////////////////////////////////////////////////////////////////////////////////////

/*!
* \file   LinkHelper.c
* \brief  Helper functions to handle sockets
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "network/LinkHelper.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "LinkHelper"

#define NAME           PROJECT_NAME
#define VERSION        PROJECT_VERSION

#define CR             "\r"
#define LF             "\n"
#define CRLF           "\r\n"
#define LFLF           "\n\n"

#define CUSTOM_HEADER  "HELLO"CRLF

#define CUSTOM_CONTENT "Mime: %s"CRLF \
                       "MaxBufferSize: %u"CRLF""CRLF

#define HTTP_BOUNDARY  ".-_."VERSION"-"NAME"-"VERSION".-_."

#define HTTP_GET       "GET /%s HTTP/1.0"CRLF \
                       "HOST: %s:%u"CRLF \
                       "User-Agent: "NAME" v"VERSION""CRLF \
                       "Connection: keep-alive"CRLF""CRLF

#define HTTP_200_OK    "HTTP/1.0 200 OK"CRLF \
                       "Server: "NAME" v"VERSION""CRLF \
                       "Accept-Ranges: none"CRLF \
                       "Connection: close"CRLF \
                       "Content-Type: multipart/x-mixed-replace;boundary="HTTP_BOUNDARY""CRLF

#define HTTP_200_OK_BIS "HTTP/1.0 200 OK"CRLF \
                        "Server: "NAME" v"VERSION""CRLF \
                        "Accept-Ranges: none"CRLF \
                        "Connection: close"CRLF \
                        "Content-type: text/html"CRLF \
                        CRLF \
                        "<!DOCTYPE html>"CRLF \
                        "<html lang=\"en-us\">"CRLF \
                        " <head>"CRLF \
                        "  <meta charset=\"utf-8\" />"CRLF \
                        "  <meta name=\"viewport\" content=\"width=device-width\" />"CRLF \
                        "  <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />"CRLF \
                        "  <meta http-equiv=\"content-language\" content=\"en-us\" />"CRLF \
                        "  <title>"NAME" v"VERSION"</title>"CRLF \
                        "  <style id=\"body_style\" type=\"text/css\">"CRLF \
                        "   html { text-align: center; }"CRLF \
                        "   body { color: #000; background-color: #ececec; }"CRLF \
                        "  </style>"CRLF \
                        " </head>"CRLF \
                        " <body>"CRLF \
                        "  <h1>"NAME" V"VERSION"</h1>"CRLF \
                        "  <img src=\"/webcam\" width=\"640px\" height=\"480px\" />"CRLF \
                        " </body>"CRLF \
                        "</html>"CRLF

#define HTTP_400_BAD_REQUEST "HTTP/1.0 400 Bad Request"CRLF \
                             "Content-type: text/html"CRLF \
                             CRLF \
                             "<!DOCTYPE html>"CRLF \
                             "<html lang=\"en-us\">"CRLF \
                             " <head>"CRLF \
                             "  <meta charset=\"utf-8\" />"CRLF \
                             "  <meta name=\"viewport\" content=\"width=device-width\" />"CRLF \
                             "  <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />"CRLF \
                             "  <meta http-equiv=\"content-language\" content=\"en-us\" />"CRLF \
                             "  <title>"NAME" V"VERSION"</title>"CRLF \
                             "  <style id=\"body_style\" type=\"text/css\">"CRLF \
                             "   html { text-align: center; }"CRLF \
                             "   body { color: #000; background-color: #ececec; }"CRLF \
                             "  </style>"CRLF \
                             " </head>"CRLF \
                             " <body>"CRLF \
                             "  <h1>Bad Request</h1>"CRLF \
                             "  <p><u><a href=\"http://%s:%u/%s\">SERVER</a></u> does not handle this request</p>"CRLF \
                             " </body>"CRLF \
                             "</html>"CRLF

#define HTTP_404_NOT_FOUND   "HTTP/1.0 404 Not Found"CRLF \
                             "Content-type: text/html"CRLF \
                             CRLF \
                             "<!DOCTYPE html>"CRLF \
                             "<html lang=\"en-us\">"CRLF \
                             " <head>"CRLF \
                             "  <meta charset=\"utf-8\" />"CRLF \
                             "  <meta name=\"viewport\" content=\"width=device-width\" />"CRLF \
                             "  <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />"CRLF \
                             "  <meta http-equiv=\"content-language\" content=\"en-us\" />"CRLF \
                             "  <title>"NAME" V"VERSION"</title>"CRLF \
                             "  <style id=\"body_style\" type=\"text/css\">"CRLF \
                             "   html { text-align: center; }"CRLF \
                             "   body { color: #000; background-color: #ececec; }"CRLF \
                             "  </style>"CRLF \
                             " </head>"CRLF \
                             " <body>"CRLF \
                             "  <h1>Not Found</h1>"CRLF \
                             "  <p>This server did not understand the requested URL %s</p>"CRLF \
                             "  <p>Please, use this <u><a href=\"http://%s:%u/%s\">LINK</a></u> instead</p>"CRLF \
                             " </body>"CRLF \
                             "</html>"CRLF

#define HTTP_CONTENT   CRLF"--"HTTP_BOUNDARY""CRLF \
                       "Content-Type: %s"CRLF \
                       "Content-Length: %u"CRLF""CRLF

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef struct LINK_HELPER_PRIVATE_DATA_S {
    fd_set          rfds;
    fd_set          wfds;
    
    IOVEC           rIov;
    MSGHDR          rMsg;
    
    IOVEC           wIov;
    MSGHDR          wMsg;
    
    int32_t         sockFlags;
    ssize_t         nbBytesReceived;
    ssize_t         nbBytesSent;
    
    struct timeval  timeout;
} LINK_HELPER_PRIVATE_DATA_S;

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

static void keepMeAlive_f(LINK_HELPER_S *obj);

static void prepareCustomHeader_f(LINK_HELPER_S *obj, CUSTOM_HEADER_S *inOut);
static void parseCustomHeader_f  (LINK_HELPER_S *obj, CUSTOM_HEADER_S *inOut);

static void prepareCustomContent_f(LINK_HELPER_S *obj, CUSTOM_CONTENT_S *inOut);
static void parseCustomContent_f  (LINK_HELPER_S *obj, CUSTOM_CONTENT_S *inOut);

static void prepareHttpGet_f(LINK_HELPER_S *obj, HTTP_GET_S *inOut);
static void parseHttpGet_f  (LINK_HELPER_S *obj, HTTP_GET_S *inOut);

static void prepareHttp200Ok_f(LINK_HELPER_S *obj, HTTP_200_OK_S *inOut);
static void parseHttp200Ok_f  (LINK_HELPER_S *obj, HTTP_200_OK_S *inOut);

static void prepareHttp400BadRequest_f(LINK_HELPER_S *obj, HTTP_400_BAD_REQUEST_S *inOut);
static void parseHttp400BadRequest_f  (LINK_HELPER_S *obj, HTTP_400_BAD_REQUEST_S *inOut);

static void prepareHttp404NotFound_f(LINK_HELPER_S *obj, HTTP_404_NOT_FOUND_S *inOut);
static void parseHttp404NotFound_f  (LINK_HELPER_S *obj, HTTP_404_NOT_FOUND_S *inOut);

static void prepareHttpContent_f(LINK_HELPER_S *obj, HTTP_CONTENT_S *inOut);
static void parseHttpContent_f  (LINK_HELPER_S *obj, HTTP_CONTENT_S *inOut);

static int8_t getPeerName_f(LINK_HELPER_S *obj, LINK_S *link, RECIPIENT_S *result);

static int8_t  setBlocking_f      (LINK_HELPER_S *obj, LINK_S *link, uint8_t blocking);
static uint8_t isReadyForWriting_f(LINK_HELPER_S *obj, LINK_S *link, uint64_t timeout_ms);
static uint8_t isReadyForReading_f(LINK_HELPER_S *obj, LINK_S *link, uint64_t timeout_ms);

static int8_t readData_f (LINK_HELPER_S *obj, LINK_S *src, LINK_S *dst, BUFFER_S *buffer, ssize_t *nbRead);
static int8_t writeData_f(LINK_HELPER_S *obj, LINK_S *src, LINK_S *dst, BUFFER_S *buffer, ssize_t *nbWritten);

static void signalHandler_f(int32_t signalNumber);

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
void LinkHelper_Init(LINK_HELPER_S **obj)
{
    assert(obj && (*obj = calloc(1, sizeof(LINK_HELPER_S))));
    
    LINK_HELPER_PRIVATE_DATA_S *pData;
    assert((pData = calloc(1, sizeof(LINK_HELPER_PRIVATE_DATA_S))));
    
    (*obj)->keepMeAlive              = keepMeAlive_f;
    
    (*obj)->prepareCustomHeader      = prepareCustomHeader_f;
    (*obj)->parseCustomHeader        = parseCustomHeader_f;
    
    (*obj)->prepareCustomContent     = prepareCustomContent_f;
    (*obj)->parseCustomContent       = parseCustomContent_f;
    
    (*obj)->prepareHttpGet           = prepareHttpGet_f;
    (*obj)->parseHttpGet             = parseHttpGet_f;
    
    (*obj)->prepareHttp200Ok         = prepareHttp200Ok_f;
    (*obj)->parseHttp200Ok           = parseHttp200Ok_f;
    
    (*obj)->prepareHttp400BadRequest = prepareHttp400BadRequest_f;
    (*obj)->parseHttp400BadRequest   = parseHttp400BadRequest_f;
    
    (*obj)->prepareHttp404NotFound   = prepareHttp404NotFound_f;
    (*obj)->parseHttp404NotFound     = parseHttp404NotFound_f;
    
    (*obj)->prepareHttpContent       = prepareHttpContent_f;
    (*obj)->parseHttpContent         = parseHttpContent_f;
    
    (*obj)->getPeerName              = getPeerName_f;
    
    (*obj)->setBlocking              = setBlocking_f;
    (*obj)->isReadyForWriting        = isReadyForWriting_f;
    (*obj)->isReadyForReading        = isReadyForReading_f;
    
    (*obj)->readData                 = readData_f;
    (*obj)->writeData                = writeData_f;
    
    (*obj)->pData = (void*)pData;
}

/*!
 *
 */
void LinkHelper_UnInit(LINK_HELPER_S **obj)
{
    assert(obj && *obj);
    
    free((*obj)->pData);
    (*obj)->pData = NULL;
    
    free(*obj);
    *obj = NULL;
}

/* -------------------------------------------------------------------------------------------- */
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static void keepMeAlive_f(LINK_HELPER_S *obj)
{
    assert(obj);
    
    sigset_t         sigset;
    struct sigaction sa;
    
    (void)sigemptyset(&sigset);
	sa.sa_mask     = sigset;
	sa.sa_flags    = 0;
	sa.sa_restorer = NULL;
	sa.sa_handler  = signalHandler_f;
	
	(void)sigaction(SIGTERM, &sa, NULL);
	(void)sigaction(SIGPIPE, &sa, NULL);
}

/*!
 *
 */
static void prepareCustomHeader_f(LINK_HELPER_S *obj, CUSTOM_HEADER_S *inOut)
{
    assert(obj && inOut);
    
    memset(inOut->str, '\0', sizeof(inOut->str));
    strcpy(inOut->str, CUSTOM_HEADER);
}

/*!
 *
 */
static void parseCustomHeader_f(LINK_HELPER_S *obj, CUSTOM_HEADER_S *inOut)
{
    assert(obj && inOut);
}

/*!
 *
 */
static void prepareCustomContent_f(LINK_HELPER_S *obj, CUSTOM_CONTENT_S *inOut)
{
    assert(obj && inOut);
    
    memset(inOut->str, '\0', sizeof(inOut->str));
    sprintf(inOut->str, CUSTOM_CONTENT, inOut->mime, (uint32_t)inOut->maxBufferSize);
}

/*!
 *
 */
static void parseCustomContent_f(LINK_HELPER_S *obj, CUSTOM_CONTENT_S *inOut)
{
    assert(obj && inOut);
    
    memset(inOut->mime, '\0', sizeof(inOut->mime));
    sscanf(inOut->str, CUSTOM_CONTENT, inOut->mime, (uint32_t*)&inOut->maxBufferSize);
}

/*!
 *
 */
static void prepareHttpGet_f(LINK_HELPER_S *obj, HTTP_GET_S *inOut)
{
    assert(obj && inOut);
    
    uint8_t offset = 0;
    if (inOut->path[0] == '/') {
        offset = 1;
    }
    
    memset(inOut->str, '\0', sizeof(inOut->str));
    sprintf(inOut->str, HTTP_GET, inOut->path + offset, inOut->host, inOut->port);
}

/*!
 *
 */
static void parseHttpGet_f(LINK_HELPER_S *obj, HTTP_GET_S *inOut)
{
    assert(obj && inOut);
    
    memset(inOut->path, '\0', sizeof(inOut->path));
    memset(inOut->host, '\0', sizeof(inOut->host));
    
    const char * const GET  = "GET ";
    const char * const HTTP = " HTTP/";
    const char * const HOST = "HOST: ";
    
    if (strncmp(inOut->str, GET, strlen(GET))) {
        inOut->isHttpGet = 0;
        return;
    }
    
    inOut->isHttpGet = 1;
    
    char *str     = strdup(inOut->str);
    char *address = strstr(str, HOST);
    char *path    = strstr(str, HTTP);
    
    if (address) {
        char *host = address + strlen(address);
        char *port  = strstr(host, ":");
        if (port) {
            port[0] = '\0';
            inOut->port = (uint16_t)atoi(port + 1);
        }
        
        strcpy(inOut->host, host);
    }
    
    if (path) {
        path[0] = '\0';
        strcpy(inOut->path, str + strlen(GET));
    }
    
    free(str);
    str = NULL;
}

/*!
 *
 */
static void prepareHttp200Ok_f(LINK_HELPER_S *obj, HTTP_200_OK_S *inOut)
{
    assert(obj && inOut);
    
    memset(inOut->str, '\0', sizeof(inOut->str));
    strcpy(inOut->str, HTTP_200_OK);
}

/*!
 *
 */
static void parseHttp200Ok_f(LINK_HELPER_S *obj, HTTP_200_OK_S *inOut)
{
    assert(obj && inOut);
    
    inOut->is200Ok = (strstr(inOut->str, "200 OK") != NULL);
}

/*!
 *
 */
static void prepareHttp400BadRequest_f(LINK_HELPER_S *obj, HTTP_400_BAD_REQUEST_S *inOut)
{
    assert(obj && inOut);
    
    uint8_t offset = 0;
    if (inOut->path[0] == '/') {
        offset = 1;
    }
    
    memset(inOut->str, '\0', sizeof(inOut->str));
    sprintf(inOut->str, HTTP_400_BAD_REQUEST, inOut->ip, inOut->port, inOut->path + offset);
}

/*!
 *
 */
static void parseHttp400BadRequest_f(LINK_HELPER_S *obj, HTTP_400_BAD_REQUEST_S *inOut)
{
    assert(obj && inOut);
    
    inOut->is400BadRequest = (strstr(inOut->str, "400 Bad Request") != NULL);
}

/*!
 *
 */
static void prepareHttp404NotFound_f(LINK_HELPER_S *obj, HTTP_404_NOT_FOUND_S *inOut)
{
    assert(obj && inOut);
    
    uint8_t offset1 = 0, offset2 = 0;
    
    if ((inOut->requestedPath[0] == '/') && (strlen(inOut->requestedPath) == 1)) {
        offset1 = 1;
    }
    
    if (inOut->path[0] == '/') {
        offset2 = 1;
    }
    
    memset(inOut->str, '\0', sizeof(inOut->str));
    sprintf(inOut->str, HTTP_404_NOT_FOUND, inOut->requestedPath + offset1, inOut->ip, inOut->port, inOut->path + offset2);
}

/*!
 *
 */
static void parseHttp404NotFound_f(LINK_HELPER_S *obj, HTTP_404_NOT_FOUND_S *inOut)
{
    assert(obj && inOut);
    
    inOut->is404NotFound = (strstr(inOut->str, "404 Not Found") != NULL);
}

/*!
 *
 */
static void prepareHttpContent_f(LINK_HELPER_S *obj, HTTP_CONTENT_S *inOut)
{
    assert(obj && inOut);
    
    memset(inOut->str, '\0', sizeof(inOut->str));
    sprintf(inOut->str, HTTP_CONTENT, inOut->mime, (uint32_t)inOut->length);
}

/*!
 *
 */
static void parseHttpContent_f(LINK_HELPER_S *obj, HTTP_CONTENT_S *inOut)
{
    assert(obj && inOut);
    
    memset(inOut->mime, '\0', sizeof(inOut->mime));
    
    const char * const CONTENT_TYPE   = "Content-Type: ";
    const char * const CONTENT_LENGTH = "Content-Length: ";
    
    char *str = strdup(inOut->str);

    char *end = strstr(str, LF""CRLF);
    if (!end) {
        end = strstr(str, LFLF);
        if (!end) {
            return;
        }
        end[1] = '\0';
    }
    else {
        end[2] = '\0';
    }
    
    inOut->bodyStart = strlen(str) + 1;
    
    char *contentType   = strstr(str, CONTENT_TYPE);
    char *contentLength = strstr(str, CONTENT_LENGTH);
    
    char *strip;
    
    if (contentType) {
        strip = strstr(contentType, CRLF);
        if (strip) {
            strip[0] = '\0';
        }
        strcpy(inOut->mime, contentType);
    }
    
    if (contentLength) {
        strip = strstr(contentLength, CRLF);
        if (strip) {
            strip[0] = '\0';
        }
        inOut->length = (size_t)atoi(contentLength + strlen(CONTENT_LENGTH));
    }
    
    free(str);
    str = NULL;
}

/*!
 *
 */
static int8_t getPeerName_f(LINK_HELPER_S *obj, LINK_S *link, RECIPIENT_S *result)
{
    assert(obj && link && result);
    
    SOCKLEN_T len;
    SOCKADDR_STORAGE addr;

    len = sizeof(SOCKADDR_STORAGE);

    if (getpeername(link->sock, (SOCKADDR*)&addr, &len) < 0) {
        Loge("getpeername() failed - %s", strerror(errno));
        return ERROR;
    }

    if (addr.ss_family == AF_INET) {
        SOCKADDR_IN *s = (SOCKADDR_IN *)&addr;

        inet_ntop(AF_INET, &s->sin_addr, result->host, sizeof(result->host));
        sprintf(result->service, "%d", ntohs(s->sin_port));
    }
    else {
        SOCKADDR_IN6 *s = (SOCKADDR_IN6*)&addr;

        inet_ntop(AF_INET6, &s->sin6_addr, result->host, sizeof(result->host));
        sprintf(result->service, "%d", ntohs(s->sin6_port));
    }

    Logd("Peer IP address: %s", result->host);
    Logd("Peer port      : %s", result->service);

    return DONE;
}

/*!
 *
 */
static int8_t setBlocking_f(LINK_HELPER_S *obj, LINK_S *link, uint8_t blocking)
{
    assert(obj && link);
    
    LINK_HELPER_PRIVATE_DATA_S *pData = (LINK_HELPER_PRIVATE_DATA_S*)(obj->pData);
    assert(pData);
    
    if ((pData->sockFlags = fcntl(link->sock, F_GETFL, 0)) < 0) {
        return ERROR;
    }
    
    if (blocking) {
        pData->sockFlags &= ~O_NONBLOCK;
    }
    else {
        pData->sockFlags |= O_NONBLOCK;
    }
    
    if (fcntl(link->sock, F_SETFL, pData->sockFlags) < 0) {
        return ERROR;
    }
    
    return DONE;
}

/*!
 *
 */
static uint8_t isReadyForWriting_f(LINK_HELPER_S *obj, LINK_S *link, uint64_t timeout_ms)
{
    assert(obj && link);
    
    LINK_HELPER_PRIVATE_DATA_S *pData = (LINK_HELPER_PRIVATE_DATA_S*)(obj->pData);
    assert(pData);
    
    FD_ZERO(&pData->wfds);
    FD_SET(link->sock, &pData->wfds);
    
    pData->timeout.tv_sec  = 0;
    pData->timeout.tv_usec = timeout_ms * 1000;
    
    if (select(link->sock + 1, NULL, &pData->wfds, NULL, &pData->timeout) <= 0) {
        return NO;
    }
    
    if (FD_ISSET(link->sock, &pData->wfds)) {
        return YES;
    }
    
    return NO;
}

/*!
 *
 */
static uint8_t isReadyForReading_f(LINK_HELPER_S *obj, LINK_S *link, uint64_t timeout_ms)
{
    assert(obj && link);
    
    LINK_HELPER_PRIVATE_DATA_S *pData = (LINK_HELPER_PRIVATE_DATA_S*)(obj->pData);
    assert(pData);
    
    FD_ZERO(&pData->rfds);
    FD_SET(link->sock, &pData->rfds);
    
    pData->timeout.tv_sec  = 0;
    pData->timeout.tv_usec = timeout_ms * 1000;
    
    if (select(link->sock + 1, &pData->rfds, NULL, NULL, &pData->timeout) <= 0) {
        return NO;
    }
    
    if (FD_ISSET(link->sock, &pData->rfds)) {
        return YES;
    }
    
    return NO;
}

/*!
 *
 */
static int8_t readData_f(LINK_HELPER_S *obj, LINK_S *src, LINK_S *dst, BUFFER_S *buffer, ssize_t *nbRead)
{
    assert(obj && src && buffer);
    
    LINK_HELPER_PRIVATE_DATA_S *pData = (LINK_HELPER_PRIVATE_DATA_S*)(obj->pData);
    assert(pData);
    
    pData->rIov.iov_base = buffer->data;
    pData->rIov.iov_len  = buffer->length;

    pData->rMsg.msg_name       = dst ? dst->destAddress : NULL;
    pData->rMsg.msg_namelen    = dst ? dst->destAddressLength : 0;
    pData->rMsg.msg_iov        = &pData->rIov;
    pData->rMsg.msg_iovlen     = 1;
    pData->rMsg.msg_control    = NULL;
    pData->rMsg.msg_controllen = 0;
    
    pData->nbBytesReceived = recvmsg(src->sock, &pData->rMsg, 0);
    
    if (pData->nbBytesReceived == SOCKET_ERROR) {
        if (errno != EINTR) {
            Loge("Failed to receive data - %s", strerror(errno));
            return ERROR;
        }
        pData->nbBytesReceived = 0;
    }
    
    if (pData->nbBytesReceived < buffer->length) {
        ssize_t nbBytes = 0;
        
        do {
            if ((isReadyForReading_f(obj, src, WAIT_TIME_10MS) == NO)) {
                if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                    if (nbRead) {
                        *nbRead = pData->nbBytesReceived;
                    }
                    return BUSY;
                }
                Loge("Reading not possible - %s", strerror(errno));
                return ERROR;
            }
            
            pData->rIov.iov_base = buffer->data + pData->nbBytesReceived;
            pData->rIov.iov_len  = buffer->length - pData->nbBytesReceived;
            
            nbBytes = recvmsg(src->sock, &pData->rMsg, 0);
                
            if (nbBytes == SOCKET_ERROR) {
                Loge("Failed to receive data - %s", strerror(errno));
                break;
            }
            
            pData->nbBytesReceived += nbBytes;
        }
        while ((pData->nbBytesReceived < buffer->length) || (errno == EINTR));
        
        if (pData->nbBytesReceived < buffer->length) {
            Logw("Received : %ld bytes < Expected : %ld bytes", (int64_t)pData->nbBytesReceived, (int64_t)buffer->length);
        }
    }
    
    if (nbRead) {
        *nbRead = pData->nbBytesReceived;
    }
    
    ((int8_t*)buffer->data)[pData->nbBytesReceived] = '\0';
    
    return DONE;
}

/*!
 *
 */
static int8_t writeData_f(LINK_HELPER_S *obj, LINK_S *src, LINK_S *dst, BUFFER_S *buffer, ssize_t *nbWritten)
{
    assert(obj && src && buffer);
    
    LINK_HELPER_PRIVATE_DATA_S *pData = (LINK_HELPER_PRIVATE_DATA_S*)(obj->pData);
    assert(pData);
    
    pData->wIov.iov_base = buffer->data;
    pData->wIov.iov_len  = buffer->length;

    pData->wMsg.msg_name       = dst ? dst->destAddress : NULL;
    pData->wMsg.msg_namelen    = dst ? dst->destAddressLength : 0;
    pData->wMsg.msg_iov        = &pData->wIov;
    pData->wMsg.msg_iovlen     = 1;
    pData->wMsg.msg_control    = NULL;
    pData->wMsg.msg_controllen = 0;
    
    pData->nbBytesSent = sendmsg(src->sock, &pData->wMsg, 0);
    
    if (pData->nbBytesSent == SOCKET_ERROR) {
        if ((errno != EINTR) && (errno != EMSGSIZE)) {
            Loge("Failed to send data - %s", strerror(errno));
            return ERROR;
        }
        pData->nbBytesSent = 0;
    }
    
    if (pData->nbBytesSent < buffer->length) {
        uint8_t sendByBlock = (errno == EMSGSIZE);
        ssize_t nbBytes     = 0;
        
        do {
            if ((isReadyForWriting_f(obj, src, WAIT_TIME_10MS) == NO)) {
                if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                    if (nbWritten) {
                        *nbWritten = pData->nbBytesSent;
                    }
                    return BUSY;
                }
                Loge("Writing not possible - %s", strerror(errno));
                return ERROR;
            }
            
            pData->wIov.iov_base = buffer->data + pData->nbBytesSent;
            pData->wIov.iov_len  = sendByBlock ? MAX_BLOCK_SIZE : buffer->length - pData->nbBytesSent;
                
            nbBytes = sendmsg(src->sock, &pData->wMsg, 0);
                
            if (nbBytes == SOCKET_ERROR) {
                Loge("Failed to send data - %s", strerror(errno));
                break;
            }
            
            pData->nbBytesSent += nbBytes;
        }
        while ((pData->nbBytesSent < buffer->length) && (sendByBlock || (errno == EINTR)));
        
        if (pData->nbBytesSent < buffer->length) {
            Logw("Sent : %ld bytes < Expected : %ld bytes", (int64_t)pData->nbBytesSent, (int64_t)buffer->length);
        }
    }
    
    if (nbWritten) {
        *nbWritten = pData->nbBytesSent;
    }
    
    return DONE;
}

/*!
 *
 */
static void signalHandler_f(int32_t signalNumber)
{
    Logd("Signal caught - signalNumber = %d", signalNumber);
}
