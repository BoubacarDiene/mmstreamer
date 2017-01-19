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
* \file   LinkHelper.h
* \author Boubacar DIENE
*/

#ifndef __LINK_HELPER_H__
#define __LINK_HELPER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include "utils/Common.h"
#include "utils/List.h"
#include "utils/Log.h"
#include "utils/Task.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                             */
/* -------------------------------------------------------------------------------------------- */

#define NO              0
#define YES             1

#define ERROR          -1
#define DONE            0
#define BUSY            1

#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1

#define IPV4           "IPv4"
#define IPV6           "IPv6"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef enum   LINK_TYPE_E            LINK_TYPE_E;
typedef enum   LINK_MODE_E            LINK_MODE_E;
typedef enum   STATE_E                STATE_E;
typedef enum   STREAM_TYPE_E          STREAM_TYPE_E;

typedef struct CUSTOM_HEADER_S        CUSTOM_HEADER_S;
typedef struct CUSTOM_CONTENT_S       CUSTOM_CONTENT_S;
typedef struct HTTP_GET_S             HTTP_GET_S;
typedef struct HTTP_200_OK_S          HTTP_200_OK_S;
typedef struct HTTP_400_BAD_REQUEST_S HTTP_400_BAD_REQUEST_S;
typedef struct HTTP_404_NOT_FOUND_S   HTTP_404_NOT_FOUND_S;
typedef struct HTTP_CONTENT_S         HTTP_CONTENT_S;
typedef struct LINK_S                 LINK_S;
typedef struct LINK_HELPER_S          LINK_HELPER_S;

typedef struct sockaddr_in            SOCKADDR_IN;
typedef struct sockaddr_in6           SOCKADDR_IN6;
typedef struct sockaddr_storage       SOCKADDR_STORAGE;
typedef struct sockaddr_un            SOCKADDR_UN;
typedef struct sockaddr               SOCKADDR;

typedef struct addrinfo               ADDRINFO;

typedef struct iovec                  IOVEC;
typedef struct msghdr                 MSGHDR;

typedef int32_t                       SOCKET;
typedef socklen_t                     SOCKLEN_T;

typedef void (*LINK_HELPER_KEEP_ME_ALIVE_F)(LINK_HELPER_S *obj);

typedef void (*LINK_HELPER_PREPARE_CUSTOM_HEADER_F)(LINK_HELPER_S *obj, CUSTOM_HEADER_S *inOut);
typedef void (*LINK_HELPER_PARSE_CUSTOM_HEADER_F  )(LINK_HELPER_S *obj, CUSTOM_HEADER_S *inOut);

typedef void (*LINK_HELPER_PREPARE_CUSTOM_CONTENT_F)(LINK_HELPER_S *obj, CUSTOM_CONTENT_S *inOut);
typedef void (*LINK_HELPER_PARSE_CUSTOM_CONTENT_F  )(LINK_HELPER_S *obj, CUSTOM_CONTENT_S *inOut);

typedef void (*LINK_HELPER_PREPARE_HTTP_GET_F)(LINK_HELPER_S *obj, HTTP_GET_S *inOut);
typedef void (*LINK_HELPER_PARSE_HTTP_GET_F  )(LINK_HELPER_S *obj, HTTP_GET_S *inOut);

typedef void (*LINK_HELPER_PREPARE_HTTP_200_OK_F)(LINK_HELPER_S *obj, HTTP_200_OK_S *inOut);
typedef void (*LINK_HELPER_PARSE_HTTP_200_OK_F  )(LINK_HELPER_S *obj, HTTP_200_OK_S *inOut);

typedef void (*LINK_HELPER_PREPARE_HTTP_400_BAD_REQUEST_F)(LINK_HELPER_S *obj, HTTP_400_BAD_REQUEST_S *inOut);
typedef void (*LINK_HELPER_PARSE_HTTP_400_BAD_REQUEST_F  )(LINK_HELPER_S *obj, HTTP_400_BAD_REQUEST_S *inOut);

typedef void (*LINK_HELPER_PREPARE_HTTP_404_NOT_FOUND_F)(LINK_HELPER_S *obj, HTTP_404_NOT_FOUND_S *inOut);
typedef void (*LINK_HELPER_PARSE_HTTP_404_NOT_FOUND_F  )(LINK_HELPER_S *obj, HTTP_404_NOT_FOUND_S *inOut);

typedef void (*LINK_HELPER_PREPARE_HTTP_CONTENT_F)(LINK_HELPER_S *obj, HTTP_CONTENT_S *inOut);
typedef void (*LINK_HELPER_PARSE_HTTP_CONTENT_F  )(LINK_HELPER_S *obj, HTTP_CONTENT_S *inOut);

typedef int8_t (*LINK_HELPER_GET_PEER_NAME_F)(LINK_HELPER_S *obj, LINK_S *link, RECIPIENT_S *result);

typedef int8_t  (*LINK_HELPER_SET_BLOCKING_F        )(LINK_HELPER_S *obj, LINK_S *link, uint8_t blocking);
typedef uint8_t (*LINK_HELPER_IS_READY_FOR_WRITING_F)(LINK_HELPER_S *obj, LINK_S *link, uint64_t timeout_ms);
typedef uint8_t (*LINK_HELPER_IS_READY_FOR_READING_F)(LINK_HELPER_S *obj, LINK_S *link, uint64_t timeout_ms);

typedef int8_t (*LINK_HELPER_READ_DATA_F )(LINK_HELPER_S *obj, LINK_S *src, LINK_S *dst, BUFFER_S *buffer, ssize_t *nbRead);
typedef int8_t (*LINK_HELPER_WRITE_DATA_F)(LINK_HELPER_S *obj, LINK_S *src, LINK_S *dst, BUFFER_S *buffer, ssize_t *nbWritten);

enum LINK_TYPE_E {
    LINK_TYPE_INET_STREAM,
    LINK_TYPE_INET_DGRAM,
    LINK_TYPE_UNIX_STREAM,
    LINK_TYPE_UNIX_DGRAM
};

enum LINK_MODE_E {
    LINK_MODE_STANDARD, // Standard
    LINK_MODE_HTTP,     // Http
    LINK_MODE_CUSTOM    // Custom
};

enum STATE_E {
    STATE_DISCONNECTED,
    STATE_CONNECTED
};

enum STREAM_TYPE_E {
    STREAM_TYPE_VIDEO,
    STREAM_TYPE_MAX
};

struct CUSTOM_HEADER_S {
    char   str[MAX_HEADER_SIZE];
};

struct CUSTOM_CONTENT_S {
    char   mime[MAX_MIME_SIZE];
    size_t maxBufferSize;
    
    char   str[MAX_HEADER_SIZE];
};

struct HTTP_GET_S {
    uint8_t  isHttpGet;
    
    char     path[MAX_PATH_SIZE];
    char     host[MAX_ADDRESS_SIZE];
    uint16_t port;
    
    char     str[MAX_STR_SIZE];
};

struct HTTP_200_OK_S {
    uint8_t  is200Ok;
    
    char     str[MAX_HEADER_SIZE];
};

struct HTTP_400_BAD_REQUEST_S {
    uint8_t  is400BadRequest;
    
    char     ip[MAX_ADDRESS_SIZE];
    uint16_t port;
    char     path[MAX_PATH_SIZE];
    
    char     str[MAX_HEADER_SIZE];
};

struct HTTP_404_NOT_FOUND_S {
    uint8_t  is404NotFound;
    
    char     ip[MAX_ADDRESS_SIZE];
    uint16_t port;
    char     path[MAX_PATH_SIZE];
    
    char     requestedPath[MAX_PATH_SIZE];
    
    char     str[MAX_HEADER_SIZE];
};

struct HTTP_CONTENT_S {
    char    mime[MAX_MIME_SIZE];
    size_t  length;
    
    int32_t bodyStart;
    
    char    str[MAX_HEADER_SIZE];
};

struct LINK_S {
    uint32_t             id;
    
    SOCKET               sock;
    int32_t              domain;
    int32_t              type;
    
    union {
        SOCKADDR_STORAGE storage;
        SOCKADDR_UN      sun;
    } addr;
    
    uint8_t              useDestAddress;
    SOCKADDR             *destAddress;
    SOCKLEN_T            destAddressLength;
    
    void                 *pData;
};

struct LINK_HELPER_S {
    LINK_HELPER_KEEP_ME_ALIVE_F                keepMeAlive;
    
    LINK_HELPER_PREPARE_CUSTOM_HEADER_F        prepareCustomHeader;
    LINK_HELPER_PARSE_CUSTOM_HEADER_F          parseCustomHeader;
    
    LINK_HELPER_PREPARE_CUSTOM_CONTENT_F       prepareCustomContent;
    LINK_HELPER_PARSE_CUSTOM_CONTENT_F         parseCustomContent;
    
    LINK_HELPER_PREPARE_HTTP_GET_F             prepareHttpGet;
    LINK_HELPER_PARSE_HTTP_GET_F               parseHttpGet;
    
    LINK_HELPER_PREPARE_HTTP_200_OK_F          prepareHttp200Ok;
    LINK_HELPER_PARSE_HTTP_200_OK_F            parseHttp200Ok;
    
    LINK_HELPER_PREPARE_HTTP_400_BAD_REQUEST_F prepareHttp400BadRequest;
    LINK_HELPER_PARSE_HTTP_400_BAD_REQUEST_F   parseHttp400BadRequest;
    
    LINK_HELPER_PREPARE_HTTP_404_NOT_FOUND_F   prepareHttp404NotFound;
    LINK_HELPER_PARSE_HTTP_404_NOT_FOUND_F     parseHttp404NotFound;
    
    LINK_HELPER_PREPARE_HTTP_CONTENT_F         prepareHttpContent;
    LINK_HELPER_PARSE_HTTP_CONTENT_F           parseHttpContent;
    
    LINK_HELPER_GET_PEER_NAME_F                getPeerName;
    
    LINK_HELPER_SET_BLOCKING_F                 setBlocking;
    LINK_HELPER_IS_READY_FOR_WRITING_F         isReadyForWriting;
    LINK_HELPER_IS_READY_FOR_READING_F         isReadyForReading;
    
    LINK_HELPER_READ_DATA_F                    readData;
    LINK_HELPER_WRITE_DATA_F                   writeData;
    
    void                                       *pData;
};

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

void LinkHelper_Init  (LINK_HELPER_S **obj);
void LinkHelper_UnInit(LINK_HELPER_S **obj);

#ifdef __cplusplus
}
#endif

#endif //__LINK_HELPER_H__
