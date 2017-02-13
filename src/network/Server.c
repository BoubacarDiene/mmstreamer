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
* \file   Server.c
* \brief  Servers management
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include <sys/time.h>

#include "network/Server.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "Server"

#define WATCHER_TASK_NAME "WatcherTask"
#define SENDER_TASK_NAME  "SenderTask"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef struct CLIENT_LINK_PDATA_S {
    uint8_t        isAuthorizedReceiver;
} CLIENT_LINK_PDATA_S;

typedef struct SERVER_CONTEXT_S {
    volatile uint8_t       quit;
    
    volatile uint8_t       senderSuspended;

    SERVER_PARAMS_S        params;
    
    LINK_S                 *server;
    
    CUSTOM_HEADER_S        customHeader;
    CUSTOM_CONTENT_S       customContent;
    HTTP_GET_S             httpGet;
    HTTP_200_OK_S          http200Ok;
    HTTP_400_BAD_REQUEST_S http400BadRequest;
    HTTP_404_NOT_FOUND_S   http404NotFound;
    HTTP_CONTENT_S         httpContent;
    
    LIST_S                 *clientsList;
    
    sem_t                  sem;
    
    pthread_mutex_t        lock;
    BUFFER_S               bufferIn;
    BUFFER_S               bufferOut;
    
    BUFFER_S               watcherTempBuffer;
    BUFFER_S               senderTempBuffer;
    
    TASK_S                 *serverTask;
    TASK_PARAMS_S          watcherTaskParams;
    TASK_PARAMS_S          senderTaskParams;
    
    ADDRINFO               hints;
    ADDRINFO               *result;
    ADDRINFO               *rp;
    void                   *addr;
    char                   *ipver;
    char                   ipstr[MAX_ADDRESS_SIZE];
    uint16_t               port;
    union {
        SOCKADDR_IN        *v4;
        SOCKADDR_IN6       *v6;
    } ip;
} SERVER_CONTEXT_S;

typedef struct SERVER_PRIVATE_DATA_S {
    LINK_HELPER_S *linkHelper;
    LIST_S        *serversList;
} SERVER_PRIVATE_DATA_S;

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

static SERVER_ERROR_E start_f(SERVER_S *obj, SERVER_PARAMS_S *params);
static SERVER_ERROR_E stop_f (SERVER_S *obj, SERVER_PARAMS_S *params);

static SERVER_ERROR_E addReceiver_f   (SERVER_S *obj, SERVER_PARAMS_S *params, LINK_S *client);
static SERVER_ERROR_E removeReceiver_f(SERVER_S *obj, SERVER_PARAMS_S *params, LINK_S *client);

static SERVER_ERROR_E suspendSender_f(SERVER_S *obj, SERVER_PARAMS_S *params);
static SERVER_ERROR_E resumeSender_f (SERVER_S *obj, SERVER_PARAMS_S *params);

static SERVER_ERROR_E disconnectClient_f(SERVER_S *obj, SERVER_PARAMS_S *params, LINK_S *client);

static SERVER_ERROR_E sendData_f(SERVER_S *obj, SERVER_PARAMS_S *params, BUFFER_S *buffer);

static SERVER_ERROR_E openServerSocket_f(SERVER_CONTEXT_S *ctx, LINK_HELPER_S *linkHelper);
static SERVER_ERROR_E closeServerSocket_f(SERVER_CONTEXT_S *ctx);
static SERVER_ERROR_E getServerContext_f(SERVER_S *obj, char *serverName, SERVER_CONTEXT_S **ctxOut);

static void watcherTaskFct_f(TASK_PARAMS_S *params);
static void senderTaskFct_f (TASK_PARAMS_S *params);

static uint8_t compareServerCb(LIST_S *obj, void *elementToCheck, void *userData);
static void    releaseServerCb(LIST_S *obj, void *element);

static uint8_t compareClientCb(LIST_S *obj, void *elementToCheck, void *userData);
static void    releaseClientCb(LIST_S *obj, void *element);

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
SERVER_ERROR_E Server_Init(SERVER_S **obj)
{
    assert(obj && (*obj = calloc(1, sizeof(SERVER_S))));

    SERVER_PRIVATE_DATA_S *pData;
    assert((pData = calloc(1, sizeof(SERVER_PRIVATE_DATA_S))));
    
    LinkHelper_Init(&pData->linkHelper);
    assert(pData->linkHelper);
    
    LIST_PARAMS_S listParams;
    memset(&listParams, '\0', sizeof(LIST_PARAMS_S));
    listParams.compareCb = compareServerCb;
    listParams.releaseCb = releaseServerCb;
    listParams.browseCb  = NULL;
    
    if (List_Init(&pData->serversList, &listParams) != LIST_ERROR_NONE) {
        Loge("List_Init() failed");
        goto exit;
    }
    
    (*obj)->start            = start_f;
    (*obj)->stop             = stop_f;
    
    (*obj)->addReceiver      = addReceiver_f;
    (*obj)->removeReceiver   = removeReceiver_f;
    
    (*obj)->suspendSender    = suspendSender_f;
    (*obj)->resumeSender     = resumeSender_f;

    (*obj)->disconnectClient = disconnectClient_f;
    
    (*obj)->sendData         = sendData_f;
    
    (*obj)->pData            = (void*)pData;
    
    pData->linkHelper->keepMeAlive(pData->linkHelper);
    
    return SERVER_ERROR_NONE;
    
exit:
    LinkHelper_UnInit(&pData->linkHelper);
    
    free(pData);
    pData = NULL;
    
    free(*obj);
    *obj = NULL;
    
    return SERVER_ERROR_INIT;
}

/*!
 *
 */
SERVER_ERROR_E Server_UnInit(SERVER_S **obj)
{
    assert(obj && *obj && (*obj)->pData);
    
    SERVER_PRIVATE_DATA_S *pData = (SERVER_PRIVATE_DATA_S*)((*obj)->pData);
    
    LinkHelper_UnInit(&pData->linkHelper);
    
    (void)List_UnInit(&pData->serversList);
    
    free(pData);
    pData = NULL;
    
    free(*obj);
    *obj = NULL;
    
    return SERVER_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static SERVER_ERROR_E start_f(SERVER_S *obj, SERVER_PARAMS_S *params)
{
    assert(obj && obj->pData && params);

    SERVER_CONTEXT_S *ctx = NULL;

    /* Check if server is already running or not */
    if (getServerContext_f(obj, params->name, &ctx) == SERVER_ERROR_NONE) {
        Loge("%s's context exists", params->name);
        return SERVER_ERROR_START;
    }

    /* Init context */
    assert((ctx = calloc(1, sizeof(SERVER_CONTEXT_S))));
    
    strncpy(ctx->params.name, params->name, sizeof(ctx->params.name));
    ctx->params.type                   = params->type;
    ctx->params.link                   = params->link;
    ctx->params.mode                   = params->mode;
    ctx->params.acceptMode             = params->acceptMode;
    strncpy(ctx->params.mime, params->mime, sizeof(params->mime));
    memcpy(&ctx->params.recipient, &params->recipient, sizeof(params->recipient));
    ctx->params.priority               = params->priority;
    ctx->params.maxClients             = params->maxClients;
    ctx->params.maxBufferSize          = params->maxBufferSize;
    ctx->params.onClientStateChangedCb = params->onClientStateChangedCb;
    ctx->params.userData               = params->userData;
    
    /* Init socket */
    SERVER_PRIVATE_DATA_S *pData = (SERVER_PRIVATE_DATA_S*)(obj->pData);

    if (openServerSocket_f(ctx, pData->linkHelper) != SERVER_ERROR_NONE) {
        Loge("openServerSocket_f() failed");
        goto exit;
    }
    
    /* Init clients list, sem and mutex */
    LIST_PARAMS_S listParams;
    memset(&listParams, '\0', sizeof(LIST_PARAMS_S));
    listParams.compareCb = compareClientCb;
    listParams.releaseCb = releaseClientCb;
    listParams.browseCb  = NULL;
    
    if (List_Init(&ctx->clientsList, &listParams) != LIST_ERROR_NONE) {
        Loge("List_Init() failed");
        goto exit;
    }
    
    if (sem_init(&ctx->sem, 0, 0) != 0) {
        Loge("sem_init() failed");
        goto sem_exit;
    }
    
    if (pthread_mutex_init(&ctx->lock, NULL) != 0) {
        Loge("pthread_mutex_init() failed");
        goto mutex_exit;
    }
    
    /* Init and start tasks */
    if (Task_Init(&ctx->serverTask) != TASK_ERROR_NONE) {
        Loge("Task_Init() failed");
        goto task_init_exit;
    }
    
    // Watcher
    snprintf(ctx->watcherTaskParams.name, sizeof(ctx->watcherTaskParams.name), "%s-%u.%u.%u",
                WATCHER_TASK_NAME, params->type, params->link, params->acceptMode);
    ctx->watcherTaskParams.priority = params->priority;
    ctx->watcherTaskParams.fct      = watcherTaskFct_f;
    ctx->watcherTaskParams.fctData  = ctx;
    ctx->watcherTaskParams.userData = pData;
    ctx->watcherTaskParams.atExit   = NULL;
    
    if (ctx->serverTask->create(ctx->serverTask, &ctx->watcherTaskParams) != TASK_ERROR_NONE) {
        Loge("Failed to create watcherTask");
        goto watcher_create_exit;
    }
    
    // Sender
    snprintf(ctx->senderTaskParams.name, sizeof(ctx->senderTaskParams.name), "%s-%u.%u.%u",
                SENDER_TASK_NAME, params->type, params->link, params->acceptMode);
    ctx->senderTaskParams.priority = params->priority;
    ctx->senderTaskParams.fct      = senderTaskFct_f;
    ctx->senderTaskParams.fctData  = ctx;
    ctx->senderTaskParams.userData = pData;
    ctx->senderTaskParams.atExit   = NULL;
    
    if (ctx->serverTask->create(ctx->serverTask, &ctx->senderTaskParams) != TASK_ERROR_NONE) {
        Loge("Failed to create senderTask");
        goto sender_create_exit;
    }
    
    /* Add server's ctx to list */
    if (!pData->serversList || (pData->serversList->lock(pData->serversList) != LIST_ERROR_NONE)) {
        Loge("Failed to lock serversList");
        goto list_exit;
    }
    pData->serversList->add(pData->serversList, (void*)ctx);
    (void)pData->serversList->unlock(pData->serversList);
    
    // Start
    (void)ctx->serverTask->start(ctx->serverTask, &ctx->watcherTaskParams);
    (void)ctx->serverTask->start(ctx->serverTask, &ctx->senderTaskParams);
    
    return SERVER_ERROR_NONE;

list_exit:
    (void)ctx->serverTask->destroy(ctx->serverTask, &ctx->senderTaskParams);

sender_create_exit:
    (void)ctx->serverTask->destroy(ctx->serverTask, &ctx->watcherTaskParams);

watcher_create_exit:
    (void)Task_UnInit(&ctx->serverTask);

task_init_exit:
    (void)pthread_mutex_destroy(&ctx->lock);

mutex_exit:
    (void)sem_destroy(&ctx->sem);

sem_exit:
    (void)List_UnInit(&ctx->clientsList);

exit:
    return SERVER_ERROR_START;
}

/*!
 *
 */
static SERVER_ERROR_E stop_f(SERVER_S *obj, SERVER_PARAMS_S *params)
{
    assert(obj && obj->pData && params);

    SERVER_CONTEXT_S *ctx = NULL;
    SERVER_ERROR_E ret    = SERVER_ERROR_NONE;

    /* Ensure that the server is running */
    if ((ret = getServerContext_f(obj, params->name, &ctx)) != SERVER_ERROR_NONE) {
        Loge("%s's context does not exist", params->name);
        goto exit;
    }

    SERVER_PRIVATE_DATA_S *pData = (SERVER_PRIVATE_DATA_S*)(obj->pData);

    if (!pData->serversList || pData->serversList->lock(pData->serversList) != LIST_ERROR_NONE) {
        Loge("Failed to lock serversList");
        ret = SERVER_ERROR_LOCK;
        goto exit;
    }

    (void)pData->serversList->remove(pData->serversList, (void*)params->name);
    (void)pData->serversList->unlock(pData->serversList);

exit:
    return ret;
}

/*!
 *
 */
static SERVER_ERROR_E addReceiver_f(SERVER_S *obj, SERVER_PARAMS_S *params, LINK_S *client)
{
    assert(obj && obj->pData && params && client);
    
    SERVER_CONTEXT_S *ctx = NULL;
    SERVER_ERROR_E ret    = SERVER_ERROR_NONE;
    
    if ((ret = getServerContext_f(obj, params->name, &ctx)) != SERVER_ERROR_NONE) {
        Loge("Failed to retrieve %s's context", params->name);
        goto exit;
    }
    
    if (ctx->clientsList->lock(ctx->clientsList) != LIST_ERROR_NONE) {
        Loge("Failed to lock clientsList");
        ret = SERVER_ERROR_LOCK;
        goto exit;
    }
    
    ((CLIENT_LINK_PDATA_S*)client->pData)->isAuthorizedReceiver = 1;
    
    (void)ctx->clientsList->unlock(ctx->clientsList);

exit:
    return ret;
}

/*!
 *
 */
static SERVER_ERROR_E removeReceiver_f(SERVER_S *obj, SERVER_PARAMS_S *params, LINK_S *client)
{
    assert(obj && obj->pData && params && client);
    
    SERVER_CONTEXT_S *ctx = NULL;
    SERVER_ERROR_E ret    = SERVER_ERROR_NONE;
    
    if ((ret = getServerContext_f(obj, params->name, &ctx)) != SERVER_ERROR_NONE) {
        Loge("Failed to retrieve %s's context", params->name);
        goto exit;
    }
    
    if (ctx->clientsList->lock(ctx->clientsList) != LIST_ERROR_NONE) {
        Loge("Failed to lock clientsList");
        ret = SERVER_ERROR_LOCK;
        goto exit;
    }
    
    ((CLIENT_LINK_PDATA_S*)client->pData)->isAuthorizedReceiver = 0;
    
    (void)ctx->clientsList->unlock(ctx->clientsList);

exit:
    return ret;
}

/*!
 *
 */
static SERVER_ERROR_E suspendSender_f(SERVER_S *obj, SERVER_PARAMS_S *params)
{
    assert(obj && obj->pData && params);

    SERVER_CONTEXT_S *ctx = NULL;
    SERVER_ERROR_E ret    = SERVER_ERROR_NONE;

    if ((ret = getServerContext_f(obj, params->name, &ctx)) != SERVER_ERROR_NONE) {
        Loge("Failed to retrieve %s's context", params->name);
        goto exit;
    }

    ctx->senderSuspended = 1;

    while (sem_trywait(&ctx->sem) == 0) {
        Logd("Resettig semaphore counter to zero");
    }

    if (pthread_mutex_lock(&ctx->lock) != 0) {
        ret = SERVER_ERROR_LOCK;
        goto exit;
    }

    ctx->bufferIn.length = 0;
    ctx->bufferIn.data   = NULL;

    (void)pthread_mutex_unlock(&ctx->lock);

exit:
    return ret;
}

/*!
 *
 */
static SERVER_ERROR_E resumeSender_f(SERVER_S *obj, SERVER_PARAMS_S *params)
{
    assert(obj && obj->pData && params);

    SERVER_CONTEXT_S *ctx = NULL;
    SERVER_ERROR_E ret    = SERVER_ERROR_NONE;

    if ((ret = getServerContext_f(obj, params->name, &ctx)) != SERVER_ERROR_NONE) {
        Loge("Failed to retrieve %s's context", params->name);
        goto exit;
    }

    ctx->senderSuspended = 0;

exit:
    return ret;
}

/*!
 *
 */
static SERVER_ERROR_E disconnectClient_f(SERVER_S *obj, SERVER_PARAMS_S *params, LINK_S *client)
{
    assert(obj && obj->pData && params && client);
    
    SERVER_CONTEXT_S *ctx = NULL;
    SERVER_ERROR_E ret    = SERVER_ERROR_NONE;
    
    if ((ret = getServerContext_f(obj, params->name, &ctx)) != SERVER_ERROR_NONE) {
        Loge("Failed to retrieve %s's context", params->name);
        goto exit;
    }
    
    if (ctx->clientsList->lock(ctx->clientsList) != LIST_ERROR_NONE) {
        Loge("Failed to lock clientsList");
        ret = SERVER_ERROR_LOCK;
        goto exit;
    }
        
    ctx->clientsList->remove(ctx->clientsList, (void*)&client->id);
    
    (void)ctx->clientsList->unlock(ctx->clientsList);

exit:
    return ret;
}

/*!
 *
 */
static SERVER_ERROR_E sendData_f(SERVER_S *obj, SERVER_PARAMS_S *params, BUFFER_S *buffer)
{
    assert(obj && obj->pData && params && buffer);
    
    SERVER_CONTEXT_S *ctx = NULL;
    SERVER_ERROR_E ret    = SERVER_ERROR_NONE;
    
    if ((ret = getServerContext_f(obj, params->name, &ctx)) != SERVER_ERROR_NONE) {
        Loge("Failed to retrieve %s's context", params->name);
        goto exit;
    }
    
    if (pthread_mutex_lock(&ctx->lock) != 0) {
        ret = SERVER_ERROR_LOCK;
        goto exit;
    }

    if (!ctx->senderSuspended) {
        ctx->bufferIn.length = buffer->length;
        ctx->bufferIn.data   = buffer->data;

        (void)sem_post(&ctx->sem);
    }

    (void)pthread_mutex_unlock(&ctx->lock);
    
exit:
    return ret;
}

/*!
 *
 */
static SERVER_ERROR_E openServerSocket_f(SERVER_CONTEXT_S *ctx, LINK_HELPER_S *linkHelper)
{
    assert(ctx && linkHelper && (ctx->server = calloc(1, sizeof(LINK_S))));
    
    ctx->server->domain = ((ctx->params.link == LINK_TYPE_INET_STREAM)
                            || (ctx->params.link == LINK_TYPE_INET_DGRAM))  ? AF_UNSPEC   : AF_UNIX;
    ctx->server->type   = ((ctx->params.link == LINK_TYPE_INET_STREAM)
                            || (ctx->params.link == LINK_TYPE_UNIX_STREAM)) ? SOCK_STREAM : SOCK_DGRAM;
    
    if (ctx->server->domain != AF_UNIX) {
        memset(&ctx->hints, '\0', sizeof(ADDRINFO));
        ctx->hints.ai_family   = ctx->server->domain;
        ctx->hints.ai_socktype = ctx->server->type;

        int status = getaddrinfo(ctx->params.recipient.server.host,
                                    ctx->params.recipient.server.service, &ctx->hints, &ctx->result);
        if (status != 0) {
            Loge("getaddrinfo() failed - %s", gai_strerror(status));
            goto exit;
        }
    }

next_addr:
    if (ctx->server->domain != AF_UNIX) {
        ctx->rp = ctx->rp ? ctx->rp->ai_next : ctx->result;
        
        if (!ctx->rp) {
            Loge("No address succeeded");
            goto exit;
        }
        
        ctx->server->domain = ctx->rp->ai_family;
        ctx->server->type   = ctx->rp->ai_socktype;
        
        if (ctx->server->domain == AF_INET) {
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
        
        inet_ntop(ctx->server->domain, ctx->addr, ctx->ipstr, sizeof(ctx->ipstr));
        Logd("Testing : %s - address : %s / port : %u", ctx->ipver, ctx->ipstr, ctx->port);
    }
    
    if ((ctx->server->sock = socket(ctx->server->domain, ctx->server->type, 0)) == INVALID_SOCKET) {
        if (ctx->server->domain != AF_UNIX) {
            goto next_addr;
        }
        Loge("Failed to create server socket");
        goto exit;
    }
    
    if (ctx->server->domain != AF_UNIX) {
        uint32_t sockopt = 1;
        if (setsockopt(ctx->server->sock, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt)) == SOCKET_ERROR) {
            Loge("setsockopt() failed %s", strerror(errno));
        }
        
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
        
        ctx->server->destAddress       = (SOCKADDR*)&(ctx->server->addr.sun);
        ctx->server->destAddressLength = sizeof(ctx->server->addr.sun);
        
        unlink(ctx->server->addr.sun.sun_path);
    }
        
    if (bind(ctx->server->sock, ctx->server->destAddress, ctx->server->destAddressLength) == SOCKET_ERROR) {
        if (ctx->server->domain != AF_UNIX) {
            close(ctx->server->sock);
            goto next_addr;
        }
        Loge("Failed to bind to server socket");
        goto exit;
    }
    
    if (ctx->server->type == SOCK_DGRAM) {
        ctx->server->useDestAddress = 1;
    }
    else if (listen(ctx->server->sock, ctx->params.maxClients) == SOCKET_ERROR) {
        if (ctx->server->domain != AF_UNIX) {
            close(ctx->server->sock);
            goto next_addr;
        }
        Loge("listen() failed");
        goto exit;
    }
    
    if (linkHelper->setBlocking(linkHelper, ctx->server, NO) == ERROR) {
        Loge("Failed to set server as non-blocking");
    }
    
    if (ctx->server->domain != AF_UNIX) {
        inet_ntop(ctx->server->domain, ctx->addr, ctx->ipstr, sizeof(ctx->ipstr));
        Logd("Using : %s - address : %s / port : %d", ctx->ipver, ctx->ipstr, ctx->port);
        
        freeaddrinfo(ctx->result);
        ctx->result = NULL;
    }
    
    return SERVER_ERROR_NONE;
    
exit:
    if (ctx->server->sock != INVALID_SOCKET) {
        close(ctx->server->sock);
    }
    
    free(ctx->server);
    ctx->server = NULL;
    
    return SERVER_ERROR_INIT;
}

/*!
 *
 */
static SERVER_ERROR_E closeServerSocket_f(SERVER_CONTEXT_S *ctx)
{
    assert(ctx);
    
    if (!ctx->server) {
        return SERVER_ERROR_NONE;
    }
    
    if (ctx->server->sock != INVALID_SOCKET) {
        shutdown(ctx->server->sock, SHUT_RDWR);
    }
    
    free(ctx->server);
    ctx->server = NULL;
    
    return SERVER_ERROR_NONE;
}

/*!
 *
 */
static SERVER_ERROR_E getServerContext_f(SERVER_S *obj, char *serverName, SERVER_CONTEXT_S **ctxOut)
{
    assert(obj && obj->pData && serverName && ctxOut);
    
    SERVER_PRIVATE_DATA_S *pData = (SERVER_PRIVATE_DATA_S*)(obj->pData);
    
    if (!pData->serversList) {
        Loge("There is currently no element in list");
        return SERVER_ERROR_LIST;
    }
    
    SERVER_ERROR_E ret = SERVER_ERROR_NONE;
    
    if (pData->serversList->lock(pData->serversList) != LIST_ERROR_NONE) {
        Loge("Failed to lock serversList");
        return SERVER_ERROR_LIST;
    }
    
    uint32_t nbElements;
    if (pData->serversList->getNbElements(pData->serversList, &nbElements) != LIST_ERROR_NONE) {
        Loge("Failed to get number of elements");
        ret = SERVER_ERROR_LIST;
        goto exit;
    }
    
    while (nbElements > 0) {
        if (pData->serversList->getElement(pData->serversList, (void**)ctxOut) != LIST_ERROR_NONE) {
            Loge("Failed to retrieve element");
            ret = SERVER_ERROR_LIST;
            goto exit;
        }
        
        if (!strcmp((*ctxOut)->params.name, serverName)) {
            break;
        }
            
        nbElements--;
    }
        
    if (nbElements == 0) {
        //Loge("Element %s not found", serverName);
        ret = SERVER_ERROR_LIST;
        goto exit;
    }

exit:
    (void)pData->serversList->unlock(pData->serversList);
        
    return ret;
}

/*!
 *
 */
static void watcherTaskFct_f(TASK_PARAMS_S *params)
{
    assert(params && params->fctData && params->userData);
    
    SERVER_CONTEXT_S      *ctx   = (SERVER_CONTEXT_S*)params->fctData;
    SERVER_PRIVATE_DATA_S *pData = (SERVER_PRIVATE_DATA_S*)params->userData;
    
    if (pData->linkHelper->isReadyForReading(pData->linkHelper, ctx->server, WAIT_TIME_2S) == NO) {
        return;
    }
    
    LINK_S *client;
    assert((client = calloc(1, sizeof(LINK_S))));
    
    client->useDestAddress = ctx->server->useDestAddress;
    
    if (ctx->server->domain != AF_UNIX) { // TCP or UDP
        client->destAddressLength = sizeof(client->addr.storage);
        client->destAddress       = (SOCKADDR*)&client->addr.storage;
    }
    else { // Unix
        client->destAddressLength = sizeof(client->addr.sun);
        client->destAddress       = (SOCKADDR*)&client->addr.sun;
    }
    
    if (!client->useDestAddress) { // Unix or TCP (Connection oriented)
        if ((client->sock = accept(ctx->server->sock, client->destAddress, &client->destAddressLength)) == SOCKET_ERROR) {
            Loge("accept() failed");
            goto exit;
        }
        
        if (ctx->params.mode == LINK_MODE_STANDARD) {
            goto standard;
        }
        
        if (pData->linkHelper->isReadyForReading(pData->linkHelper, client, WAIT_TIME_10MS) == NO) {
            Loge("No data read on new client socket");
            goto exit;
        }
        
        if (ctx->params.mode == LINK_MODE_CUSTOM) {
            // Get Custom header
            ctx->watcherTempBuffer.data   = (void*)ctx->customHeader.str;
            ctx->watcherTempBuffer.length = sizeof(ctx->customHeader.str);
            
            if (pData->linkHelper->readData(pData->linkHelper, client, NULL, &ctx->watcherTempBuffer, NULL) == ERROR) {
                Loge("Failed to receive data from client");
                goto exit;
            }
            
            pData->linkHelper->parseCustomHeader(pData->linkHelper, &ctx->customHeader);
            Logd("Custom header : %s", ctx->customHeader.str);
        
            strcpy(ctx->customContent.mime, ctx->params.mime);
            ctx->customContent.maxBufferSize = ctx->params.maxBufferSize;
            pData->linkHelper->prepareCustomContent(pData->linkHelper, &ctx->customContent);
            Logd("Custom Content : %s", ctx->customContent.str);
            
            if (pData->linkHelper->isReadyForWriting(pData->linkHelper, client, WAIT_TIME_10MS) == NO) {
                Loge("New client is not ready for writing");
                goto exit;
            }
            
            ctx->watcherTempBuffer.data   = (void*)ctx->customContent.str;
            ctx->watcherTempBuffer.length = strlen(ctx->customContent.str);
            
            if (pData->linkHelper->writeData(pData->linkHelper, client, NULL, &ctx->watcherTempBuffer, NULL) == ERROR) {
                Loge("Failed to send customContent to client");
                goto exit;
            }
        }
        else if (ctx->params.mode == LINK_MODE_HTTP) {
            if (ctx->server->domain == AF_UNIX) {
                Loge("Bad domain - Inet is expected for HTTP");
                return;
            }
            
            // Http Get
            ctx->watcherTempBuffer.data   = (void*)ctx->httpGet.str;
            ctx->watcherTempBuffer.length = sizeof(ctx->httpGet.str);
            
            if (pData->linkHelper->readData(pData->linkHelper, client, NULL, &ctx->watcherTempBuffer, NULL) == ERROR) {
                Loge("Failed to receive data from client");
                goto exit;
            }
            
            pData->linkHelper->parseHttpGet(pData->linkHelper, &ctx->httpGet);
            Logd("Http Get : %s", ctx->httpGet.str);
            
            if (!ctx->httpGet.isHttpGet || strcmp(ctx->params.recipient.server.path, ctx->httpGet.path)) {
                Loge("Bad HTTP request");
                
                if (!ctx->httpGet.isHttpGet) {
                    strcpy(ctx->http400BadRequest.ip, ctx->ipstr);
                    ctx->http400BadRequest.port = ctx->port;
                    strcpy(ctx->http400BadRequest.path, ctx->params.recipient.server.path);
                        
                    pData->linkHelper->prepareHttp400BadRequest(pData->linkHelper, &ctx->http400BadRequest);
                    Logd("Http 400 Bad Request : %s", ctx->http400BadRequest.str);
                    
                    ctx->watcherTempBuffer.data   = (void*)ctx->http400BadRequest.str;
                    ctx->watcherTempBuffer.length = strlen(ctx->http400BadRequest.str);
                }
                else {
                    strcpy(ctx->http404NotFound.ip, ctx->ipstr);
                    ctx->http404NotFound.port = ctx->port;
                    strcpy(ctx->http404NotFound.path, ctx->params.recipient.server.path);
                    strcpy(ctx->http404NotFound.requestedPath, ctx->httpGet.path);

                    pData->linkHelper->prepareHttp404NotFound(pData->linkHelper, &ctx->http404NotFound);
                    Logd("Http 404 Not Found : %s", ctx->http404NotFound.str);
                    
                    ctx->watcherTempBuffer.data   = (void*)ctx->http404NotFound.str;
                    ctx->watcherTempBuffer.length = strlen(ctx->http404NotFound.str);
                }
                
                if ((pData->linkHelper->isReadyForWriting(pData->linkHelper, client, WAIT_TIME_10MS) == NO)
                    || (pData->linkHelper->writeData(pData->linkHelper, client, NULL, &ctx->watcherTempBuffer, NULL) == ERROR)) {
                    Loge("Failed to send HTTP 40X to client");
                }
                
                goto exit;
            }
            
            if (pData->linkHelper->isReadyForWriting(pData->linkHelper, client, WAIT_TIME_10MS) == NO) {
                Loge("New client is not ready for writing");
                goto exit;
            }
            
            pData->linkHelper->prepareHttp200Ok(pData->linkHelper, &ctx->http200Ok);
            Logd("Http 200 OK : %s", ctx->http200Ok.str);
             
            ctx->watcherTempBuffer.data   = (void*)ctx->http200Ok.str;
            ctx->watcherTempBuffer.length = strlen(ctx->http200Ok.str);
            
            if (pData->linkHelper->writeData(pData->linkHelper, client, NULL, &ctx->watcherTempBuffer, NULL) == ERROR) {
                Loge("Failed to send http200Ok to client");
                goto exit;
            }
        }

standard:
        if (pData->linkHelper->setBlocking(pData->linkHelper, client, NO) == ERROR) {
            Loge("Failed to set client as non-blocking");
        }
    }
    else { // Unix or UDP (connectionless)
        if (ctx->params.mode == LINK_MODE_HTTP) {
            Loge("Bad domain - Inet is expected for HTTP");
            goto exit;
        }
        
        // Read something from client to get its address
        ctx->watcherTempBuffer.data   = (void*)ctx->customHeader.str;
        ctx->watcherTempBuffer.length = sizeof(ctx->customHeader.str);
            
        if (pData->linkHelper->readData(pData->linkHelper, ctx->server, client, &ctx->watcherTempBuffer, NULL) == ERROR) {
            Loge("Failed to receive data from client");
            goto exit;
        }
        
        pData->linkHelper->parseCustomHeader(pData->linkHelper, &ctx->customHeader);
        Logd("Custom header : %s", ctx->customHeader.str);
            
        if (ctx->params.mode == LINK_MODE_CUSTOM) {
            strcpy(ctx->customContent.mime, ctx->params.mime);
            ctx->customContent.maxBufferSize = ctx->params.maxBufferSize;
            pData->linkHelper->prepareCustomContent(pData->linkHelper, &ctx->customContent);
            Logd("Custom Content : %s", ctx->customContent.str);
            
            if (pData->linkHelper->isReadyForWriting(pData->linkHelper, ctx->server, WAIT_TIME_10MS) == NO) {
                Loge("New client is not ready for writing");
                goto exit;
            }
            
            ctx->watcherTempBuffer.data   = (void*)ctx->customContent.str;
            ctx->watcherTempBuffer.length = strlen(ctx->customContent.str);
                
            if (pData->linkHelper->writeData(pData->linkHelper, ctx->server, client, &ctx->watcherTempBuffer, NULL) == ERROR) {
                Loge("Failed to send customContent to client");
                goto exit;
            }
        }
    }

    assert((client->pData = calloc(1, sizeof(CLIENT_LINK_PDATA_S))));

    if (ctx->clientsList->lock(ctx->clientsList) != LIST_ERROR_NONE) {
        Loge("Failed to lock clientsList");
        goto exit_calloc;
    }

    (void)ctx->clientsList->getNbElements(ctx->clientsList, &client->id);
    client->id += (uint32_t)time(NULL);

    if (ctx->params.acceptMode == SERVER_ACCEPT_MODE_AUTOMATIC) {
        ((CLIENT_LINK_PDATA_S*)client->pData)->isAuthorizedReceiver = 1;
    }
    
    ctx->clientsList->add(ctx->clientsList, (void*)client);
    
    (void)ctx->clientsList->unlock(ctx->clientsList);
    
    if (ctx->params.onClientStateChangedCb) {
        ctx->params.onClientStateChangedCb(&ctx->params, client, STATE_CONNECTED, ctx->params.userData);
    }
    
    return;

exit_calloc:
    free(client->pData);
    client->pData = NULL;
 
exit:
    if (client->sock != INVALID_SOCKET) {
        close(client->sock);
    }
    
    free(client);
    client = NULL;
}

/*!
 *
 */
static void senderTaskFct_f(TASK_PARAMS_S *params)
{
    assert(params && params->fctData && params->userData);
    
    SERVER_CONTEXT_S      *ctx   = (SERVER_CONTEXT_S*)params->fctData;
    SERVER_PRIVATE_DATA_S *pData = (SERVER_PRIVATE_DATA_S*)params->userData;
    
    if (ctx->quit) {
        return;
    }
    
    sem_wait(&ctx->sem);
    
    if (ctx->quit) {
        return;
    }

    if (ctx->senderSuspended) {
        return;
    }

    if (ctx->clientsList->lock(ctx->clientsList) != LIST_ERROR_NONE) {
        Logd("Failed to lock clientsList");
        return;
    }
    
    uint32_t nbClients;
    if (ctx->clientsList->getNbElements(ctx->clientsList, &nbClients) == LIST_ERROR_NONE) {
        if (nbClients > 0) {
            if (pthread_mutex_lock(&ctx->lock) != 0) {
                goto exit;
            }

            if (ctx->bufferIn.data && (ctx->bufferIn.length != 0)) {
                ctx->bufferOut.length = ctx->bufferIn.length;
                assert((ctx->bufferOut.data = calloc(1, ctx->bufferOut.length)));
                memcpy(ctx->bufferOut.data, ctx->bufferIn.data, ctx->bufferOut.length);
            }
            else {
                nbClients = 0; // Force exit!
            }

            (void)pthread_mutex_unlock(&ctx->lock);
        }
        else {
            goto exit;
        }
        
        LINK_S *client = NULL;
        while (nbClients > 0) {
            nbClients--;
            
            if (ctx->clientsList->getElement(ctx->clientsList, (void*)&client) != LIST_ERROR_NONE) {
                break;
            }
            
            if (((CLIENT_LINK_PDATA_S*)client->pData)->isAuthorizedReceiver != 1) {
                continue;
            }
            
            if (pData->linkHelper->isReadyForWriting(pData->linkHelper, client->useDestAddress ? ctx->server : client, 0) == NO) {
                continue;
            }
            
            if (ctx->params.mode == LINK_MODE_HTTP) {
                strcpy(ctx->httpContent.mime, ctx->params.mime);
                ctx->httpContent.length = ctx->bufferOut.length;
                pData->linkHelper->prepareHttpContent(pData->linkHelper, &ctx->httpContent);
                
                ctx->senderTempBuffer.data   = (void*)ctx->httpContent.str;
                ctx->senderTempBuffer.length = strlen(ctx->httpContent.str);
                    
                if (pData->linkHelper->writeData(pData->linkHelper, client->useDestAddress ? ctx->server : client,
                                                    client->useDestAddress ? client : NULL, &ctx->senderTempBuffer, NULL) == ERROR) {
                    if (ctx->params.onClientStateChangedCb) {
                        ctx->params.onClientStateChangedCb(&ctx->params, client, STATE_DISCONNECTED, ctx->params.userData);
                    }
                    
                    (void)ctx->clientsList->remove(ctx->clientsList, (void*)&client->id); // Client disconnected
                    goto next_client;
                }
            }
            
            if (pData->linkHelper->writeData(pData->linkHelper, client->useDestAddress ? ctx->server : client,
                                                client->useDestAddress ? client : NULL, &ctx->bufferOut, NULL) == ERROR) {
                if (ctx->params.onClientStateChangedCb) {
                    ctx->params.onClientStateChangedCb(&ctx->params, client, STATE_DISCONNECTED, ctx->params.userData);
                }
                
                (void)ctx->clientsList->remove(ctx->clientsList, (void*)&client->id); // Client disconnected
                goto next_client;
            }

next_client:
            ;
        }
        
        (void)pthread_mutex_lock(&ctx->lock);
        
        if (ctx->bufferOut.data) {
            free(ctx->bufferOut.data);
            ctx->bufferOut.data = NULL;
        }
            
        (void)pthread_mutex_unlock(&ctx->lock);
    }

exit:
    (void)ctx->clientsList->unlock(ctx->clientsList);
}

/*!
 *
 */
static uint8_t compareServerCb(LIST_S *obj, void *elementToCheck, void *userData)
{
    assert(obj && elementToCheck && userData);
    
    SERVER_CONTEXT_S *ctx       = (SERVER_CONTEXT_S*)elementToCheck;
    char *nameOfElementToRemove = (char*)userData;
    
    return (!strcmp(nameOfElementToRemove, ctx->params.name));
}

/*!
 *
 */
static void releaseServerCb(LIST_S *obj, void *element)
{
    assert(obj && element);
    
    SERVER_CONTEXT_S *ctx = (SERVER_CONTEXT_S*)element;;
    
    /* Stop and uninit tasks */
    ctx->quit = 1;
    sem_post(&ctx->sem);

    (void)ctx->serverTask->stop(ctx->serverTask, &ctx->watcherTaskParams);
    (void)ctx->serverTask->stop(ctx->serverTask, &ctx->senderTaskParams);
    
    (void)ctx->serverTask->destroy(ctx->serverTask, &ctx->watcherTaskParams);
    (void)ctx->serverTask->destroy(ctx->serverTask, &ctx->senderTaskParams);
    
    (void)Task_UnInit(&ctx->serverTask);
    
    /* Destroy sem and mutex */
    (void)pthread_mutex_destroy(&ctx->lock);
    (void)sem_destroy(&ctx->sem);
    
    /* Uninit clientsList */
    (void)ctx->clientsList->lock(ctx->clientsList);
    ctx->clientsList->removeAll(ctx->clientsList);
    (void)ctx->clientsList->unlock(ctx->clientsList);
        
    (void)List_UnInit(&ctx->clientsList);
    
    /* Close socket */
    (void)closeServerSocket_f(ctx);
    
    free(ctx);
    ctx = NULL;
}

/*!
 *
 */
static uint8_t compareClientCb(LIST_S *obj, void *elementToCheck, void *userData)
{
    assert(obj && elementToCheck && userData);
    
    LINK_S *client               = (LINK_S*)elementToCheck;
    uint32_t idOfElementToRemove = *((uint32_t*)userData);
    
    return (idOfElementToRemove == client->id);
}

/*!
 *
 */
static void releaseClientCb(LIST_S *obj, void *element)
{
    assert(obj && element);
    
    LINK_S *client = (LINK_S*)element;
    
    close(client->sock);
    
    if (client->pData) {
        free(client->pData);
        client->pData = NULL;
    }
    
    free(client);
    client = NULL;
}
