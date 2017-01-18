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
* \file   List.c
* \brief  Lists management
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include <pthread.h>

#include "utils/List.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "LIST"
    
/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef struct LIST_ELEMENT_S {
    void                  *element;
    struct LIST_ELEMENT_S *next;
} LIST_ELEMENT_S;

typedef struct LIST_PRIVATE_DATA_S {
    uint32_t           nbElements;
    LIST_ELEMENT_S     *list;
    LIST_ELEMENT_S     *current;
    pthread_mutex_t    lock;
} LIST_PRIVATE_DATA_S;

/* -------------------------------------------------------------------------------------------- */
/*                                          VARIABLES                                           */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

static LIST_ERROR_E add_f      (LIST_S *obj, void *element);
static LIST_ERROR_E remove_f   (LIST_S *obj, void *userData);
static LIST_ERROR_E removeAll_f(LIST_S *obj);

static LIST_ERROR_E getNbElements_f (LIST_S *obj, uint32_t *nbElements);
static LIST_ERROR_E getElement_f    (LIST_S *obj, void **element);
static LIST_ERROR_E browseElements_f(LIST_S *obj, void *userData);

static LIST_ERROR_E lock_f  (LIST_S *obj);
static LIST_ERROR_E unlock_f(LIST_S *obj);

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
LIST_ERROR_E List_Init(LIST_S **obj, LIST_PARAMS_S *params)
{
    assert(obj && params);
    
    assert((*obj = calloc(1, sizeof(LIST_S))));
    
    memset(&(*obj)->params, '\0', sizeof(LIST_PARAMS_S));
    
    (*obj)->params.compareCb = params->compareCb;
    (*obj)->params.releaseCb = params->releaseCb;
    (*obj)->params.browseCb  = params->browseCb;
    
    (*obj)->add            = add_f;
    (*obj)->remove         = remove_f;
    (*obj)->removeAll      = removeAll_f;
    (*obj)->getNbElements  = getNbElements_f;
    (*obj)->getElement     = getElement_f;
    (*obj)->browseElements = browseElements_f;
    (*obj)->lock           = lock_f;
    (*obj)->unlock         = unlock_f;
    
    LIST_PRIVATE_DATA_S *pData;
    assert((pData = calloc(1, sizeof(LIST_PRIVATE_DATA_S))));
    
    if (pthread_mutex_init(&pData->lock, NULL) != 0) {
        goto exit;
    }
    
    (*obj)->pData = (void*)pData;
    
    return LIST_ERROR_NONE;

exit:
    free(pData);
    pData = NULL;

    free(*obj);
    *obj = NULL;

    return LIST_ERROR_INIT;
}

/*!
 *
 */
LIST_ERROR_E List_UnInit(LIST_S **obj)
{
    assert(obj && *obj && (*obj)->pData);
    
    LIST_ERROR_E        ret    = LIST_ERROR_NONE;
    LIST_PRIVATE_DATA_S *pData = (LIST_PRIVATE_DATA_S*)((*obj)->pData);
    
    if (pthread_mutex_destroy(&pData->lock) != 0) {
        ret = LIST_ERROR_UNINIT;
    }
    
    free(pData);
    pData = NULL;
    
    free(*obj);
    *obj = NULL;
    
    return ret;
}

/* -------------------------------------------------------------------------------------------- */
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static LIST_ERROR_E add_f(LIST_S *obj, void *element)
{
    assert(obj && obj->pData && element);
    
    LIST_ELEMENT_S *newElement = NULL;
    LIST_PRIVATE_DATA_S *pData = (LIST_PRIVATE_DATA_S*)obj->pData;
    
    assert((newElement = calloc(1, sizeof(LIST_ELEMENT_S))));
    
    newElement->element = element;
    newElement->next    = NULL;
    
    LIST_ELEMENT_S *list = NULL;
    
    if (!pData->list) {
        pData->list = newElement;
    }
    else {
        list = pData->list;
        while (list->next) {
            list = list->next;
        }
        list->next = newElement;
    }
    
    pData->nbElements++;
    
    return LIST_ERROR_NONE;
}

/*!
 *
 */
static LIST_ERROR_E remove_f(LIST_S *obj, void *userData)
{
    assert(obj && obj->pData && userData);
    
    LIST_PRIVATE_DATA_S *pData = (LIST_PRIVATE_DATA_S*)obj->pData;
    
    if (!pData->list) {
        return LIST_ERROR_PARAMS;
    }
    
    LIST_ELEMENT_S *previous = NULL;
    LIST_ELEMENT_S *current  = pData->list;
    
    while (current) {
        if (obj->params.compareCb(obj, current->element, userData)) {
            if (!previous) {
                pData->list = current->next;
            }
            else {
                previous->next = current->next;
            }
            obj->params.releaseCb(obj, current->element);
            free(current);
            current = NULL;
            pData->nbElements--;
            break;
        }
        previous = current;
        current  = current->next;
    }
    
    return LIST_ERROR_NONE;
}

/*!
 *
 */
static LIST_ERROR_E removeAll_f(LIST_S *obj)
{
    assert(obj && obj->pData);
    
    LIST_PRIVATE_DATA_S *pData = (LIST_PRIVATE_DATA_S*)obj->pData;
    
    if (!pData->list) {
        return LIST_ERROR_PARAMS;
    }
    
    LIST_ELEMENT_S *current  = pData->list;
    LIST_ELEMENT_S *next     = current->next;
    
    while (current) {
        obj->params.releaseCb(obj, current->element);
        free(current);
        if ((current = next)) {
            next = current->next;
        }
        else {
            next = NULL;
        }
    }
    
    pData->list       = NULL;
    pData->nbElements = 0;
    
    return LIST_ERROR_NONE;
}

/*!
 *
 */
static LIST_ERROR_E getNbElements_f(LIST_S *obj, uint32_t *nbElements)
{
    assert(obj && obj->pData && nbElements);
    
    LIST_PRIVATE_DATA_S *pData = (LIST_PRIVATE_DATA_S*)obj->pData;
    
    *nbElements = pData->nbElements;
    
    return LIST_ERROR_NONE;
}

/*!
 *
 */
static LIST_ERROR_E getElement_f(LIST_S *obj, void **element)
{
    assert(obj && obj->pData && element);
    
    LIST_PRIVATE_DATA_S *pData = (LIST_PRIVATE_DATA_S*)obj->pData;
    
    if (!pData->current) {
        pData->current = pData->list;
    }
    *element = pData->current->element;
    pData->current = pData->current->next;
    
    return LIST_ERROR_NONE;
}

/*!
 *
 */
static LIST_ERROR_E browseElements_f(LIST_S *obj, void *userData)
{
    assert(obj && obj->pData);
    
    LIST_PRIVATE_DATA_S *pData = (LIST_PRIVATE_DATA_S*)obj->pData;
    
    LIST_ELEMENT_S *current  = pData->list;
    
    while (current) {
        obj->params.browseCb(obj, current->element, userData);
        current = current->next;
    }
    
    return LIST_ERROR_NONE;
}

/*!
 *
 */
static LIST_ERROR_E lock_f(LIST_S *obj)
{
    assert(obj && obj->pData);
    
    LIST_PRIVATE_DATA_S *pData = (LIST_PRIVATE_DATA_S*)obj->pData;
    
    if (pData && (pthread_mutex_lock(&pData->lock) != 0)) {
        return LIST_ERROR_LOCK;
    }
    
    return LIST_ERROR_NONE;
}

/*!
 *
 */
static LIST_ERROR_E unlock_f(LIST_S *obj)
{
    assert(obj && obj->pData);
    
    LIST_PRIVATE_DATA_S *pData = (LIST_PRIVATE_DATA_S*)obj->pData;
    
    if (pData && (pthread_mutex_unlock(&pData->lock) != 0)) {
        return LIST_ERROR_LOCK;
    }
    
    return LIST_ERROR_NONE;
}
