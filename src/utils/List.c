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
* \file List.c
* \brief Lists management
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include <pthread.h>

#include "utils/Log.h"
#include "utils/List.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "List"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct list_element_s {
    void                  *element;
    struct list_element_s *next;
};

struct list_private_data_s {
    uint32_t              nbElements;
    struct list_element_s *list;
    struct list_element_s *current;
    pthread_mutex_t       lock;
};

/* -------------------------------------------------------------------------------------------- */
/*                                 PUBLIC FUNCTIONS PROTOTYPES                                  */
/* -------------------------------------------------------------------------------------------- */

static enum list_error_e add_f(struct list_s *obj, void *element);
static enum list_error_e remove_f(struct list_s *obj, const void *userData);
static enum list_error_e removeAll_f(struct list_s *obj);

static enum list_error_e getNbElements_f(struct list_s *obj, uint32_t *nbElements);
static enum list_error_e getElement_f(struct list_s *obj, void **element);
static enum list_error_e browseElements_f(struct list_s *obj, const void *userData);

static enum list_error_e lock_f(struct list_s *obj);
static enum list_error_e unlock_f(struct list_s *obj);

/* -------------------------------------------------------------------------------------------- */
/*                                         INITIALIZER                                          */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum list_error_e List_Init(struct list_s **obj, struct list_params_s *params)
{
    ASSERT(obj && params);
    
    ASSERT((*obj = calloc(1, sizeof(struct list_s))));
    
    (*obj)->params = *params;
    
    struct list_private_data_s *pData;
    ASSERT((pData = calloc(1, sizeof(struct list_private_data_s))));
    
    if (pthread_mutex_init(&pData->lock, NULL) != 0) {
        Loge("pthread_mutex_init() failed");
        goto exit;
    }
    
    (*obj)->add            = add_f;
    (*obj)->remove         = remove_f;
    (*obj)->removeAll      = removeAll_f;
    (*obj)->getNbElements  = getNbElements_f;
    (*obj)->getElement     = getElement_f;
    (*obj)->browseElements = browseElements_f;
    (*obj)->lock           = lock_f;
    (*obj)->unlock         = unlock_f;
    
    (*obj)->pData = (void*)pData;
    
    return LIST_ERROR_NONE;

exit:
    free(pData);
    free(*obj);
    *obj = NULL;

    return LIST_ERROR_INIT;
}

/*!
 *
 */
enum list_error_e List_UnInit(struct list_s **obj)
{
    ASSERT(obj && *obj && (*obj)->pData);
    
    enum list_error_e ret             = LIST_ERROR_NONE;
    struct list_private_data_s *pData = (struct list_private_data_s*)((*obj)->pData);
    
    if (pthread_mutex_destroy(&pData->lock) != 0) {
        Loge("pthread_mutex_destroy() failed");
        ret = LIST_ERROR_UNINIT;
    }
    
    free(pData);
    free(*obj);
    *obj = NULL;
    
    return ret;
}

/* -------------------------------------------------------------------------------------------- */
/*                               PUBLIC FUNCTIONS IMPLEMENTATION                                */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
static enum list_error_e add_f(struct list_s *obj, void *element)
{
    ASSERT(obj && obj->pData && element);
    
    struct list_element_s *newElement = NULL;
    struct list_private_data_s *pData = (struct list_private_data_s*)obj->pData;
    
    ASSERT((newElement = calloc(1, sizeof(struct list_element_s))));
    
    newElement->element = element;
    newElement->next    = NULL;
    
    struct list_element_s *list = NULL;
    
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
static enum list_error_e remove_f(struct list_s *obj, const void *userData)
{
    ASSERT(obj && obj->pData && userData);
    
    struct list_private_data_s *pData = (struct list_private_data_s*)obj->pData;
    
    if (!pData->list) {
        return LIST_ERROR_PARAMS;
    }
    
    struct list_element_s *previous = NULL;
    struct list_element_s *current  = pData->list;
    uint32_t elementFound           = 0;
    
    while (current) {
        if (obj->params.compareCb
            && obj->params.compareCb(obj, current->element, userData)) {
            if (!previous) {
                pData->list = current->next;
            }
            else {
                previous->next = current->next;
            }
            if (obj->params.releaseCb) {
                // A release callback might be not needed in case there is no
                // resource to release in "element" (E.g. a simple list of integers)
                obj->params.releaseCb(obj, current->element);
            }
            free(current);
            pData->nbElements--;
            elementFound = 1;
            break;
        }
        previous = current;
        current  = current->next;
    }
    
    return elementFound ? LIST_ERROR_NONE : LIST_ERROR_PARAMS;
}

/*!
 *
 */
static enum list_error_e removeAll_f(struct list_s *obj)
{
    ASSERT(obj && obj->pData);
    
    struct list_private_data_s *pData = (struct list_private_data_s*)obj->pData;
    
    if (!pData->list) {
        return LIST_ERROR_PARAMS;
    }
    
    struct list_element_s *current = pData->list;
    struct list_element_s *next    = current->next;
    
    while (current) {
        if (obj->params.releaseCb) {
            obj->params.releaseCb(obj, current->element);
        }
        free(current);
        if ((current = next)) {
            next = current->next;
        }
    }
    
    pData->list       = NULL;
    pData->nbElements = 0;
    
    return LIST_ERROR_NONE;
}

/*!
 *
 */
static enum list_error_e getNbElements_f(struct list_s *obj, uint32_t *nbElements)
{
    ASSERT(obj && obj->pData && nbElements);
    
    struct list_private_data_s *pData = (struct list_private_data_s*)obj->pData;
    
    *nbElements = pData->nbElements;
    
    return LIST_ERROR_NONE;
}

/*!
 *
 */
static enum list_error_e getElement_f(struct list_s *obj, void **element)
{
    ASSERT(obj && obj->pData && element);
    
    struct list_private_data_s *pData = (struct list_private_data_s*)obj->pData;

    if (!pData->list) {
        return LIST_ERROR_PARAMS;
    }

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
static enum list_error_e browseElements_f(struct list_s *obj, const void *userData)
{
    ASSERT(obj && obj->pData);
    
    struct list_private_data_s *pData = (struct list_private_data_s*)obj->pData;

    if (!obj->params.browseCb || !pData->list) {
        return LIST_ERROR_PARAMS;
    }

    struct list_element_s *current = pData->list;
    
    while (current) {
        obj->params.browseCb(obj, current->element, userData);
        current = current->next;
    }
    
    return LIST_ERROR_NONE;
}

/*!
 *
 */
static enum list_error_e lock_f(struct list_s *obj)
{
    ASSERT(obj && obj->pData);
    
    struct list_private_data_s *pData = (struct list_private_data_s*)obj->pData;
    
    if (pData && (pthread_mutex_lock(&pData->lock) != 0)) {
        Loge("pthread_mutex_lock() failed");
        return LIST_ERROR_LOCK;
    }

    pData->current = NULL;

    return LIST_ERROR_NONE;
}

/*!
 *
 */
static enum list_error_e unlock_f(struct list_s *obj)
{
    ASSERT(obj && obj->pData);
    
    struct list_private_data_s *pData = (struct list_private_data_s*)obj->pData;
    
    if (pData && (pthread_mutex_unlock(&pData->lock) != 0)) {
        Loge("pthread_mutex_unlock() failed");
        return LIST_ERROR_LOCK;
    }
    
    return LIST_ERROR_NONE;
}
