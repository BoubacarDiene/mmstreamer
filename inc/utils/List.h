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
* \file   List.h
* \author Boubacar DIENE
*/

#ifndef __LIST_H__
#define __LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include "utils/Common.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

typedef enum   LIST_ERROR_E  LIST_ERROR_E;

typedef struct LIST_PARAMS_S LIST_PARAMS_S;
typedef struct LIST_S        LIST_S;

typedef uint8_t (*LIST_COMPARE_CB)(LIST_S *obj, void *elementToCheck, void *userData);
typedef void    (*LIST_RELEASE_CB)(LIST_S *obj, void *element);
typedef void    (*LIST_BROWSE_CB )(LIST_S *obj, void *element, void *dataProvidedToBrowseFunction);

typedef LIST_ERROR_E (*LIST_ADD_F       )(LIST_S *obj, void *element);
typedef LIST_ERROR_E (*LIST_REMOVE_F    )(LIST_S *obj, void *userData);
typedef LIST_ERROR_E (*LIST_REMOVE_ALL_F)(LIST_S *obj);

typedef LIST_ERROR_E (*LIST_GET_NB_ELEMENTS_F)(LIST_S *obj, uint32_t *nbElements);
typedef LIST_ERROR_E (*LIST_GET_ELEMENT_F    )(LIST_S *obj, void **element);
typedef LIST_ERROR_E (*LIST_BROWSE_ELEMENTS_F)(LIST_S *obj, void *userData);

typedef LIST_ERROR_E (*LIST_LOCK_F  )(LIST_S *obj);
typedef LIST_ERROR_E (*LIST_UNLOCK_F)(LIST_S *obj);

enum LIST_ERROR_E {
    LIST_ERROR_NONE,
    LIST_ERROR_INIT,
    LIST_ERROR_UNINIT,
    LIST_ERROR_LOCK,
    LIST_ERROR_PARAMS
};

struct LIST_PARAMS_S {
    LIST_COMPARE_CB compareCb;
    LIST_RELEASE_CB releaseCb;
    LIST_BROWSE_CB  browseCb;
};

struct LIST_S {
    LIST_PARAMS_S          params;
    
    LIST_ADD_F             add;
    LIST_REMOVE_F          remove;
    LIST_REMOVE_ALL_F      removeAll;
    
    LIST_GET_NB_ELEMENTS_F getNbElements;
    LIST_GET_ELEMENT_F     getElement;
    LIST_BROWSE_ELEMENTS_F browseElements;
    
    LIST_LOCK_F            lock;
    LIST_UNLOCK_F          unlock;
    
    void                   *pData;
};

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

LIST_ERROR_E List_Init  (LIST_S **obj, LIST_PARAMS_S *params);
LIST_ERROR_E List_UnInit(LIST_S **obj);

#ifdef __cplusplus
}
#endif

#endif //__LIST_H__
