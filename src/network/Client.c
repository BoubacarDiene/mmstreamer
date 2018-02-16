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
* \file   Client.c
* \brief  Clients management
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "network/Client.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "Client"

#define WATCHER_TASK_NAME  "WatcherTask"
#define RECEIVER_TASK_NAME "ReceiverTask"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef struct CLIENT_CONTEXT_S {
    volatile uint8_t quit;
    
    CLIENT_PARAMS_S  params;
    
    LINK_S           *client;
    LINK_S           *server;
    
    CUSTOM_HEADER_S  customHeader;
    CUSTOM_CONTENT_S customContent;

    HTTP_GET_S       httpGet;
    HTTP_200_OK_S    http200Ok;
    HTTP_CONTENT_S   httpContent;
    BUFFER_S         httpBuffer;
    size_t           nbBodyRead;
    
    BUFFER_S         bufferIn;
    BUFFER_S         bufferOut;
    ssize_t          nbRead;
    
    sem_t            sem;
    pthread_mutex_t  lock;
    
    TASK_S           *clientTask;
    TASK_PARAMS_S    watcherTaskParams;
    TASK_PARAMS_S    receiverTaskParams;
    
    ADDRINFO         hints;
    ADDRINFO         *result;
    ADDRINFO         *rp;
    void             *addr;
    char             *ipver;
    char             ipstr[MAX_ADDRESS_SIZE];
    uint16_t         port;
    union {
        SOCKADDR_IN  *v4;
        SOCKADDR_IN6 *v6;
    } ip;
    
    uint8_t          ackReceived;
} CLIENT_CONTEXT_S;

typedef struct CLIENT_PRIVATE_DATA_S {
    LINK_HELPER_S *linkHelper;
    LIST_S        *clientsList;
} CLIENT_PRIVATE_DATA_S;

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

static CLIENT_ERROR_E start_f(CLIENT_S *obj, CLIENT_PARAMS_S *params);
static CLIENT_ERROR_E stop_f (CLIENT_S *obj, CLIENT_PARAMS_S *params);

static CLIENT_ERROR_E sendData_f(CLIENT_S *obj, CLIENT_PARAMS_S *params, BUFFER_S *buffer);

static CLIENT_ERROR_E openClientSocket_f(CLIENT_CONTEXT_S *ctx, LINK_HELPER_S *linkHelper);
static CLIENT_ERROR_E closeClientSocket_f(CLIENT_CONTEXT_S *ctx);
static CLIENT_ERROR_E getClientContext_f(CLIENT_S *obj, char *clientName, CLIENT_CONTEXT_S **ctxOut, uint8_t lock);

static void watcherTaskFct_f (TASK_PARAMS_S *params);
static void receiverTaskFct_f(TASK_PARAMS_S *params);

static uint8_t compareCb(LIST_S *obj, void *elementToCheck, void *userData);
static void    releaseCb(LIST_S *obj, void *element);

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
CLIENT_ERROR_E Client_Init(CLIENT_S **obj)
{
    assert(obj && (*obj = calloc(1, sizeof(CLIENT_S))));

    CLIENT_PRIVATE_DATA_S *pData;
    assert((pData = calloc(1, sizeof(CLIENT_PRIVATE_DATA_S))));
    
    LinkHelper_Init(&pData->linkHelper);
    assert(pData->linkHelper);
    
    LIST_PARAMS_S listParams;
    memset(&listParams, '\0', sizeof(LIST_PARAMS_S));
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
    pData = NULL;
    
    free(*obj);
    *obj = NULL;
    
    return CLIENT_ERROR_INIT;
}

/*!
 *
 */
CLIENT_ERROR_E Client_UnInit(CLIENT_S **obj)
{
    assert(obj && *obj && (*obj)->pData);
    
    CLIENT_PRIVATE_DATA_S *pData = (CLIENT_PRIVATE_DATA_S*)((*obj)->pData);
    
    LinkHelper_UnInit(&pData->linkHelper);
    
    (void)List_UnInit(&pData->clientsList);
    
    free(pData);
    pData = NULL;
    
    free(*obj);
    *obj = NULL;
    
    return CLIENT_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static CLIENT_ERROR_E start_f(CLIENT_S *obj, CLIENT_PARAMS_S *params)
{
    assert(obj && obj->pData && params);

    CLIENT_CONTEXT_S *ctx = NULL;

    /* Check if client is already started or not */
    if (getClientContext_f(obj, params->name, &ctx, 1) == CLIENT_ERROR_NONE) {
        Loge("%s's context exists", params->name);
        return CLIENT_ERROR_START;
    }

    /* Init context */
    assert((ctx = calloc(1, sizeof(CLIENT_CONTEXT_S))));
    
    strncpy(ctx->params.name, params->name, sizeof(ctx->params.name));
    ctx->params.type             = params->type;
    ctx->params.link             = params->link;
    ctx->params.mode             = params->mode;
    memcpy(&ctx->params.recipient, &params->recipient, sizeof(params->recipient));
    ctx->params.priority         = params->priority;
    ctx->params.maxBufferSize    = params->maxBufferSize;
    ctx->params.onDataReceivedCb = params->onDataReceivedCb;
    ctx->params.onLinkBrokenCb   = params->onLinkBrokenCb;
    ctx->params.userData         = params->userData;
    
    /* Init socket */
    CLIENT_PRIVATE_DATA_S *pData = (CLIENT_PRIVATE_DATA_S*)(obj->pData);

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
    if (!pData->clientsList || (pData->clientsList->lock(pData->clientsList) != LIST_ERROR_NONE)) {
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
    ctx = NULL;

    return CLIENT_ERROR_START;
}

/*!
 *
 */
static CLIENT_ERROR_E stop_f(CLIENT_S *obj, CLIENT_PARAMS_S *params)
{
    assert(obj && obj->pData && params);

    CLIENT_CONTEXT_S *ctx = NULL;
    CLIENT_ERROR_E ret    = CLIENT_ERROR_NONE;

    /* Ensure that the client is started */
    if ((ret = getClientContext_f(obj, params->name, &ctx, 1)) != CLIENT_ERROR_NONE) {
        Loge("%s's context does not exist", params->name);
        goto exit;
    }

    CLIENT_PRIVATE_DATA_S *pData = (CLIENT_PRIVATE_DATA_S*)(obj->pData);

    if (!pData->clientsList || pData->clientsList->lock(pData->clientsList) != LIST_ERROR_NONE) {
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
static CLIENT_ERROR_E sendData_f(CLIENT_S *obj, CLIENT_PARAMS_S *params, BUFFER_S *buffer)
{
    assert(obj && obj->pData && params && buffer);

    CLIENT_PRIVATE_DATA_S *pData = (CLIENT_PRIVATE_DATA_S*)(obj->pData);
    
    if (!pData->clientsList) {
        Loge("No element in list");
        return CLIENT_ERROR_LIST;
    }
    
    if (pData->clientsList->lock(pData->clientsList) != LIST_ERROR_NONE) {
        Loge("Failed to lock clientsList");
        return CLIENT_ERROR_LIST;
    }
    
    // Retrieve client context
    CLIENT_CONTEXT_S *ctx = NULL;
    CLIENT_ERROR_E ret    = CLIENT_ERROR_NONE;

    if ((ret = getClientContext_f(obj, params->name, &ctx, 0)) != CLIENT_ERROR_NONE) {
        Loge("%s's context exists", params->name);
        goto exit;
    }
    
    // Send data to server
    if (pData->linkHelper->writeData(pData->linkHelper, ctx->client, ctx->server, buffer, NULL) == ERROR) {
        Loge("Failed to send data to server");
    }

exit:
    (void)pData->clientsList->unlock(pData->clientsList);
        
    return ret;
}

/*!
 *
 */
static CLIENT_ERROR_E openClientSocket_f(CLIENT_CONTEXT_S *ctx, LINK_HELPER_S *linkHelper)
{
    assert(ctx && linkHelper);
    
    if ((ctx->params.mode == LINK_MODE_HTTP) && (ctx->params.link != LINK_TYPE_INET_STREAM)) {
        Loge("Bad config : mode = HTTP but link != INET_STREAM");
        return CLIENT_ERROR_PARAMS;
    }
    
    assert((ctx->client = calloc(1, sizeof(LINK_S)))
            && (ctx->server = calloc(1, sizeof(LINK_S))));
    
    ctx->client->domain = ((ctx->params.link == LINK_TYPE_INET_STREAM)
                            || (ctx->params.link == LINK_TYPE_INET_DGRAM))  ? AF_UNSPEC   : AF_UNIX;
    ctx->client->type   = ((ctx->params.link == LINK_TYPE_INET_STREAM)
                            || (ctx->params.link == LINK_TYPE_UNIX_STREAM)) ? SOCK_STREAM : SOCK_DGRAM;
    
    if (ctx->client->domain != AF_UNIX) {
        memset(&ctx->hints, '\0', sizeof(ADDRINFO));
        ctx->hints.ai_family   = ctx->client->domain;
        ctx->hints.ai_socktype = ctx->server->type;

        int status = getaddrinfo(ctx->params.recipient.server.host,
                                    ctx->params.recipient.server.service, &ctx->hints, &ctx->result);
        if (status != 0) {
            Loge("getaddrinfo() failed - %s", gai_strerror(status));
            goto exit;
        }
    }

next_addr:
    if (ctx->client->domain != AF_UNIX) {
        ctx->rp = ctx->rp ? ctx->rp->ai_next : ctx->result;
        
        if (!ctx->rp) {
            Loge("No address succeeded");
            goto exit;
        }
        
        ctx->client->domain = ctx->rp->ai_family;
        ctx->client->type   = ctx->rp->ai_socktype;
        
        if (ctx->client->domain != AF_UNIX) {
            ctx->ip.v4  = (SOCKADDR_IN*)ctx->rp->ai_addr;
            ctx->addr   = &(ctx->ip.v4->sin_addr);
            ctx->port   = (uint16_t)ntohs(ctx->ip.v4->sin_port);
            ctx->ipver  = IPV4;
        }
        else {
            ctx->ip.v6  = (SOCKADDR_IN6*)ctx->rp->ai_addr;
            ctx->addr   = &(ctx->ip.v6->sin6_addr);
            ctx->port   = (uint16_t)ntohs(ctx->ip.v6->sin6_port);
            ctx->ipver  = IPV6;
        }
        
        inet_ntop(ctx->client->domain, ctx->addr, ctx->ipstr, sizeof(ctx->ipstr));
        Logd("Testing : %s - address : %s / port : %d", ctx->ipver, ctx->ipstr, ctx->port);
    }
    
    if ((ctx->client->sock = socket(ctx->client->domain, ctx->client->type, 0)) == INVALID_SOCKET) {
        if (ctx->client->domain != AF_UNIX) {
            goto next_addr;
        }
        Loge("Failed to create client socket");
        goto exit;
    }
    
    /* Init server's data */
    if (ctx->client->domain != AF_UNIX) {
        if (!strcmp(ctx->ipver, IPV4)) {
            ctx->server->destAddress       = (SOCKADDR*)(ctx->ip.v4);
            ctx->server->destAddressLength = sizeof(*(ctx->ip.v4));
        }
        else {
            ctx->server->destAddress       = (SOCKADDR*)(ctx->ip.v6);
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
        ctx->server->destAddress       = (SOCKADDR*)&ctx->server->addr.sun;
    }
    
    /* Connect to server if required */
    if (ctx->client->type == SOCK_STREAM) { // Connection oriented
        if ((connect(ctx->client->sock, ctx->server->destAddress, ctx->server->destAddressLength) == SOCKET_ERROR)
            && (errno != EINPROGRESS)) {
            if (ctx->client->domain != AF_UNIX) {
                close(ctx->client->sock);
                goto next_addr;
            }
            Loge("Failed to connect to server");
            goto exit;
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
            if (bind(ctx->client->sock, (SOCKADDR*)&(ctx->client->addr.sun), sizeof(ctx->client->addr.sun)) == SOCKET_ERROR) {
                Loge("Failed to bind to client socket");
                goto exit;
            }
        }
    }
    
    /* Exchange first data with server to finalize initialization */
    BUFFER_S buffer;
    if (ctx->params.mode == LINK_MODE_HTTP) {
        strcpy(ctx->httpGet.path, ctx->params.recipient.server.path);
        strcpy(ctx->httpGet.host, ctx->ipstr);
        ctx->httpGet.port = ctx->port;
        
        linkHelper->prepareHttpGet(linkHelper, &ctx->httpGet);
        Logd("Http Get : %s", ctx->httpGet.str);
        
        if (linkHelper->isReadyForWriting(linkHelper, ctx->client, WAIT_TIME_10MS) == NO) {
            Loge("Server not ready for writing");
            goto exit;
        }
        
        buffer.data   = (void*)ctx->httpGet.str;
        buffer.length = strlen(ctx->httpGet.str);
            
        if (linkHelper->writeData(linkHelper, ctx->client, NULL, &buffer, NULL) == ERROR) {
            Loge("Failed to send data to server");
            goto exit;
        }
    }
    else if ((ctx->client->type == SOCK_DGRAM) || (ctx->params.mode == LINK_MODE_CUSTOM)) {
        if (linkHelper->isReadyForWriting(linkHelper, ctx->client, WAIT_TIME_10MS) == NO) {
            Loge("Server not ready for writing");
            goto exit;
        }
            
        linkHelper->prepareCustomHeader(linkHelper, &ctx->customHeader); 
        Logd("Custom header : %s", ctx->customHeader.str);
        
        buffer.data   = (void*)ctx->customHeader.str;
        buffer.length = strlen(ctx->customHeader.str);
            
        if (linkHelper->writeData(linkHelper, ctx->client, ctx->server, &buffer, NULL) == ERROR) {
            Loge("Failed to send data to server");
            goto exit;
        }
    }
    
    if (ctx->client->domain != AF_UNIX) {
        inet_ntop(ctx->client->domain, ctx->addr, ctx->ipstr, sizeof(ctx->ipstr));
        Logd("Using : %s - address : %s / port : %u", ctx->ipver, ctx->ipstr, ctx->port);
        
        freeaddrinfo(ctx->result);
        ctx->result = NULL;
    }
    
    return CLIENT_ERROR_NONE;
    
exit:
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
static CLIENT_ERROR_E closeClientSocket_f(CLIENT_CONTEXT_S *ctx)
{
    assert(ctx);
    
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
static CLIENT_ERROR_E getClientContext_f(CLIENT_S *obj, char *clientName, CLIENT_CONTEXT_S **ctxOut, uint8_t lock)
{
    assert(obj && obj->pData && clientName && ctxOut);

    CLIENT_PRIVATE_DATA_S *pData = (CLIENT_PRIVATE_DATA_S*)(obj->pData);

    if (!pData->clientsList) {
        Loge("There is currently no element in list");
        return CLIENT_ERROR_LIST;
    }

    CLIENT_ERROR_E ret = CLIENT_ERROR_NONE;

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
        if (pData->clientsList->getElement(pData->clientsList, (void**)ctxOut) != LIST_ERROR_NONE) {
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
static void watcherTaskFct_f(TASK_PARAMS_S *params)
{
    assert(params && params->fctData && params->userData);
    
    CLIENT_CONTEXT_S      *ctx   = (CLIENT_CONTEXT_S*)params->fctData;
    CLIENT_PRIVATE_DATA_S *pData = (CLIENT_PRIVATE_DATA_S*)params->userData;

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

            if ((pData->linkHelper->readData(pData->linkHelper, ctx->client, NULL, &ctx->bufferIn, &ctx->nbRead) == ERROR)
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
                    
            if ((pData->linkHelper->readData(pData->linkHelper, ctx->client, ctx->server, &ctx->bufferIn, &ctx->nbRead) == ERROR)
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
            else {
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
        
        assert((ctx->bufferIn.data = calloc(1, ctx->params.maxBufferSize)));
        ctx->bufferIn.length = ctx->params.maxBufferSize;
        
        ctx->ackReceived = 1;
        Logd("ackReceived = %d / maxBufferSize = %lu", ctx->ackReceived, ctx->params.maxBufferSize);
    }
    else {
        if (ctx->params.mode == LINK_MODE_HTTP) {
            ctx->httpBuffer.data   = (void*)ctx->httpContent.str;
            ctx->httpBuffer.length = sizeof(ctx->httpContent.str);
            
            if ((pData->linkHelper->readData(pData->linkHelper, ctx->client, NULL, &ctx->httpBuffer, &ctx->nbRead) == ERROR)
                || (ctx->nbRead == 0)) {
                Loge("Failed to receive http content's header");
                goto exitNoFree;
            }
            
            pData->linkHelper->parseHttpContent(pData->linkHelper, &ctx->httpContent);
            Logd("Http Content : %s / read = %ld / bodyStart = %d", ctx->httpContent.str, ctx->nbRead, ctx->httpContent.bodyStart);
            
            if (ctx->httpContent.length == 0) {
                Logw("Bad HTTP response");
                goto exitNoFree;
            }
            
            ctx->nbBodyRead        = ctx->nbRead - ctx->httpContent.bodyStart;
            ctx->httpBuffer.length = ctx->httpContent.length - ctx->nbBodyRead;
            
            assert((ctx->httpBuffer.data = calloc(1, ctx->httpBuffer.length)));
            if (pData->linkHelper->readData(pData->linkHelper, ctx->client, NULL, &ctx->httpBuffer, &ctx->nbRead) == ERROR) {
                Loge("Failed to read from server");
                goto exit;
            }
            
            // Adjust buffer size if necessary
            if (ctx->httpContent.length > ctx->params.maxBufferSize) {
                Logw("Ajusting maxBufferSize from %lu bytes to %lu bytes", ctx->params.maxBufferSize, ctx->httpContent.length);
                free(ctx->bufferIn.data);

                ctx->bufferIn.length       = ctx->httpContent.length;
                ctx->params.maxBufferSize  = ctx->httpContent.length;
                assert((ctx->bufferIn.data = calloc(1, ctx->bufferIn.length)));
            }

            // Copy received data
            memcpy(ctx->bufferIn.data, ctx->httpContent.str + ctx->httpContent.bodyStart, ctx->nbBodyRead);
            memcpy(ctx->bufferIn.data + ctx->nbBodyRead, ctx->httpBuffer.data, ctx->nbRead);
            ctx->bufferIn.length = ctx->nbBodyRead + ctx->nbRead;
            Logd("Http body length : read = %ld / expected : %ld", ctx->bufferIn.length, ctx->httpBuffer.length);
        }
        else if (pData->linkHelper->readData(pData->linkHelper, ctx->client, ctx->server, &ctx->bufferIn, &ctx->nbRead) == ERROR) {
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
static void receiverTaskFct_f(TASK_PARAMS_S *params)
{
    assert(params && params->fctData && params->userData);
    
    CLIENT_CONTEXT_S *ctx = (CLIENT_CONTEXT_S*)params->fctData;
    
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
    assert((ctx->bufferOut.data = calloc(1, ctx->bufferIn.length)));
    memcpy(ctx->bufferOut.data, ctx->bufferIn.data, ctx->bufferIn.length);
    
    (void)pthread_mutex_unlock(&ctx->lock);
    ctx->params.onDataReceivedCb(&ctx->params, &ctx->bufferOut, ctx->params.userData);
    
    (void)pthread_mutex_lock(&ctx->lock);
    free(ctx->bufferOut.data);
    ctx->bufferOut.data = NULL;
    (void)pthread_mutex_unlock(&ctx->lock);
}

/*!
 *
 */
static uint8_t compareCb(LIST_S *obj, void *elementToCheck, void *userData)
{
    assert(obj && elementToCheck && userData);
    
    CLIENT_CONTEXT_S *ctx  = (CLIENT_CONTEXT_S*)elementToCheck;
    char *nameOfElementToRemove = (char*)userData;
    
    return (strncmp(nameOfElementToRemove, ctx->params.name, strlen(ctx->params.name)) == 0);
}

/*!
 *
 */
static void releaseCb(LIST_S *obj, void *element)
{
    assert(obj && element);
    
    CLIENT_CONTEXT_S *ctx = (CLIENT_CONTEXT_S*)element;
       
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
    ctx->params.userData = NULL;
    
    free(ctx);
    ctx = NULL;
}
