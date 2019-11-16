//////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                              //
//              Copyright © 2016, 2018 Boubacar DIENE                                           //
//                                                                                              //
//              This file is part of mmstreamer project.                                        //
//                                                                                              //
//              mmstreamer is free software: you can redistribute it and/or modify              //
//              it under the terms of the GNU General Public License as published by            //
//              the Free Software Foundation, either version 2 of the License, or               //
//              (at your option) any later version.                                             //
//                                                                                              //
//              mmstreamer is distributed in the hope that it will be useful,                   //
//              but WITHOUT ANY WARRANTY; without even the implied warranty of                  //
//              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   //
//              GNU General Public License for more details.                                    //
//                                                                                              //
//              You should have received a copy of the GNU General Public License               //
//              along with mmstreamer. If not, see <http://www.gnu.org/licenses/>               //
//              or write to the Free Software Foundation, Inc., 51 Franklin Street,             //
//              51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.                   //
//                                                                                              //
//////////////////////////////////////////////////////////////////////////////////////////////////

/*!
* \file Client.c
* \brief Clients management
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "network/Client.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "Client"

#define WATCHER_TASK_NAME  "client-WatcherTask"
#define RECEIVER_TASK_NAME "client-ReceiverTask"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct client_context_s {
    volatile uint8_t        quit;
    
    struct client_params_s  params;
    
    struct link_s           *client;
    struct link_s           *server;
    
    struct custom_header_s  customHeader;
    struct custom_content_s customContent;

    struct http_get_s       httpGet;
    struct http_200_ok_s    http200Ok;
    struct http_content_s   httpContent;
    struct buffer_s         httpBuffer;
    size_t                  nbBodyRead;
    
    struct buffer_s         bufferIn;
    struct buffer_s         bufferOut;
    ssize_t                 nbRead;
    
    sem_t                   sem;
    pthread_mutex_t         lock;
    
    struct task_s           *clientTask;
    struct task_params_s    watcherTaskParams;
    struct task_params_s    receiverTaskParams;
    
    struct addrinfo         hints;
    struct addrinfo         *result;
    struct addrinfo         *rp;
    void                    *addr;
    char                    *ipver;
    char                    ipstr[MAX_ADDRESS_SIZE];
    uint16_t                port;
    union {
        struct sockaddr_in  *v4;
        struct sockaddr_in6 *v6;
    } ip;
    
    uint8_t                 ackReceived;
};

struct client_private_data_s {
    struct link_helper_s *linkHelper;
    struct list_s        *clientsList;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static enum client_error_e start_f(struct client_s *obj, struct client_params_s *params);
static enum client_error_e stop_f(struct client_s *obj, struct client_params_s *params);

static enum client_error_e sendData_f(struct client_s *obj, struct client_params_s *params,
                                      struct buffer_s *buffer);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PRIVATE FUNCTIONS PROTOTYPES /////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static enum client_error_e openClientSocket_f(struct client_context_s *ctx,
                                              struct link_helper_s *linkHelper);
static enum client_error_e closeClientSocket_f(struct client_context_s *ctx);
static enum client_error_e getClientContext_f(struct client_s *obj, char *clientName,
                                              struct client_context_s **ctxOut, uint8_t lock);

static void watcherTaskFct_f(struct task_params_s *params);
static void receiverTaskFct_f(struct task_params_s *params);

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static uint8_t compareCb(struct list_s *obj, void *elementToCheck, void *userData);
static void releaseCb(struct list_s *obj, void *element);

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum client_error_e Client_Init(struct client_s **obj)
{
    ASSERT(obj && (*obj = calloc(1, sizeof(struct client_s))));

    struct client_private_data_s *pData;
    ASSERT((pData = calloc(1, sizeof(struct client_private_data_s))));
    
    LinkHelper_Init(&pData->linkHelper);
    ASSERT(pData->linkHelper);
    
    struct list_params_s listParams = {0};
    listParams.compareCb = compareCb;
    listParams.releaseCb = releaseCb;
    listParams.browseCb  = NULL;
    
    if (List_Init(&pData->clientsList, &listParams) != LIST_ERROR_NONE) {
        Loge("List_Init() failed");
        goto exit;
    }
    
    (*obj)->start    = start_f;
    (*obj)->stop     = stop_f;
    
    (*obj)->sendData = sendData_f;
    
    (*obj)->pData    = (void*)pData;
    
    pData->linkHelper->keepMeAlive(pData->linkHelper);
    
    return CLIENT_ERROR_NONE;
    
exit:
    LinkHelper_UnInit(&pData->linkHelper);

    free(pData);
    free(*obj);
    *obj = NULL;
    
    return CLIENT_ERROR_INIT;
}

/*!
 *
 */
enum client_error_e Client_UnInit(struct client_s **obj)
{
    ASSERT(obj && *obj && (*obj)->pData);
    
    struct client_private_data_s *pData = (struct client_private_data_s*)((*obj)->pData);
    
    LinkHelper_UnInit(&pData->linkHelper);
    
    (void)List_UnInit(&pData->clientsList);
    
    free(pData);
    free(*obj);
    *obj = NULL;
    
    return CLIENT_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static enum client_error_e start_f(struct client_s *obj, struct client_params_s *params)
{
    ASSERT(obj && obj->pData && params);

    struct client_context_s *ctx = NULL;

    /* Check if client is already started or not */
    if (getClientContext_f(obj, params->name, &ctx, 1) == CLIENT_ERROR_NONE) {
        Loge("%s's context exists", params->name);
        return CLIENT_ERROR_START;
    }

    /* Init context */
    ASSERT((ctx = calloc(1, sizeof(struct client_context_s))));
    ctx->params = *params;
    
    /* Init socket */
    struct client_private_data_s *pData = (struct client_private_data_s*)(obj->pData);

    if (openClientSocket_f(ctx, pData->linkHelper) != CLIENT_ERROR_NONE) {
        Loge("openClientSocket_f() failed");
        goto exit;
    }
    
    /* Init sem and mutex */
    if (sem_init(&ctx->sem, 0, 0) != 0) {
        Loge("sem_init() failed");
        goto sem_exit;
    }
    
    if (pthread_mutex_init(&ctx->lock, NULL) != 0) {
        Loge("pthread_mutex_init() failed");
        goto mutex_exit;
    }
    
    /* Init and start tasks */
    if (Task_Init(&ctx->clientTask) != TASK_ERROR_NONE) {
        Loge("Task_Init() failed");
        goto task_init_exit;
    }
    
    // Watcher
    snprintf(ctx->watcherTaskParams.name, sizeof(ctx->watcherTaskParams.name),
                "%s-%u.%u", WATCHER_TASK_NAME, params->type, params->link);
    ctx->watcherTaskParams.priority = params->priority;
    ctx->watcherTaskParams.fct      = watcherTaskFct_f;
    ctx->watcherTaskParams.fctData  = ctx;
    ctx->watcherTaskParams.userData = pData;
    ctx->watcherTaskParams.atExit   = NULL;
    
    if (ctx->clientTask->create(ctx->clientTask, &ctx->watcherTaskParams) != TASK_ERROR_NONE) {
        Loge("Failed to create watcherTask");
        goto watcher_create_exit;
    }
    
    // Receiver
    snprintf(ctx->receiverTaskParams.name, sizeof(ctx->receiverTaskParams.name),
                "%s-%u.%u", RECEIVER_TASK_NAME, params->type, params->link);
    ctx->receiverTaskParams.priority = params->priority;
    ctx->receiverTaskParams.fct      = receiverTaskFct_f;
    ctx->receiverTaskParams.fctData  = ctx;
    ctx->receiverTaskParams.userData = pData;
    ctx->receiverTaskParams.atExit   = NULL;
    
    if (ctx->clientTask->create(ctx->clientTask, &ctx->receiverTaskParams) != TASK_ERROR_NONE) {
        Loge("Failed to create receiverTask");
        goto receiver_create_exit;
    }
    
    /* Add client's ctx to list */
    if (!pData->clientsList
        || (pData->clientsList->lock(pData->clientsList) != LIST_ERROR_NONE)) {
        Loge("Failed to lock clientsList");
        goto list_exit;
    }
    pData->clientsList->add(pData->clientsList, (void*)ctx);
    (void)pData->clientsList->unlock(pData->clientsList);
    
    //Start
    (void)ctx->clientTask->start(ctx->clientTask, &ctx->watcherTaskParams);
    (void)ctx->clientTask->start(ctx->clientTask, &ctx->receiverTaskParams);
    
    return CLIENT_ERROR_NONE;

list_exit:
    (void)ctx->clientTask->destroy(ctx->clientTask, &ctx->receiverTaskParams);

receiver_create_exit:
    (void)ctx->clientTask->destroy(ctx->clientTask, &ctx->watcherTaskParams);

watcher_create_exit:
    (void)Task_UnInit(&ctx->clientTask);

task_init_exit:
    (void)pthread_mutex_destroy(&ctx->lock);

mutex_exit:
    (void)sem_destroy(&ctx->sem);

sem_exit:
    (void)closeClientSocket_f(ctx);

exit:
    free(ctx);

    return CLIENT_ERROR_START;
}

/*!
 *
 */
static enum client_error_e stop_f(struct client_s *obj, struct client_params_s *params)
{
    ASSERT(obj && obj->pData && params);

    struct client_context_s *ctx = NULL;
    enum client_error_e ret      = CLIENT_ERROR_NONE;

    /* Ensure that the client is started */
    if ((ret = getClientContext_f(obj, params->name, &ctx, 1)) != CLIENT_ERROR_NONE) {
        Loge("%s's context does not exist", params->name);
        goto exit;
    }

    struct client_private_data_s *pData = (struct client_private_data_s*)(obj->pData);

    if (!pData->clientsList
        || pData->clientsList->lock(pData->clientsList) != LIST_ERROR_NONE) {
        Loge("Failed to lock clientsList");
        ret = CLIENT_ERROR_LOCK;
        goto exit;
    }

    (void)pData->clientsList->remove(pData->clientsList, (void*)params->name);
    (void)pData->clientsList->unlock(pData->clientsList);

exit:
    return ret;
}

/*!
 *
 */
static enum client_error_e sendData_f(struct client_s *obj, struct client_params_s *params,
                                      struct buffer_s *buffer)
{
    ASSERT(obj && obj->pData && params && buffer);

    struct client_private_data_s *pData = (struct client_private_data_s*)(obj->pData);
    
    if (!pData->clientsList) {
        Loge("No element in list");
        return CLIENT_ERROR_LIST;
    }
    
    if (pData->clientsList->lock(pData->clientsList) != LIST_ERROR_NONE) {
        Loge("Failed to lock clientsList");
        return CLIENT_ERROR_LIST;
    }
    
    // Retrieve client context
    struct client_context_s *ctx = NULL;
    enum client_error_e ret      = CLIENT_ERROR_NONE;

    if ((ret = getClientContext_f(obj, params->name, &ctx, 0)) != CLIENT_ERROR_NONE) {
        Loge("%s's context exists", params->name);
        goto exit;
    }
    
    // Send data to server
    if (pData->linkHelper->writeData(pData->linkHelper,
                                     ctx->client, ctx->server, buffer, NULL) == ERROR) {
        Loge("Failed to send data to server");
    }

exit:
    (void)pData->clientsList->unlock(pData->clientsList);
        
    return ret;
}

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////// PRIVATE FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static enum client_error_e openClientSocket_f(struct client_context_s *ctx,
                                              struct link_helper_s *linkHelper)
{
    ASSERT(ctx && linkHelper);
    
    if ((ctx->params.mode == LINK_MODE_HTTP) && (ctx->params.link != LINK_TYPE_INET_STREAM)) {
        Loge("Bad config : mode = HTTP but link != INET_STREAM");
        return CLIENT_ERROR_PARAMS;
    }
    
    ASSERT((ctx->client = calloc(1, sizeof(struct link_s)))
            && (ctx->server = calloc(1, sizeof(struct link_s))));
    
    ctx->client->domain = ((ctx->params.link == LINK_TYPE_INET_STREAM)
                          || (ctx->params.link == LINK_TYPE_INET_DGRAM)) ? AF_UNSPEC : AF_UNIX;
    ctx->client->type = ((ctx->params.link == LINK_TYPE_INET_STREAM)
                        || (ctx->params.link == LINK_TYPE_UNIX_STREAM)) ? SOCK_STREAM : SOCK_DGRAM;
    
    if (ctx->client->domain != AF_UNIX) {
        ctx->hints.ai_family   = ctx->client->domain;
        ctx->hints.ai_socktype = ctx->server->type;

        int status = getaddrinfo(ctx->params.recipient.server.host,
                                 ctx->params.recipient.server.service, &ctx->hints, &ctx->result);
        if (status != 0) {
            Loge("getaddrinfo() failed - %s", gai_strerror(status));
            goto getaddrinfo_exit;
        }
    }

next_addr:
    if (ctx->client->domain != AF_UNIX) {
        ctx->rp = ctx->rp ? ctx->rp->ai_next : ctx->result;
        
        if (!ctx->rp) {
            Loge("No address succeeded");
            goto freeaddrinfo_exit;
        }
        
        ctx->client->domain = ctx->rp->ai_family;
        ctx->client->type   = ctx->rp->ai_socktype;
        
        if (ctx->client->domain != AF_UNIX) {
            ctx->ip.v4  = (struct sockaddr_in*)ctx->rp->ai_addr;
            ctx->addr   = &(ctx->ip.v4->sin_addr);
            ctx->port   = (uint16_t)ntohs(ctx->ip.v4->sin_port);
            ctx->ipver  = IPV4;
        }
        else {
            ctx->ip.v6  = (struct sockaddr_in6*)ctx->rp->ai_addr;
            ctx->addr   = &(ctx->ip.v6->sin6_addr);
            ctx->port   = (uint16_t)ntohs(ctx->ip.v6->sin6_port);
            ctx->ipver  = IPV6;
        }
        
        inet_ntop(ctx->client->domain, ctx->addr, ctx->ipstr, sizeof(ctx->ipstr));
        Logd("Testing : %s - address : %s / port : %d", ctx->ipver, ctx->ipstr, ctx->port);
    }
    
    if ((ctx->client->sock = socket(ctx->client->domain,
                                    ctx->client->type, 0)) == INVALID_SOCKET) {
        if (ctx->client->domain != AF_UNIX) {
            goto next_addr;
        }
        Loge("Failed to create client socket");
        goto freeaddrinfo_exit;
    }
    
    /* Init server's data */
    if (ctx->client->domain != AF_UNIX) {
        if (!strcmp(ctx->ipver, IPV4)) {
            ctx->server->destAddress       = (struct sockaddr*)(ctx->ip.v4);
            ctx->server->destAddressLength = sizeof(*(ctx->ip.v4));
        }
        else {
            ctx->server->destAddress       = (struct sockaddr*)(ctx->ip.v6);
            ctx->server->destAddressLength = sizeof(*(ctx->ip.v6));
        }
    }
    else {
        ctx->server->addr.sun.sun_family = AF_UNIX;
        snprintf(ctx->server->addr.sun.sun_path + 1,
                    sizeof(ctx->server->addr.sun.sun_path) - 1,
                    "s%u%u%u-%s",
                    ctx->params.type,
                    ctx->params.link,
                    ctx->params.mode,
                    ctx->params.recipient.serverSocketName);
                    
        ctx->server->destAddressLength = sizeof(ctx->server->addr.sun);
        ctx->server->destAddress       = (struct sockaddr*)&ctx->server->addr.sun;
    }
    
    /* Connect to server if required */
    if (ctx->client->type == SOCK_STREAM) { // Connection oriented
        if ((connect(ctx->client->sock, ctx->server->destAddress,
                                        ctx->server->destAddressLength) == SOCKET_ERROR)
                                        && (errno != EINPROGRESS)) {
            if (ctx->client->domain != AF_UNIX) {
                close(ctx->client->sock);
                goto next_addr;
            }
            Loge("Failed to connect to server");
            goto freeaddrinfo_exit;
        }
        
        if (linkHelper->setBlocking(linkHelper, ctx->client, NO) == ERROR) {
            Loge("Failed to set client as non-blocking");
        }

        ctx->server->destAddress       = NULL;
        ctx->server->destAddressLength = 0;
    }
    else {
        ctx->server->useDestAddress = 1;
        
        if (ctx->client->domain == AF_UNIX) {
            ctx->client->addr.sun.sun_family = AF_UNIX;
            snprintf(ctx->client->addr.sun.sun_path + 1,
                        sizeof(ctx->client->addr.sun.sun_path) - 1,
                    "c%u%u%u-%s",
                    ctx->params.type,
                    ctx->params.link,
                    ctx->params.mode,
                    ctx->params.recipient.serverSocketName);
                    
            unlink(ctx->client->addr.sun.sun_path);
            if (bind(ctx->client->sock, (struct sockaddr*)&(ctx->client->addr.sun),
                                        sizeof(ctx->client->addr.sun)) == SOCKET_ERROR) {
                Loge("Failed to bind to client socket");
                goto freeaddrinfo_exit;
            }
        }
    }
    
    /* Exchange first data with server to finalize initialization */
    struct buffer_s buffer;
    if (ctx->params.mode == LINK_MODE_HTTP) {
        strcpy(ctx->httpGet.path, ctx->params.recipient.server.path);
        strcpy(ctx->httpGet.host, ctx->ipstr);
        ctx->httpGet.port = ctx->port;
        
        linkHelper->prepareHttpGet(linkHelper, &ctx->httpGet);
        Logd("Http Get : %s", ctx->httpGet.str);
        
        if (linkHelper->isReadyForWriting(linkHelper, ctx->client, WAIT_TIME_10MS) == NO) {
            Loge("Server not ready for writing");
            goto freeaddrinfo_exit;
        }
        
        buffer.data   = (void*)ctx->httpGet.str;
        buffer.length = strlen(ctx->httpGet.str);
            
        if (linkHelper->writeData(linkHelper, ctx->client, NULL, &buffer, NULL) == ERROR) {
            Loge("Failed to send data to server");
            goto freeaddrinfo_exit;
        }
    }
    else if ((ctx->client->type == SOCK_DGRAM) || (ctx->params.mode == LINK_MODE_CUSTOM)) {
        if (linkHelper->isReadyForWriting(linkHelper, ctx->client, WAIT_TIME_10MS) == NO) {
            Loge("Server not ready for writing");
            goto freeaddrinfo_exit;
        }
            
        linkHelper->prepareCustomHeader(linkHelper, &ctx->customHeader); 
        Logd("Custom header : %s", ctx->customHeader.str);
        
        buffer.data   = (void*)ctx->customHeader.str;
        buffer.length = strlen(ctx->customHeader.str);
            
        if (linkHelper->writeData(linkHelper, ctx->client, ctx->server, &buffer, NULL) == ERROR) {
            Loge("Failed to send data to server");
            goto freeaddrinfo_exit;
        }
    }
    
    if (ctx->client->domain != AF_UNIX) {
        inet_ntop(ctx->client->domain, ctx->addr, ctx->ipstr, sizeof(ctx->ipstr));
        Logd("Using : %s - address : %s / port : %u", ctx->ipver, ctx->ipstr, ctx->port);
        
        freeaddrinfo(ctx->result);
        ctx->result = NULL;
    }
    
    return CLIENT_ERROR_NONE;

freeaddrinfo_exit:
    if (ctx->result) {
        freeaddrinfo(ctx->result);
        ctx->result = NULL;
    }

getaddrinfo_exit:
    if (ctx->client->sock != INVALID_SOCKET) {
        close(ctx->client->sock);
    }
    
    free(ctx->client);
    ctx->client = NULL;
    
    free(ctx->server);
    ctx->server = NULL;
    
    return CLIENT_ERROR_INIT;
}

/*!
 *
 */
static enum client_error_e closeClientSocket_f(struct client_context_s *ctx)
{
    ASSERT(ctx);
    
    // Close socket
    if (ctx->client->sock != INVALID_SOCKET) {
        close(ctx->client->sock);
    }
    
    // Release links
    free(ctx->client);
    ctx->client = NULL;
    
    free(ctx->server);
    ctx->server = NULL;
    
    // Release memory allocated to buffer's data
    if (ctx->bufferIn.data) {
        free(ctx->bufferIn.data);
        ctx->bufferIn.data = NULL;
    }
    
    return CLIENT_ERROR_NONE;
}

/*!
 *
 */
static enum client_error_e getClientContext_f(struct client_s *obj, char *clientName,
                                              struct client_context_s **ctxOut, uint8_t lock)
{
    ASSERT(obj && obj->pData && clientName && ctxOut);

    struct client_private_data_s *pData = (struct client_private_data_s*)(obj->pData);

    if (!pData->clientsList) {
        Loge("There is currently no element in list");
        return CLIENT_ERROR_LIST;
    }

    enum client_error_e ret = CLIENT_ERROR_NONE;

    if (lock && (pData->clientsList->lock(pData->clientsList) != LIST_ERROR_NONE)) {
        Loge("Failed to lock clientsList");
        return CLIENT_ERROR_LIST;
    }

    uint32_t nbElements;
    if (pData->clientsList->getNbElements(pData->clientsList, &nbElements) != LIST_ERROR_NONE) {
        Loge("Failed to get number of elements");
        ret = CLIENT_ERROR_LIST;
        goto exit;
    }

    while (nbElements > 0) {
        if (pData->clientsList->getElement(pData->clientsList,
                                           (void**)ctxOut) != LIST_ERROR_NONE) {
            Loge("Failed to retrieve element");
            ret = CLIENT_ERROR_LIST;
            goto exit;
        }

        if (!strcmp((*ctxOut)->params.name, clientName)) {
            break;
        }

        nbElements--;
    }

    if (nbElements == 0) {
        //Loge("Element %s not found", clientName);
        ret = CLIENT_ERROR_LIST;
        goto exit;
    }

exit:
    if (lock) {
        (void)pData->clientsList->unlock(pData->clientsList);
    }

    return ret;
}

/*!
 *
 */
static void watcherTaskFct_f(struct task_params_s *params)
{
    ASSERT(params && params->fctData && params->userData);
    
    struct client_context_s *ctx        = (struct client_context_s*)params->fctData;
    struct client_private_data_s *pData = (struct client_private_data_s*)params->userData;

    if (pData->linkHelper->isReadyForReading(pData->linkHelper, ctx->client, WAIT_TIME_2S) == NO) {
        return;
    }

    if (pthread_mutex_lock(&ctx->lock) != 0) {
        return;
    }
    
    if (!ctx->ackReceived) {
        uint8_t allocateBufferNeeded = 1;

        if (ctx->params.mode == LINK_MODE_HTTP) {
            ctx->bufferIn.data   = (void*)ctx->http200Ok.str;
            ctx->bufferIn.length = sizeof(ctx->http200Ok.str);

            if ((pData->linkHelper->readData(pData->linkHelper,
                                             ctx->client, NULL,
                                             &ctx->bufferIn, &ctx->nbRead) == ERROR)
                || (ctx->nbRead == 0)) {
                Loge("No ack received from server yet");
                ctx->bufferIn.data = NULL;
                goto exit;
            }

            pData->linkHelper->parseHttp200Ok(pData->linkHelper, &ctx->http200Ok);
            Logd("Http 200 OK : %s", ctx->http200Ok.str);
            
            if (!ctx->http200Ok.is200Ok) {
                Loge("200 OK not received");
                allocateBufferNeeded = 0;
            }
            
            ctx->bufferIn.data = NULL;
        }
        else if (ctx->params.mode == LINK_MODE_CUSTOM) {
            ctx->bufferIn.data   = (void*)ctx->customContent.str;
            ctx->bufferIn.length = sizeof(ctx->customContent.str);
                    
            if ((pData->linkHelper->readData(pData->linkHelper,
                                             ctx->client, ctx->server,
                                             &ctx->bufferIn, &ctx->nbRead) == ERROR)
                || (ctx->nbRead == 0)) {
                Loge("No ack received from server yet");
                ctx->bufferIn.data = NULL;
                goto exit;
            }

            pData->linkHelper->parseCustomContent(pData->linkHelper, &ctx->customContent);
            Logd("Custom Content : %s", ctx->customContent.str);
            
            if (ctx->customContent.maxBufferSize == 0) {
                Loge("Unexpected message received from server");
                allocateBufferNeeded = 0;
            }
            else if (ctx->params.maxBufferSize < ctx->customContent.maxBufferSize) {
                Logw("maxBufferSize changed by remote server from %lu bytes to %lu bytes",
                        ctx->params.maxBufferSize, ctx->customContent.maxBufferSize);
                ctx->params.maxBufferSize = ctx->customContent.maxBufferSize;
            }
            
            ctx->bufferIn.data = NULL;
        }
        
        if (!allocateBufferNeeded) {
            Logd("Client socket already closed on server side");
            close(ctx->client->sock);
            if (ctx->params.onLinkBrokenCb) {
                ctx->params.onLinkBrokenCb(&ctx->params, ctx->params.userData);
            }
            goto exit;
        }
        
        ASSERT((ctx->bufferIn.data = calloc(1, ctx->params.maxBufferSize)));
        ctx->bufferIn.length = ctx->params.maxBufferSize;
        
        ctx->ackReceived = 1;
        Logd("ackReceived = %d / maxBufferSize = %lu",
                ctx->ackReceived, ctx->params.maxBufferSize);
    }
    else {
        if (ctx->params.mode == LINK_MODE_HTTP) {
            ctx->httpBuffer.data   = (void*)ctx->httpContent.str;
            ctx->httpBuffer.length = sizeof(ctx->httpContent.str);
            
            if ((pData->linkHelper->readData(pData->linkHelper,
                                             ctx->client, NULL,
                                             &ctx->httpBuffer, &ctx->nbRead) == ERROR)
                || (ctx->nbRead == 0)) {
                Loge("Failed to receive http content's header");
                close(ctx->client->sock);
                if (ctx->params.onLinkBrokenCb) {
                    ctx->params.onLinkBrokenCb(&ctx->params, ctx->params.userData);
                }
                goto exitNoFree;
            }
            
            pData->linkHelper->parseHttpContent(pData->linkHelper, &ctx->httpContent);
            Logd("Http Content length : %ld / read = %ld / bodyStart = %d",
                    ctx->httpContent.length, ctx->nbRead, ctx->httpContent.bodyStart);
            
            if (ctx->httpContent.length == 0) {
                Logw("Bad HTTP response");
                goto exitNoFree;
            }
            
            ctx->nbBodyRead        = ctx->nbRead - ctx->httpContent.bodyStart;
            ctx->httpBuffer.length = ctx->httpContent.length - ctx->nbBodyRead;
            
            ASSERT((ctx->httpBuffer.data = calloc(1, ctx->httpBuffer.length)));
            if (pData->linkHelper->readData(pData->linkHelper,
                                            ctx->client, NULL,
                                            &ctx->httpBuffer, &ctx->nbRead) == ERROR) {
                Loge("Failed to read from server");
                goto exit;
            }

            if ((ctx->nbBodyRead + ctx->nbRead) != ctx->httpContent.length) {
                Logw("Incomplete frame => ignored");
                goto exit;
            }

            // Adjust buffer size if necessary
            if (ctx->httpContent.length > ctx->params.maxBufferSize) {
                Logw("Ajusting maxBufferSize from %lu bytes to %lu bytes",
                        ctx->params.maxBufferSize, ctx->httpContent.length);
                free(ctx->bufferIn.data);

                ctx->bufferIn.length       = ctx->httpContent.length;
                ctx->params.maxBufferSize  = ctx->httpContent.length;
                ASSERT((ctx->bufferIn.data = calloc(1, ctx->bufferIn.length)));
            }

            // Copy received data
            memcpy(ctx->bufferIn.data, ctx->httpContent.str + ctx->httpContent.bodyStart, ctx->nbBodyRead);
            memcpy(ctx->bufferIn.data + ctx->nbBodyRead, ctx->httpBuffer.data, ctx->nbRead);
            ctx->bufferIn.length = ctx->nbBodyRead + ctx->nbRead;
        }
        else if (pData->linkHelper->readData(pData->linkHelper,
                                             ctx->client, ctx->server,
                                             &ctx->bufferIn, &ctx->nbRead) == ERROR) {
            Loge("Failed to read from server");
            goto exit;
        }

        if ((ctx->nbRead == 0) && ctx->params.onLinkBrokenCb) {
            ctx->params.onLinkBrokenCb(&ctx->params, ctx->params.userData);
            goto exit;
        }
        
        sem_post(&ctx->sem);
    }

exit:
    if (ctx->httpBuffer.data) {
        free(ctx->httpBuffer.data);
        ctx->httpBuffer.data = NULL;
    }

exitNoFree:
    (void)pthread_mutex_unlock(&ctx->lock);
}

/*!
 *
 */
static void receiverTaskFct_f(struct task_params_s *params)
{
    ASSERT(params && params->fctData && params->userData);
    
    struct client_context_s *ctx = (struct client_context_s*)params->fctData;
    
    if (ctx->quit) {
        return;
    }
    
    sem_wait(&ctx->sem);
    
    if (ctx->quit) {
        return;
    }
    
    if (pthread_mutex_lock(&ctx->lock) != 0) {
        return;
    }
    
    ctx->bufferOut.length = ctx->bufferIn.length;
    ASSERT((ctx->bufferOut.data = calloc(1, ctx->bufferIn.length)));
    memcpy(ctx->bufferOut.data, ctx->bufferIn.data, ctx->bufferIn.length);
    
    (void)pthread_mutex_unlock(&ctx->lock);
    ctx->params.onDataReceivedCb(&ctx->params, &ctx->bufferOut, ctx->params.userData);
    
    (void)pthread_mutex_lock(&ctx->lock);
    free(ctx->bufferOut.data);
    ctx->bufferOut.data = NULL;
    (void)pthread_mutex_unlock(&ctx->lock);
}

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static uint8_t compareCb(struct list_s *obj, void *elementToCheck, void *userData)
{
    ASSERT(obj && elementToCheck && userData);
    
    struct client_context_s *ctx = (struct client_context_s*)elementToCheck;
    char *nameOfElementToRemove  = (char*)userData;
    
    return (strncmp(nameOfElementToRemove, ctx->params.name, strlen(ctx->params.name)) == 0);
}

/*!
 *
 */
static void releaseCb(struct list_s *obj, void *element)
{
    ASSERT(obj && element);
    
    struct client_context_s *ctx = (struct client_context_s*)element;
       
    /* Stop tasks */
    ctx->quit = 1;
    sem_post(&ctx->sem);
    
    (void)ctx->clientTask->stop(ctx->clientTask, &ctx->receiverTaskParams);
    (void)ctx->clientTask->stop(ctx->clientTask, &ctx->watcherTaskParams);
    
    (void)ctx->clientTask->destroy(ctx->clientTask, &ctx->receiverTaskParams);
    (void)ctx->clientTask->destroy(ctx->clientTask, &ctx->watcherTaskParams);
    
    /* UnInit tasks */
    (void)Task_UnInit(&ctx->clientTask);
    
    /* Destroy sem and mutex */
    (void)pthread_mutex_destroy(&ctx->lock);
    (void)sem_destroy(&ctx->sem);
    
    /* Close socket */
    (void)closeClientSocket_f(ctx);
    
    /* Release resources */
    free(ctx);
}
