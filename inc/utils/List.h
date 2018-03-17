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
* \file List.h
* \author Boubacar DIENE
*/

#ifndef __LIST_H__
#define __LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "utils/Common.h"

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////// TYPES DECLARATION ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum list_error_e;

struct list_params_s;
struct list_s;

/* -------------------------------------------------------------------------------------------- */
/* //////////////////////////////////////// CALLBACKS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef uint8_t (*list_compare_cb)(struct list_s *obj, void *elementToCheck, void *userData);
typedef void (*list_release_cb)(struct list_s *obj, void *element);
typedef void (*list_browse_cb)(struct list_s *obj, void *element, void *userData);

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

typedef enum list_error_e (*list_add_f)(struct list_s *obj, void *element);
typedef enum list_error_e (*list_remove_f)(struct list_s *obj, void *userData);
typedef enum list_error_e (*list_remove_all_f)(struct list_s *obj);

typedef enum list_error_e (*list_get_nb_elements_f)(struct list_s *obj, uint32_t *nbElements);
typedef enum list_error_e (*list_get_element_f)(struct list_s *obj, void **element);
typedef enum list_error_e (*list_browse_elements_f)(struct list_s *obj, void *userData);

typedef enum list_error_e (*list_lock_f)(struct list_s *obj);
typedef enum list_error_e (*list_unlock_f)(struct list_s *obj);

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum list_error_e {
    LIST_ERROR_NONE,
    LIST_ERROR_INIT,
    LIST_ERROR_UNINIT,
    LIST_ERROR_LOCK,
    LIST_ERROR_PARAMS
};

struct list_params_s {
    list_compare_cb compareCb;
    list_release_cb releaseCb;
    list_browse_cb  browseCb;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// MAIN CONTEXT /////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

struct list_s {
    list_add_f             add;
    list_remove_f          remove;
    list_remove_all_f      removeAll;

    list_get_nb_elements_f getNbElements;
    list_get_element_f     getElement;
    list_browse_elements_f browseElements;

    list_lock_f            lock;
    list_unlock_f          unlock;
    
    struct list_params_s params;
    void                 *pData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

enum list_error_e List_Init(struct list_s **obj, struct list_params_s *params);
enum list_error_e List_UnInit(struct list_s **obj);

#ifdef __cplusplus
}
#endif

#endif //__LIST_H__
