//////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                              //
//              Copyright © 2016, 2018 Boubacar DIENE                                           //
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
* \file LinkHelper.h
* \author Boubacar DIENE
*/

#ifndef __LINK_HELPER_H__
#define __LINK_HELPER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
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
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
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
/* //////////////////////////////////// TYPES DECLARATION ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum link_type_e;
enum link_mode_e;
enum state_e;
enum stream_type_e;

struct custom_header_s;
struct custom_content_s;
struct http_get_s;
struct http_200_ok_s;
struct http_400_bad_request_s;
struct http_404_not_found_s;
struct http_content_s;
struct link_s;
struct link_helper_s;

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef void (*link_helper_keep_me_alive_f)(struct link_helper_s *obj);

typedef void (*link_helper_prepare_custom_header_f)(struct link_helper_s *obj,
                                                    struct custom_header_s *inOut);
typedef void (*link_helper_parse_custom_header_f)(struct link_helper_s *obj,
                                                  struct custom_header_s *inOut);

typedef void (*link_helper_prepare_custom_content_f)(struct link_helper_s *obj,
                                                     struct custom_content_s *inOut);
typedef void (*link_helper_parse_custom_content_f)(struct link_helper_s *obj,
                                                   struct custom_content_s *inOut);

typedef void (*link_helper_prepare_http_get_f)(struct link_helper_s *obj,
                                               struct http_get_s *inOut);
typedef void (*link_helper_parse_http_get_f)(struct link_helper_s *obj,
                                             struct http_get_s *inOut);

typedef void (*link_helper_prepare_http_200_ok_f)(struct link_helper_s *obj,
                                                  struct http_200_ok_s *inOut);
typedef void (*link_helper_parse_http_200_ok_f)(struct link_helper_s *obj,
                                                struct http_200_ok_s *inOut);

typedef void (*link_helper_prepare_http_400_bad_request_f)(struct link_helper_s *obj,
                                                           struct http_400_bad_request_s *inOut);
typedef void (*link_helper_parse_http_400_bad_request_f)(struct link_helper_s *obj,
                                                         struct http_400_bad_request_s *inOut);

typedef void (*link_helper_prepare_http_404_not_found_f)(struct link_helper_s *obj,
                                                         struct http_404_not_found_s *inOut);
typedef void (*link_helper_parse_http_404_not_found_f)(struct link_helper_s *obj,
                                                       struct http_404_not_found_s *inOut);

typedef void (*link_helper_prepare_http_content_f)(struct link_helper_s *obj,
                                                   struct http_content_s *inOut);
typedef void (*link_helper_parse_http_content_f)(struct link_helper_s *obj,
                                                 struct http_content_s *inOut);

typedef int8_t (*link_helper_get_peer_name_f)(struct link_helper_s *obj, struct link_s *link,
                                              struct recipient_s *result);

typedef int8_t (*link_helper_set_blocking_f)(struct link_helper_s *obj, struct link_s *link,
                                             uint8_t blocking);
typedef uint8_t (*link_helper_is_ready_for_writing_f)(struct link_helper_s *obj,
                                                      struct link_s *link, uint64_t timeout_ms);
typedef uint8_t (*link_helper_is_ready_for_reading_f)(struct link_helper_s *obj,
                                                      struct link_s *link, uint64_t timeout_ms);

typedef int8_t (*link_helper_read_data_f)(struct link_helper_s *obj, struct link_s *src,
                                          struct link_s *dst, struct buffer_s *buffer,
                                          ssize_t *nbRead);
typedef int8_t (*link_helper_write_data_f)(struct link_helper_s *obj, struct link_s *src,
                                           struct link_s *dst, struct buffer_s *buffer,
                                           ssize_t *nbWritten);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum link_type_e {
    LINK_TYPE_INET_STREAM,
    LINK_TYPE_INET_DGRAM,
    LINK_TYPE_UNIX_STREAM,
    LINK_TYPE_UNIX_DGRAM
};

enum link_mode_e {
    LINK_MODE_STANDARD,
    LINK_MODE_HTTP,
    LINK_MODE_CUSTOM
};

enum state_e {
    STATE_DISCONNECTED,
    STATE_CONNECTED
};

enum stream_type_e {
    STREAM_TYPE_VIDEO,
    STREAM_TYPE_MAX
};

struct custom_header_s {
    char str[MAX_HEADER_SIZE];
};

struct custom_content_s {
    char   mime[MAX_MIME_SIZE];
    size_t maxBufferSize;
    
    char   str[MAX_HEADER_SIZE];
};

struct http_get_s {
    uint8_t  isHttpGet;
    
    char     path[MAX_PATH_SIZE];
    char     host[MAX_ADDRESS_SIZE];
    uint16_t port;
    
    char     str[MAX_STR_SIZE];
};

struct http_200_ok_s {
    uint8_t  is200Ok;
    
    char     str[MAX_HEADER_SIZE];
};

struct http_400_bad_request_s {
    uint8_t  is400BadRequest;
    
    char     ip[MAX_ADDRESS_SIZE];
    uint16_t port;
    char     path[MAX_PATH_SIZE];
    
    char     str[MAX_HEADER_SIZE];
};

struct http_404_not_found_s {
    uint8_t  is404NotFound;
    
    char     ip[MAX_ADDRESS_SIZE];
    uint16_t port;
    char     path[MAX_PATH_SIZE];
    
    char     requestedPath[MAX_PATH_SIZE];
    
    char     str[MAX_HEADER_SIZE];
};

struct http_content_s {
    char    mime[MAX_MIME_SIZE];
    size_t  length;
    
    int32_t bodyStart;
    
    char    str[MAX_HEADER_SIZE];
};

struct link_s {
    uint32_t                    id;
    
    int32_t                     sock;
    int32_t                     domain;
    int32_t                     type;
    
    union {
        struct sockaddr_storage storage;
        struct sockaddr_un      sun;
    } addr;
    
    uint8_t                     useDestAddress;
    struct sockaddr             *destAddress;
    socklen_t                   destAddressLength;
    
    void                        *pData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// MAIN CONTEXT /////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct link_helper_s {
    link_helper_keep_me_alive_f                keepMeAlive;

    link_helper_prepare_custom_header_f        prepareCustomHeader;
    link_helper_parse_custom_header_f          parseCustomHeader;

    link_helper_prepare_custom_content_f       prepareCustomContent;
    link_helper_parse_custom_content_f         parseCustomContent;

    link_helper_prepare_http_get_f             prepareHttpGet;
    link_helper_parse_http_get_f               parseHttpGet;

    link_helper_prepare_http_200_ok_f          prepareHttp200Ok;
    link_helper_parse_http_200_ok_f            parseHttp200Ok;

    link_helper_prepare_http_400_bad_request_f prepareHttp400BadRequest;
    link_helper_parse_http_400_bad_request_f   parseHttp400BadRequest;

    link_helper_prepare_http_404_not_found_f   prepareHttp404NotFound;
    link_helper_parse_http_404_not_found_f     parseHttp404NotFound;

    link_helper_prepare_http_content_f         prepareHttpContent;
    link_helper_parse_http_content_f           parseHttpContent;

    link_helper_get_peer_name_f                getPeerName;

    link_helper_set_blocking_f                 setBlocking;
    link_helper_is_ready_for_writing_f         isReadyForWriting;
    link_helper_is_ready_for_reading_f         isReadyForReading;

    link_helper_read_data_f                    readData;
    link_helper_write_data_f                   writeData;
    
    void *pData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

void LinkHelper_Init(struct link_helper_s **obj);
void LinkHelper_UnInit(struct link_helper_s **obj);

#ifdef __cplusplus
}
#endif

#endif //__LINK_HELPER_H__
