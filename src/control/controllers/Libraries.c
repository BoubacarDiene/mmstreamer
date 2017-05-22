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
* \file   Libraries.c
* \brief  TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/*                                           INCLUDE                                            */
/* -------------------------------------------------------------------------------------------- */

#include <dlfcn.h>

#include "control/Controllers.h"

/* -------------------------------------------------------------------------------------------- */
/*                                           DEFINE                                            */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "Libraries"

/* -------------------------------------------------------------------------------------------- */
/*                                           TYPEDEF                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                         PROTOTYPES                                           */
/* -------------------------------------------------------------------------------------------- */

CONTROLLERS_ERROR_E loadLibs_f  (CONTROLLERS_S *obj);
CONTROLLERS_ERROR_E unloadLibs_f(CONTROLLERS_S *obj);

extern void registerEvents_f  (void *userData, int32_t eventsMask);
extern void unregisterEvents_f(void *userData, int32_t eventsMask);

extern void sendCommand_f(void *userData, CONTROLLER_COMMAND_S *command);

/* -------------------------------------------------------------------------------------------- */
/*                                         VARIABLES                                            */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                      PUBLIC FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------------- */
/*                                     PRIVATE FUNCTIONS                                        */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
CONTROLLERS_ERROR_E loadLibs_f(CONTROLLERS_S *obj)
{
    assert(obj && obj->pData);

    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(obj->pData);
    INPUT_S *input                    = &pData->params.ctx->input;
    LIBRARY_S *ctrlLibs               = input->ctrlLibs;
    uint8_t priority                  = input->ctrlLibsPrio;

    pData->nbLibs = input->nbCtrlLibs;
    if (input->nbCtrlLibs == 0) {
        Logw("No library provided");
        return CONTROLLERS_ERROR_NONE;
    }

    Logd("Load \"%u\" controller(s)", pData->nbLibs);

    assert((pData->libs = (CONTROLLERS_LIB_S*)calloc(pData->nbLibs, sizeof(CONTROLLERS_LIB_S))));

    CONTROLLER_FUNCTIONS_S fcts;
    fcts.registerEvents   = registerEvents_f;
    fcts.unregisterEvents = unregisterEvents_f;
    fcts.sendCommand      = sendCommand_f;

    CONTROLLERS_LIB_S *lib;
    uint8_t index, count;
    for (index = 0; index < pData->nbLibs; ++index) {
        lib = &pData->libs[index];

        lib->handle = dlopen(ctrlLibs[index].name, RTLD_LAZY);
        if (!lib->handle) {
            Loge("Failed to load library \"%s\"", ctrlLibs[index].name);
            goto libExit;
        }

        lib->init = dlsym(lib->handle, ctrlLibs[index].initFn);
        if (!lib->init) {
            Loge("Failed to load init function \"%s\"", ctrlLibs[index].initFn);
            goto libExit;
        }

        lib->uninit = dlsym(lib->handle, ctrlLibs[index].uninitFn);
        if (!lib->uninit) {
            Loge("Failed to load uninit function \"%s\"", ctrlLibs[index].uninitFn);
            goto libExit;
        }

        lib->notify = dlsym(lib->handle, ctrlLibs[index].notifyFn);
        if (!lib->notify) {
            Loge("Failed to load notify function \"%s\"", ctrlLibs[index].notifyFn);
            goto libExit;
        }

        lib->pData    = pData;
        fcts.userData = lib;
        if (lib->init(&lib->obj, &fcts) != CONTROLLER_ERROR_NONE) {
            Loge("\"%s\" failed", ctrlLibs[index].initFn);
            goto libExit;
        }
    }

    return CONTROLLERS_ERROR_NONE;

libExit:
    while (index > 0) {
        count = index - 1;
        lib = &pData->libs[count];

        lib->uninit(&lib->obj);
        lib->obj = NULL;

        lib->init   = NULL;
        lib->uninit = NULL;
        lib->notify = NULL;
        lib->pData  = NULL;

        if (lib->handle) {
            dlclose(lib->handle);
            lib->handle = NULL;
        }

        --index;
    }

    free(pData->libs);
    pData->libs = NULL;

    return CONTROLLERS_ERROR_LIB;
}

CONTROLLERS_ERROR_E unloadLibs_f(CONTROLLERS_S *obj)
{
    assert(obj && obj->pData);

    CONTROLLERS_PRIVATE_DATA_S *pData = (CONTROLLERS_PRIVATE_DATA_S*)(obj->pData);

    if (pData->nbLibs == 0) {
        goto exit;
    }

    Logd("Unload \"%u\" controller(s)", pData->nbLibs);

    CONTROLLERS_LIB_S *lib;
    uint8_t index;
    for (index = 0; index < pData->nbLibs; ++index) {
        lib = &pData->libs[index];
        if (lib->obj) {
            lib->uninit(&lib->obj);
            lib->obj = NULL;
        }

        lib->init   = NULL;
        lib->uninit = NULL;
        lib->notify = NULL;
        lib->pData  = NULL;

        if (lib->handle) {
            dlclose(lib->handle);
            lib->handle = NULL;
        }
    }

    free(pData->libs);
    pData->libs = NULL;

exit:
    return CONTROLLERS_ERROR_NONE;
}
